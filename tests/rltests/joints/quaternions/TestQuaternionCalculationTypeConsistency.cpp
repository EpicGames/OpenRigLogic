// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/controls/ControlFixtures.h"
#include "rltests/joints/quaternions/QuaternionFixtures.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/JointBehaviorFilter.h"
#include "riglogic/joints/cpu/CPUJointsOutputInstance.h"
#include "riglogic/joints/cpu/quaternions/QuaternionJointsBuilder.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/SIMD.h"
#include "riglogic/utils/Extd.h"

#include <cstdint>
#include <string>

namespace {

struct QuaternionConsistencyTestParams {
    rl4::CalculationType calculationType;
    rl4::RotationType rotationType;
    rl4::FloatingPointModel floatingPointModel;
};

// Runs the full builder + evaluator chain per built calculation type and floating point model against the
// layout-independent expected outputs. Fast rows silently degrade to Precise where the relaxed kernels are unavailable.
class QuaternionCalculationTypeConsistencyTest : public ::testing::TestWithParam<QuaternionConsistencyTestParams> {
protected:
    static bool isSupportedAtRuntime(rl4::CalculationType calculationType) {
#ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
        // Mirrors getActiveFeatures: every compiled-in type is assumed available
        static_cast<void>(calculationType);
        return true;
#else
        const auto features = trimd::getCPUFeatures();
        switch (calculationType) {
        case rl4::CalculationType::SSE:
            return features.SSE2;
        case rl4::CalculationType::AVX:
            return features.AVX;
        case rl4::CalculationType::AVX512F:
            return features.AVX512F;
        case rl4::CalculationType::NEON:
            return features.NEON;
        case rl4::CalculationType::Scalar:
        case rl4::CalculationType::AnyVector:
        default:
            return true;
        }
#endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::qs::QuaternionReader reader;
};

}  // namespace

TEST_P(QuaternionCalculationTypeConsistencyTest, CalculationOutputsMatchCanonicalResults) {
    const auto testParams = GetParam();
    if (!isSupportedAtRuntime(testParams.calculationType)) {
#if defined(TRIMD_HAS_SSE)
        // The build already emits SSE: a negative answer means runtime CPU feature detection is broken (a TU reaching
        // trimd/Platform.h without the TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION gate), not a CPU limit.
        ASSERT_NE(testParams.calculationType, rl4::CalculationType::SSE)
            << "runtime CPU feature detection reports no SSE2 in an SSE build";
#endif  // TRIMD_HAS_SSE
        GTEST_SKIP() << "CPU does not support the requested calculation type at runtime.";
    }

    rl4::Configuration config{};
    config.calculationType = testParams.calculationType;
    config.rotationType = testParams.rotationType;
    config.floatingPointModel = testParams.floatingPointModel;

    auto meta = rl4::RigMetadata::create(config, &reader, &memRes);
    rl4::QuaternionJointsBuilder builder(config, meta.get(), &memRes);

    rl4::JointBehaviorFilter filter{&reader, &memRes};
    filter.include(dna::RotationRepresentation::Quaternion);

    builder.computeStorageRequirements(filter);
    builder.allocateStorage(filter);
    builder.fillStorage(filter);
    auto joints = builder.build();
    ASSERT_NE(joints, nullptr);

    const std::size_t rotationSelectorIndex = (testParams.rotationType == rl4::RotationType::EulerAngles) ? 1ul : 0ul;
    const auto& expectedPerLOD = rltests::qs::output::valuesPerLODPerConfig[rotationSelectorIndex];

    const auto jointAttrCount = static_cast<std::uint16_t>(expectedPerLOD[0].size());
    rl4::CPUJointsOutputInstance outputInstance{jointAttrCount,
                                                rl4::TranslationType::Vector,
                                                testParams.rotationType,
                                                rl4::ScaleType::Vector,
                                                &memRes};
    auto outputBuffer = outputInstance.getOutputBuffer();

    auto inputInstanceFactory =
        ControlsFactory::getInstanceFactory(0, static_cast<std::uint16_t>(rltests::qs::input::values.size()), 0, 0, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    std::copy(rltests::qs::input::values.begin(), rltests::qs::input::values.end(), inputBuffer.begin());

    for (std::uint16_t lod = {}; lod < rltests::qs::unoptimized::lodCount; ++lod) {
        std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
        joints->calculate(inputInstance.get(), &outputInstance, lod);
        // Expected euler outputs are in radians while the builder picks the adapter matching the reader's unit
        // (degrees); convert back to radians so the threshold stays meaningful. Every output value is an angle.
        const auto& expected = expectedPerLOD[lod];
        ASSERT_LE(expected.size(), outputBuffer.size());
        rl4::Vector<float> actual{outputBuffer.begin(), extd::advanced(outputBuffer.begin(), expected.size()), &memRes};
        if (testParams.rotationType == rl4::RotationType::EulerAngles) {
            for (auto& value : actual) {
                value = tdm::frad{tdm::fdeg{value}}.value;
            }
        }
#ifdef RL_BUILD_WITH_HALF_FLOATS
        static constexpr float threshold = 0.05f;
#else
        static constexpr float threshold = 0.002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
        ASSERT_ELEMENTS_NEAR(actual, expected, expected.size(), threshold);
    }
}

namespace {

const QuaternionConsistencyTestParams quaternionConsistencyTestParams[] = {
    {rl4::CalculationType::Scalar, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::Scalar, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
#if defined(RL_BUILD_WITH_XYZ_ROTATION_ORDER)
    {rl4::CalculationType::Scalar, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::Scalar, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
#endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER
#if defined(RL_BUILD_WITH_SSE)
    {rl4::CalculationType::SSE, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::SSE, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
    #if defined(RL_BUILD_WITH_XYZ_ROTATION_ORDER)
    {rl4::CalculationType::SSE, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::SSE, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    #endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER
#endif      // RL_BUILD_WITH_SSE
#if defined(RL_BUILD_WITH_AVX)
    {rl4::CalculationType::AVX, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
    #if defined(RL_BUILD_WITH_XYZ_ROTATION_ORDER)
    {rl4::CalculationType::AVX, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    #endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER
#endif      // RL_BUILD_WITH_AVX
#if defined(RL_BUILD_WITH_AVX512F)
    {rl4::CalculationType::AVX512F, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX512F, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
    #if defined(RL_BUILD_WITH_XYZ_ROTATION_ORDER)
    {rl4::CalculationType::AVX512F, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX512F, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    #endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER
#endif      // RL_BUILD_WITH_AVX512F
#if defined(RL_BUILD_WITH_NEON)
    {rl4::CalculationType::NEON, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::NEON, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
    #if defined(RL_BUILD_WITH_XYZ_ROTATION_ORDER)
    {rl4::CalculationType::NEON, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::NEON, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    #endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER
#endif      // RL_BUILD_WITH_NEON
};

std::string quaternionConsistencyTestName(const ::testing::TestParamInfo<QuaternionConsistencyTestParams>& info) {
    std::string name;
    switch (info.param.calculationType) {
    case rl4::CalculationType::Scalar:
        name = "Scalar";
        break;
    case rl4::CalculationType::SSE:
        name = "SSE";
        break;
    case rl4::CalculationType::AVX:
        name = "AVX";
        break;
    case rl4::CalculationType::AVX512F:
        name = "AVX512F";
        break;
    case rl4::CalculationType::NEON:
        name = "NEON";
        break;
    case rl4::CalculationType::AnyVector:
    default:
        name = "AnyVector";
        break;
    }
    name += (info.param.rotationType == rl4::RotationType::Quaternions) ? "_Quaternions" : "_EulerAngles";
    name += (info.param.floatingPointModel == rl4::FloatingPointModel::Fast) ? "_Fast" : "_Precise";
    return name;
}

}  // namespace

INSTANTIATE_TEST_SUITE_P(QuaternionCalculationTypeConsistencyTest,
                         QuaternionCalculationTypeConsistencyTest,
                         ::testing::ValuesIn(quaternionConsistencyTestParams),
                         quaternionConsistencyTestName);
