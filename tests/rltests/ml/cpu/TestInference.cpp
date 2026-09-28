// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef _MSC_VER
    #pragma warning(disable : 4503)
#endif

#include "rltests/Defs.h"
#include "rltests/StorageValueType.h"
#include "rltests/controls/ControlFixtures.h"
#include "rltests/ml/cpu/FixturesBlock4.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/CPUMachineLearnedBehaviorFactory.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/Utils.h"

#include <string>
#include <tuple>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4324)
#endif

namespace {

struct MLBInferenceTestParams {
    rl4::CalculationType calculationType;
    rl4::FloatingPointModel floatingPointModel;
};

class MLBInferenceTest : public ::testing::TestWithParam<MLBInferenceTestParams> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam().calculationType;
        config.floatingPointModel = GetParam().floatingPointModel;
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::block4::CanonicalReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

}  // namespace

TEST_P(MLBInferenceTest, InferencePerLOD) {
    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0,
                                                                    rltests::ml::block4::unoptimized::rawControlCount,
                                                                    0,
                                                                    rltests::ml::block4::unoptimized::mlControlCount,
                                                                    0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer =
        inputBuffer.subview(rltests::ml::block4::unoptimized::rawControlCount, rltests::ml::block4::unoptimized::mlControlCount);
    std::copy(rltests::ml::block4::input::values.begin(), rltests::ml::block4::input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    for (std::uint16_t lod = 0u; lod < rltests::ml::block4::unoptimized::lodCount; ++lod) {
        std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
        this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
        const auto& expected = rltests::ml::block4::output::valuesPerLOD[lod];
#ifdef RL_BUILD_WITH_HALF_FLOATS
        static constexpr float threshold = 0.15f;
#else
        static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
        ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
    }
}

namespace {

/*
 * The Fast floating point model rows silently degrade to Precise when the binary is built
 * without RL_BUILD_WITH_FAST or the runtime environment cannot provide the relaxed kernels,
 * so they are valid (if then redundant) in every configuration. Calculation types that are
 * not built or not supported by the CPU fall back to the first available arm, as everywhere.
 */
const MLBInferenceTestParams mlbInferenceTestParams[] = {
    {rl4::CalculationType::AVX512F, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX512F, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::AVX, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::AVX, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::SSE, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::SSE, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::NEON, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::NEON, rl4::FloatingPointModel::Fast},
    {rl4::CalculationType::Scalar, rl4::FloatingPointModel::Precise},
    {rl4::CalculationType::Scalar, rl4::FloatingPointModel::Fast},
};

std::string mlbInferenceTestName(const ::testing::TestParamInfo<MLBInferenceTestParams>& info) {
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
    name += (info.param.floatingPointModel == rl4::FloatingPointModel::Fast) ? "_Fast" : "_Precise";
    return name;
}

}  // namespace

INSTANTIATE_TEST_SUITE_P(MLBInferenceTestSuite,
                         MLBInferenceTest,
                         ::testing::ValuesIn(mlbInferenceTestParams),
                         mlbInferenceTestName);

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
