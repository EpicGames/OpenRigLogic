// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef _MSC_VER
    #pragma warning(disable : 4503)
#endif

#include "rltests/Defs.h"
#include "rltests/StorageValueType.h"
#include "rltests/controls/ControlFixtures.h"
#include "rltests/ml/cpu/FixturesMLBChained.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/CPUMachineLearnedBehaviorFactory.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/Utils.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4324)
#endif

namespace {

// Exercises the chained topology: Gather -> MLP(odd layers) -> WeightedSum -> MLP(even layers) -> Scatter.
// The odd-layer intermediate MLP triggers the ping-pong normalization fix in Operation.h.
// Running multiple frames per LOD guards against stale-buffer regressions.
class MLBChainedInferenceTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes, rl4::InitializationMethod::Create);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::chained::CanonicalReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

// Same topology as above, but each neural net is assigned to a mesh region, so mask weights apply:
// mask region 0 attenuates NN1 (final MLP, scatter outputs), region 1 attenuates NN0 (intermediate dep MLP).
class MLBChainedMaskedInferenceTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes, rl4::InitializationMethod::Create);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::chained::MaskedCanonicalReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

// Same topology, but the MLP op set contains a layer-less op at index 0 (real op shifted to index 1).
// The factory must keep a placeholder for it so op indices stay aligned; results must match the canonical
// topology exactly, since the placeholder contributes nothing.
class MLBChainedMixedOpSetInferenceTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes, rl4::InitializationMethod::Create);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::chained::MixedOpSetReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

// The WeightedSum reads more elements (12) from its dependency's buffer than the dependency outputs (8).
// The factory-computed tail-zeroing must make the over-read region blend zeros - matching the old
// unconditional tail fill - so the results stay canonical and stable across frames.
class MLBChainedWSOverreadInferenceTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes, rl4::InitializationMethod::Create);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::chained::WSOverreadReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

// The final MLP outputs 8 values at LOD 0 but only 4 at LOD 1; with masks available, all three scatter
// paths (unmasked, attenuated, zero-weight defaults) must write only the current LOD's output count.
class MLBChainedLODLimitedMaskedInferenceTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes, rl4::InitializationMethod::Create);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::chained::LODLimitedMaskedReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

// The over-reading WeightedSum composed with a mask on the over-read intermediate dep: the over-read
// tail must blend zeros in every mask state, including when the dep is masked off before it ever ran.
class MLBChainedMaskedWSOverreadInferenceTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    void SetUp() override {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes, rl4::InitializationMethod::Create);
        evaluator = rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    rltests::ml::chained::MaskedWSOverreadReader reader;
    rl4::MachineLearnedBehaviorEvaluator::Pointer evaluator;
};

}  // namespace

TEST_P(MLBChainedMaskedWSOverreadInferenceTest, OverreadTailStaysZeroAcrossMaskStates) {
    using namespace rltests::ml::chained;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer = inputBuffer.subview(rawControlCount, mlControlCount);
    std::copy(input::values.begin(), input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    auto maskBuffer = intermediateOutputs->getMaskBuffer();
    ASSERT_EQ(maskBuffer.size(), 1ul);

    // The first evaluation runs with the dep masked off (never evaluated unmasked yet - the over-read tail
    // holds the instance's zero-initialized memory), then unmasked (tail re-zeroed by the trailing memset),
    // then masked off again (the zero-weight path preserves the previously zeroed tail).
    const float depMaskWeights[] = {0.0f, 1.0f, 0.0f};

    for (std::uint16_t lod = 0u; lod < lodCount; ++lod) {
        for (const float depWeight : depMaskWeights) {
            maskBuffer[0] = depWeight;  // region 0 -> NN0 (the over-read intermediate dep)
            std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
            this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
            // Masked off: NN0 publishes zero defaults and the over-read tail is zero, so everything is 0.
            // Unmasked: canonical outputs (the over-read tail blends zeros, not stale data).
            const auto& unmasked = output::valuesPerLOD[lod];
            rl4::Vector<float> expected{unmasked.size(), {}, &this->memRes};
            for (std::size_t i = {}; i < unmasked.size(); ++i) {
                expected[i] = unmasked[i] * depWeight;
            }
#ifdef RL_BUILD_WITH_HALF_FLOATS
            static constexpr float threshold = 0.15f;
#else
            static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
            ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
        }
    }
}

TEST_P(MLBChainedMixedOpSetInferenceTest, PlaceholderOpsKeepIndicesAligned) {
    using namespace rltests::ml::chained;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer = inputBuffer.subview(rawControlCount, mlControlCount);
    std::copy(input::values.begin(), input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    for (std::uint16_t lod = 0u; lod < lodCount; ++lod) {
        for (int frame = 0; frame < 3; ++frame) {
            std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
            this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
            const auto& expected = output::valuesPerLOD[lod];
#ifdef RL_BUILD_WITH_HALF_FLOATS
            static constexpr float threshold = 0.15f;
#else
            static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
            ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
        }
    }
}

TEST_P(MLBChainedInferenceTest, InferencePerLOD) {
    using namespace rltests::ml::chained;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer = inputBuffer.subview(rawControlCount, mlControlCount);
    std::copy(input::values.begin(), input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    for (std::uint16_t lod = 0u; lod < lodCount; ++lod) {
        // Run multiple frames per LOD to detect stale-buffer bugs.
        for (int frame = 0; frame < 3; ++frame) {
            std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
            this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
            const auto& expected = output::valuesPerLOD[lod];
#ifdef RL_BUILD_WITH_HALF_FLOATS
            static constexpr float threshold = 0.15f;
#else
            static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
            ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
        }
    }
}

TEST_P(MLBChainedMaskedInferenceTest, MaskWeightsAttenuateOutputs) {
    using namespace rltests::ml::chained;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer = inputBuffer.subview(rawControlCount, mlControlCount);
    std::copy(input::values.begin(), input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    auto maskBuffer = intermediateOutputs->getMaskBuffer();
    ASSERT_EQ(maskBuffer.size(), 2ul);

    struct MaskCase {
        float finalNetWeight;         // mask region 0 -> NN1
        float intermediateNetWeight;  // mask region 1 -> NN0
        float expectedScale;          // all-linear nets, so outputs scale by the product of applied weights
    };
    // A weight of 0 short-circuits evaluation and emits default values (zeros), for both final and intermediate nets.
    const MaskCase maskCases[] =
        {{1.0f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.5f}, {1.0f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.25f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

    for (std::uint16_t lod = 0u; lod < lodCount; ++lod) {
        for (const auto& maskCase : maskCases) {
            maskBuffer[0] = maskCase.finalNetWeight;
            maskBuffer[1] = maskCase.intermediateNetWeight;
            std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
            this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
            const auto& unmasked = output::valuesPerLOD[lod];
            rl4::Vector<float> expected{unmasked.size(), {}, &this->memRes};
            for (std::size_t i = {}; i < unmasked.size(); ++i) {
                expected[i] = unmasked[i] * maskCase.expectedScale;
            }
#ifdef RL_BUILD_WITH_HALF_FLOATS
            static constexpr float threshold = 0.15f;
#else
            static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
            ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
        }
    }
}

TEST_P(MLBChainedWSOverreadInferenceTest, OverreadTailBlendsZeros) {
    using namespace rltests::ml::chained;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer = inputBuffer.subview(rawControlCount, mlControlCount);
    std::copy(input::values.begin(), input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    for (std::uint16_t lod = 0u; lod < lodCount; ++lod) {
        // The stale over-read data only appears from the first evaluation onward (buffers start zeroed),
        // so run multiple frames to prove the tail stays zeroed on every evaluation.
        for (int frame = 0; frame < 3; ++frame) {
            std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
            this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
            const auto& expected = output::valuesPerLOD[lod];
#ifdef RL_BUILD_WITH_HALF_FLOATS
            static constexpr float threshold = 0.15f;
#else
            static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
            ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
        }
    }
}

TEST_P(MLBChainedLODLimitedMaskedInferenceTest, ScatterWritesOnlyCurrentLODOutputs) {
    using namespace rltests::ml::chained;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &this->memRes);
    auto inputBuffer = inputInstance->getInputBuffer();
    auto outputBuffer = inputBuffer.subview(rawControlCount, mlControlCount);
    std::copy(input::values.begin(), input::values.end(), inputBuffer.begin());

    auto intermediateOutputs = this->evaluator->createInstance(&this->memRes);
    auto maskBuffer = intermediateOutputs->getMaskBuffer();
    ASSERT_EQ(maskBuffer.size(), 2ul);

    // Control slots past the current LOD's output count must keep this sentinel across all mask states.
    static constexpr float sentinel = 7.7f;
    const std::uint16_t outputCountPerLOD[] = {8u, 4u};
    const float finalNetWeights[] = {1.0f, 0.5f, 0.0f};

    for (std::uint16_t lod = 0u; lod < lodCount; ++lod) {
        for (const float weight : finalNetWeights) {
            maskBuffer[0] = weight;  // region 0 -> NN1 (final MLP)
            maskBuffer[1] = 1.0f;    // region 1 -> NN0 (intermediate MLP)
            std::fill(outputBuffer.begin(), outputBuffer.end(), sentinel);
            this->evaluator->calculate(inputInstance.get(), intermediateOutputs.get(), lod);
            // weight 0 emits default values (zeros); any other weight scales the canonical outputs.
            const auto& unmasked = output::valuesPerLOD[lod];
            rl4::Vector<float> expected{unmasked.size(), sentinel, &this->memRes};
            for (std::size_t i = {}; i < outputCountPerLOD[lod]; ++i) {
                expected[i] = unmasked[i] * weight;
            }
#ifdef RL_BUILD_WITH_HALF_FLOATS
            static constexpr float threshold = 0.15f;
#else
            static constexpr float threshold = 0.0002f;
#endif  // RL_BUILD_WITH_HALF_FLOATS
            ASSERT_ELEMENTS_NEAR(outputBuffer, expected, expected.size(), threshold);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(MLBChainedInferenceTestSuite,
                         MLBChainedInferenceTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

INSTANTIATE_TEST_SUITE_P(MLBChainedMaskedInferenceTestSuite,
                         MLBChainedMaskedInferenceTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

INSTANTIATE_TEST_SUITE_P(MLBChainedMixedOpSetInferenceTestSuite,
                         MLBChainedMixedOpSetInferenceTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

INSTANTIATE_TEST_SUITE_P(MLBChainedMaskedWSOverreadInferenceTestSuite,
                         MLBChainedMaskedWSOverreadInferenceTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

INSTANTIATE_TEST_SUITE_P(MLBChainedWSOverreadInferenceTestSuite,
                         MLBChainedWSOverreadInferenceTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

INSTANTIATE_TEST_SUITE_P(MLBChainedLODLimitedMaskedInferenceTestSuite,
                         MLBChainedLODLimitedMaskedInferenceTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
