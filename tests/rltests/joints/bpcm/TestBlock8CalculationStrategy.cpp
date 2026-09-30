// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef _MSC_VER
    #pragma warning(disable : 4503)
#endif

#include "rltests/Defs.h"
#include "rltests/controls/ControlFixtures.h"
#include "rltests/joints/bpcm/BPCMFixturesBlock8.h"
#include "rltests/joints/bpcm/Helpers.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/bpcm/BPCMCalculationStrategy.h"
#include "riglogic/joints/cpu/bpcm/BPCMJointsEvaluator.h"
#include "riglogic/joints/cpu/bpcm/RotationAdapters.h"
#include "riglogic/system/simd/SIMD.h"

namespace {

// Per-arm SIMD width sets as RuntimeTemplateInstantiator passes them; the fixture storage uses per-group block heights,
// so the strategies under test are the multi-width ones dispatching on JointGroup::blockHeight.
#if defined(RL_BUILD_WITH_AVX)
struct AVXWidths {
    using TF512 = trimd::fallback::T512<trimd::avx::F256>;
    using TF256 = trimd::avx::F256;
    using TF128 = trimd::sse::F128;
};
#endif  // RL_BUILD_WITH_AVX

#if defined(RL_BUILD_WITH_AVX) || defined(RL_BUILD_WITH_SSE)
struct SSEWidths {
    using TF512 = trimd::fallback::T512<trimd::sse::F256>;
    using TF256 = trimd::sse::F256;
    using TF128 = trimd::sse::F128;
};
#endif  // RL_BUILD_WITH_AVX || RL_BUILD_WITH_SSE

#if defined(RL_BUILD_WITH_NEON)
struct NEONWidths {
    using TF512 = trimd::fallback::T512<trimd::neon::F256>;
    using TF256 = trimd::neon::F256;
    using TF128 = trimd::neon::F128;
};
#endif  // RL_BUILD_WITH_NEON

struct ScalarWidths {
    using TF512 = trimd::fallback::T512<trimd::scalar::F256>;
    using TF256 = trimd::scalar::F256;
    using TF128 = trimd::scalar::F128;
};

template<typename TTestTypes>
class Block8JointCalculationStrategyTest : public ::testing::TestWithParam<StrategyTestParams> {
protected:
    void SetUp() override {
        Block8JointCalculationStrategyTest::SetUpImpl();
    }

    template<typename TestTypes = TTestTypes>
    typename std::enable_if<std::tuple_size<TestTypes>::value != 0ul, void>::type SetUpImpl() {
        using T = typename std::tuple_element<0, TestTypes>::type;
        using TWidths = typename std::tuple_element<1, TestTypes>::type;
        using TStrategyTestParams = typename std::tuple_element<2, TestTypes>::type;
        using TRotationAdapter = typename std::tuple_element<3, TestTypes>::type;
        params.lod = TStrategyTestParams::lod();

        using CalculationStrategyBase = rl4::bpcm::JointGroupLinearCalculationStrategy;
        using CalculationStrategy = rl4::bpcm::MultiWidthJointGroupLinearCalculationStrategy<T,
                                                                                             typename TWidths::TF512,
                                                                                             typename TWidths::TF256,
                                                                                             typename TWidths::TF128,
                                                                                             TRotationAdapter>;
        strategy = pma::UniqueInstance<CalculationStrategy, CalculationStrategyBase>::with(&memRes).create(
            TRotationAdapter{block8::unoptimized::rotationSigns});

        rotationSelectorIndex = BPCMRotationOutputTypeSelector<TRotationAdapter>::value();
        rotationType = BPCMRotationOutputTypeSelector<TRotationAdapter>::rotation();
    }

    template<typename TestTypes = TTestTypes>
    typename std::enable_if<std::tuple_size<TestTypes>::value == 0ul, void>::type SetUpImpl() {
        GTEST_SKIP();
    }

    template<typename TArray>
    void execute(const rl4::bpcm::Evaluator& joints, const TArray& expected, OutputScope scope) {
        auto outputInstance = joints.createInstance(&memRes);
        outputInstance->resetOutputBuffer();
        auto outputBuffer = outputInstance->getOutputBuffer();

        auto inputInstanceFactory =
            ControlsFactory::getInstanceFactory(0, static_cast<std::uint16_t>(block8::input::values.size()), 0, 0, 0);
        rl4::Vector<rl4::ControlInitializer> initialValues;
        auto inputInstance = inputInstanceFactory(initialValues, &memRes);
        auto inputBuffer = inputInstance->getInputBuffer();
        std::copy(block8::input::values.begin(), block8::input::values.end(), inputBuffer.begin());

        rl4::ConstArrayView<float> expectedView{expected[scope.lod].data() + scope.offset, scope.size};
        rl4::ConstArrayView<float> outputView{outputBuffer.data() + scope.offset, scope.size};
        joints.calculate(inputInstance.get(), outputInstance.get(), scope.lod);
        ASSERT_ELEMENTS_NEAR(outputView, expectedView, expectedView.size(), 0.002f);
    }

protected:
    pma::AlignedMemoryResource memRes;
    block8::OptimizedStorage<StorageValueType>::StrategyPtr strategy;
    StrategyTestParams params;
    std::size_t rotationSelectorIndex;
    rl4::RotationType rotationType;
};

}  // namespace

using Block8JointCalculationTypeList = ::testing::Types<
#if defined(RL_BUILD_WITH_AVX)
    std::tuple<StorageValueType, AVXWidths, TStrategyTestParams<0>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               AVXWidths,
               TStrategyTestParams<0>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, AVXWidths, TStrategyTestParams<1>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               AVXWidths,
               TStrategyTestParams<1>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, AVXWidths, TStrategyTestParams<2>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               AVXWidths,
               TStrategyTestParams<2>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, AVXWidths, TStrategyTestParams<3>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               AVXWidths,
               TStrategyTestParams<3>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
#endif  // RL_BUILD_WITH_AVX
#if defined(RL_BUILD_WITH_AVX) || defined(RL_BUILD_WITH_SSE)
    std::tuple<StorageValueType, SSEWidths, TStrategyTestParams<0>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               SSEWidths,
               TStrategyTestParams<0>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, SSEWidths, TStrategyTestParams<1>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               SSEWidths,
               TStrategyTestParams<1>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, SSEWidths, TStrategyTestParams<2>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               SSEWidths,
               TStrategyTestParams<2>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, SSEWidths, TStrategyTestParams<3>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               SSEWidths,
               TStrategyTestParams<3>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
#endif  // RL_BUILD_WITH_AVX || RL_BUILD_WITH_SSE
#if defined(RL_BUILD_WITH_NEON)
    std::tuple<StorageValueType, NEONWidths, TStrategyTestParams<0>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               NEONWidths,
               TStrategyTestParams<0>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, NEONWidths, TStrategyTestParams<1>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               NEONWidths,
               TStrategyTestParams<1>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, NEONWidths, TStrategyTestParams<2>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               NEONWidths,
               TStrategyTestParams<2>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, NEONWidths, TStrategyTestParams<3>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               NEONWidths,
               TStrategyTestParams<3>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
#endif  // RL_BUILD_WITH_NEON
#if !defined(RL_BUILD_WITH_HALF_FLOATS)
    std::tuple<StorageValueType, ScalarWidths, TStrategyTestParams<0>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               ScalarWidths,
               TStrategyTestParams<0>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, ScalarWidths, TStrategyTestParams<1>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               ScalarWidths,
               TStrategyTestParams<1>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, ScalarWidths, TStrategyTestParams<2>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               ScalarWidths,
               TStrategyTestParams<2>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
    std::tuple<StorageValueType, ScalarWidths, TStrategyTestParams<3>, rl4::bpcm::NoopAdapter>,
    std::tuple<StorageValueType,
               ScalarWidths,
               TStrategyTestParams<3>,
               rl4::bpcm::EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>>,
#endif  // RL_BUILD_WITH_HALF_FLOATS
    std::tuple<>>;

TYPED_TEST_SUITE(Block8JointCalculationStrategyTest, Block8JointCalculationTypeList, );

TYPED_TEST(Block8JointCalculationStrategyTest, Block8Padded) {
    const std::uint16_t jointGroupIndex = 0u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block8Exact) {
    const std::uint16_t jointGroupIndex = 1u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block16Padded) {
    const std::uint16_t jointGroupIndex = 2u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block16Exact) {
    const std::uint16_t jointGroupIndex = 3u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block24Padded) {
    const std::uint16_t jointGroupIndex = 4u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block24Exact) {
    const std::uint16_t jointGroupIndex = 5u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block32Padded) {
    const std::uint16_t jointGroupIndex = 6u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, Block32Exact) {
    const std::uint16_t jointGroupIndex = 7u;
    const auto& outputIndices = block8::optimized::outputIndices[this->rotationSelectorIndex][jointGroupIndex];
    const OutputScope scope{this->params.lod, outputIndices[0], block8::unoptimized::outputIndices[jointGroupIndex].size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     jointGroupIndex,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}

TYPED_TEST(Block8JointCalculationStrategyTest, MultipleBlocks) {
    const OutputScope scope{this->params.lod, 0ul, block8::output::valuesPerLOD[this->rotationSelectorIndex].front().size()};
    auto joints = block8::OptimizedStorage<StorageValueType>::create(std::move(this->strategy),
                                                                     this->rotationSelectorIndex,
                                                                     this->rotationType,
                                                                     &this->memRes);
    this->execute(joints, block8::output::valuesPerLOD[this->rotationSelectorIndex], scope);
}
