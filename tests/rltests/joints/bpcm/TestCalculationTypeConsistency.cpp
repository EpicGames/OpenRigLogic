// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/controls/ControlFixtures.h"
#include "rltests/joints/bpcm/BPCMFixturesBlock8.h"
#include "rltests/joints/bpcm/Helpers.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/JointBehaviorFilter.h"
#include "riglogic/joints/JointsBuilder.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/SIMD.h"

#include <algorithm>
#include <cstdint>
#include <string>

namespace {

struct CalculationTypeTestParams {
    rl4::CalculationType calculationType;
    rl4::RotationType rotationType;
    rl4::FloatingPointModel floatingPointModel;
};

// FakeReader defaults are unusable here: a zero joint count sizes the composite evaluator's output buffers to nothing
// (kernels write out of bounds) and zero-initialized rotation signs collapse every quaternion output to identity.
class ConsistencyTestReader : public block8::CanonicalReader {
public:
    std::uint16_t getJointCount() const override {
        // Expected output rows are not necessarily an exact multiple of the per-joint attribute
        // count, so round up to guarantee the output buffer covers every expected value.
        const std::size_t quaternionOutputSize = block8::output::valuesPerLOD[0].front().size();
        const std::size_t eulerOutputSize = block8::output::valuesPerLOD[1].front().size();
        const std::size_t jointCount = std::max((eulerOutputSize + 8ul) / 9ul, (quaternionOutputSize + 9ul) / 10ul);
        return static_cast<std::uint16_t>(jointCount);
    }

    dna::RotationSign getRotationSign() const override {
        return block8::unoptimized::rotationSigns;
    }

    dna::RotationSequence getRotationSequence() const override {
        // The expected quaternion outputs are generated for xyz - the only rotation order guaranteed compiled in on
        // every consumer, so the builder can always construct the matching adapter.
        return dna::RotationSequence::xyz;
    }
};

// Runs the full builder + evaluator chain per built calculation type against the layout-independent expected outputs,
// covering calculation types whose optimized storage layout has no dedicated fixture set (e.g. AVX-512F).
class BPCMCalculationTypeConsistencyTest : public ::testing::TestWithParam<CalculationTypeTestParams> {
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
    ConsistencyTestReader reader;
};

}  // namespace

TEST_P(BPCMCalculationTypeConsistencyTest, CalculationOutputsMatchCanonicalResults) {
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
    auto builder = rl4::JointsBuilder::create(config, meta.get(), &memRes);

    rl4::JointBehaviorFilter filter{&reader, &memRes};
    filter.include(dna::TranslationRepresentation::Vector);
    filter.include(dna::RotationRepresentation::EulerAngles);
    filter.include(dna::ScaleRepresentation::Vector);

    builder->computeStorageRequirements(filter);
    builder->allocateStorage(filter);
    builder->fillStorage(filter);
    auto joints = builder->build();
    ASSERT_NE(joints, nullptr);

    auto outputInstance = joints->createInstance(&memRes);

    auto inputInstanceFactory =
        ControlsFactory::getInstanceFactory(0, static_cast<std::uint16_t>(block8::input::values.size()), 0, 0, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    std::copy(block8::input::values.begin(), block8::input::values.end(), inputBuffer.begin());

    const std::size_t rotationSelectorIndex = (testParams.rotationType == rl4::RotationType::Quaternions) ? 0ul : 1ul;
    const auto& expectedPerLOD = block8::output::valuesPerLOD[rotationSelectorIndex];
    for (std::uint16_t lod = {}; lod < block8::unoptimized::lodCount; ++lod) {
        outputInstance->resetOutputBuffer();
        joints->calculate(inputInstance.get(), outputInstance.get(), lod);
        auto outputBuffer = outputInstance->getOutputBuffer();
        const auto& expected = expectedPerLOD[lod];
        ASSERT_LE(expected.size(), outputBuffer.size());
        ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), 0.002f);
    }
}

namespace {

// Fast floating point rows silently degrade to Precise when built without RL_BUILD_WITH_FAST or the CPU lacks FMA, so
// they are valid in every configuration; outputs must match within the same tolerance - FMA only changes ulp-scale.
const CalculationTypeTestParams calculationTypeTestParams[] = {
    {rl4::CalculationType::Scalar, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::Scalar, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::Scalar, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::Scalar, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
#if defined(RL_BUILD_WITH_SSE)
    {rl4::CalculationType::SSE, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::SSE, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::SSE, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::SSE, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
#endif  // RL_BUILD_WITH_SSE
#if defined(RL_BUILD_WITH_AVX)
    {rl4::CalculationType::AVX, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::AVX, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
#endif  // RL_BUILD_WITH_AVX
#if defined(RL_BUILD_WITH_AVX512F)
    {rl4::CalculationType::AVX512F, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX512F, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::AVX512F, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX512F, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
#endif  // RL_BUILD_WITH_AVX512F
#if defined(RL_BUILD_WITH_NEON)
    {rl4::CalculationType::NEON, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::NEON, rl4::RotationType::EulerAngles, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::NEON, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::NEON, rl4::RotationType::Quaternions, rl4::FloatingPointModel::Fast},
#endif  // RL_BUILD_WITH_NEON
};

std::string calculationTypeTestName(const ::testing::TestParamInfo<CalculationTypeTestParams>& info) {
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

INSTANTIATE_TEST_SUITE_P(BPCMCalculationTypeConsistencyTest,
                         BPCMCalculationTypeConsistencyTest,
                         ::testing::ValuesIn(calculationTypeTestParams),
                         calculationTypeTestName);
