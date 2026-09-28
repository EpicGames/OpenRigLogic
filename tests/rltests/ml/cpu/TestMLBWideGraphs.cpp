// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef _MSC_VER
    #pragma warning(disable : 4503)
#endif

#include "rltests/Defs.h"
#include "rltests/dna/FakeReader.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/CPUMachineLearnedBehaviorFactory.h"
#include "riglogic/ml/cpu/CPUMachineLearnedBehaviorOutputInstance.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "rltests/controls/ControlFixtures.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4324 4365 4987)
#endif
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Count-boundary ML graphs through the real DNA factory path. Each case pairs the largest shape the runtime supports,
// which must compute, with one step past it, which the factory must reject (nullptr) before any work buffer is allocated.

namespace {

// Two WeightedSum op-sets of 32768 and 32769 dependency-less ops (2 outputs each) in one ML type, so the per-set
// running op offsets in the output instance pass 65535.
// Namespace-scope because gtest assertions bind by reference, which in C++11 ODR-uses in-class static constexpr members.
constexpr std::uint16_t kManyOpsSet0 = 32768u;
constexpr std::uint16_t kManyOpsSet1 = 32769u;

class ManyWeightedSumOpsReader : public dna::FakeReader {
public:
    ManyWeightedSumOpsReader() :
        activeOpsSet0(kManyOpsSet0),
        activeOpsSet1(kManyOpsSet1) {
        std::iota(activeOpsSet0.begin(), activeOpsSet0.end(), static_cast<std::uint16_t>(0u));
        std::iota(activeOpsSet1.begin(), activeOpsSet1.end(), static_cast<std::uint16_t>(0u));
    }

    ~ManyWeightedSumOpsReader();

    std::uint16_t getRawControlCount() const override {
        return 1u;
    }

    std::uint16_t getMLControlCount() const override {
        return 1u;
    }

    std::uint16_t getLODCount() const override {
        return 1u;
    }

    std::uint16_t getMLTypeCount() const override {
        return 1u;
    }

    std::uint16_t getMLOperationSetCount(std::uint16_t /*unused*/) const override {
        return 2u;
    }

    std::uint16_t getMLOperationCount(std::uint16_t /*unused*/, std::uint16_t mlOperationSetIndex) const override {
        return (mlOperationSetIndex == 0u) ? kManyOpsSet0 : kManyOpsSet1;
    }

    dna::MachineLearnedBehaviorOperationType getMLOperationType(std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/) const override {
        return dna::MachineLearnedBehaviorOperationType::WeightedSum;
    }

    rl4::ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/) const override {
        static const std::uint32_t wsParams[] = {2u};  // outputCount 2, no dependency weights
        return {wsParams, 1ul};
    }

    rl4::ConstArrayView<std::uint16_t> getMLOperationIndicesForLOD(std::uint16_t /*unused*/,
                                                                   std::uint16_t mlOperationSetIndex,
                                                                   std::uint16_t /*unused*/) const override {
        const auto& active = (mlOperationSetIndex == 0u) ? activeOpsSet0 : activeOpsSet1;
        return {active.data(), active.size()};
    }

private:
    std::vector<std::uint16_t> activeOpsSet0;
    std::vector<std::uint16_t> activeOpsSet1;
};

ManyWeightedSumOpsReader::~ManyWeightedSumOpsReader() = default;

// A fully data-driven single-type ML graph: every DNA field the factory reads is a public vector, so each rejection
// case below is the canonical Gather -> MLP -> Scatter graph with exactly one field perturbed.
struct GraphSpec {
    std::uint16_t rawControlCount = 1u;
    std::uint16_t mlControlCount = 1u;
    std::vector<std::vector<dna::MachineLearnedBehaviorOperationType>> opTypes;  // [set][op]
    std::vector<std::vector<std::vector<std::uint32_t>>> params;                 // [set][op]
    std::vector<std::vector<std::vector<std::uint16_t>>> depSets;                // [set][op]
    std::vector<std::vector<std::vector<std::uint16_t>>> depOps;                 // [set][op]
    std::vector<std::vector<std::uint16_t>> activeOps;                           // [set], same list for every LOD
    std::vector<float> weights;                                                  // net 0, one linear layer
    std::vector<float> biases;
    std::vector<std::uint16_t> mlJointsKeys;
    std::vector<std::uint16_t> mlJointsValues;
    std::vector<std::uint16_t> mlJointsInputIndices;
    std::vector<std::uint16_t> mlJointsOutputIndices;

    // Bias of output row i; every weight is 1, so output[i] = sum(inputs) + bias(i) exactly in float and half.
    static float bias(std::size_t outputIndex) {
        return static_cast<float>(outputIndex % 8u) * 0.125f;
    }

    // Gather(set 0) of raw controls [0, inputCount) -> MLP(set 1, outputCount x inputCount, weights 1, bias(i))
    // -> Scatter(set 2) into ML controls [inputCount, inputCount + outputCount).
    static GraphSpec singleLayer(std::uint16_t inputCount, std::uint16_t outputCount) {
        using OpType = dna::MachineLearnedBehaviorOperationType;
        GraphSpec g;
        g.rawControlCount = inputCount;
        g.mlControlCount = outputCount;
        g.opTypes = {{OpType::Gather}, {OpType::MLP}, {OpType::Scatter}};
        std::vector<std::uint32_t> gather(inputCount);
        std::iota(gather.begin(), gather.end(), 0u);
        std::vector<std::uint32_t> scatter(outputCount);
        std::iota(scatter.begin(), scatter.end(), static_cast<std::uint32_t>(inputCount));
        g.params = {{gather}, {{0u}}, {scatter}};
        g.depSets = {{{}}, {{0u}}, {{1u}}};
        g.depOps = {{{}}, {{0u}}, {{0u}}};
        g.activeOps = {{0u}, {0u}, {0u}};
        g.weights.assign(static_cast<std::size_t>(inputCount) * outputCount, 1.0f);
        g.biases.resize(outputCount);
        for (std::size_t i = {}; i < g.biases.size(); ++i) {
            g.biases[i] = bias(i);
        }
        return g;
    }
};

class GraphReader : public dna::FakeReader {
public:
    explicit GraphReader(GraphSpec spec_) :
        spec{std::move(spec_)} {
    }

    ~GraphReader();

    std::uint16_t getRawControlCount() const override {
        return spec.rawControlCount;
    }

    std::uint16_t getMLControlCount() const override {
        return spec.mlControlCount;
    }

    std::uint16_t getLODCount() const override {
        return 1u;
    }

    std::uint16_t getMLTypeCount() const override {
        return 1u;
    }

    std::uint16_t getMLOperationSetCount(std::uint16_t /*unused*/) const override {
        return static_cast<std::uint16_t>(spec.opTypes.size());
    }

    std::uint16_t getMLOperationCount(std::uint16_t /*unused*/, std::uint16_t set) const override {
        return static_cast<std::uint16_t>(spec.opTypes[set].size());
    }

    dna::MachineLearnedBehaviorOperationType getMLOperationType(std::uint16_t /*unused*/,
                                                                std::uint16_t set,
                                                                std::uint16_t op) const override {
        return spec.opTypes[set][op];
    }

    rl4::ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t /*unused*/,
                                                                std::uint16_t set,
                                                                std::uint16_t op) const override {
        return {spec.params[set][op].data(), spec.params[set][op].size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLOperationDependencyOperationSetIndices(std::uint16_t /*unused*/,
                                                                                   std::uint16_t set,
                                                                                   std::uint16_t op) const override {
        return {spec.depSets[set][op].data(), spec.depSets[set][op].size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLOperationDependencyOperationIndices(std::uint16_t /*unused*/,
                                                                                std::uint16_t set,
                                                                                std::uint16_t op) const override {
        return {spec.depOps[set][op].data(), spec.depOps[set][op].size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLOperationIndicesForLOD(std::uint16_t /*unused*/,
                                                                   std::uint16_t set,
                                                                   std::uint16_t /*unused*/) const override {
        return {spec.activeOps[set].data(), spec.activeOps[set].size()};
    }

    std::uint16_t getNeuralNetworkLayerCount(std::uint16_t /*unused*/) const override {
        return 1u;
    }

    dna::ActivationFunction getNeuralNetworkLayerActivationFunction(std::uint16_t /*unused*/,
                                                                    std::uint16_t /*unused*/) const override {
        return dna::ActivationFunction::linear;
    }

    rl4::ConstArrayView<float> getNeuralNetworkLayerActivationFunctionParameters(std::uint16_t /*unused*/,
                                                                                 std::uint16_t /*unused*/) const override {
        return {};
    }

    rl4::ConstArrayView<float> getNeuralNetworkLayerBiases(std::uint16_t /*unused*/, std::uint16_t /*unused*/) const override {
        return {spec.biases.data(), spec.biases.size()};
    }

    rl4::ConstArrayView<float> getNeuralNetworkLayerWeights(std::uint16_t /*unused*/, std::uint16_t /*unused*/) const override {
        return {spec.weights.data(), spec.weights.size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLJointsParameterKeys() const override {
        return {spec.mlJointsKeys.data(), spec.mlJointsKeys.size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLJointsParameterValues() const override {
        return {spec.mlJointsValues.data(), spec.mlJointsValues.size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLJointsInputIndices() const override {
        return {spec.mlJointsInputIndices.data(), spec.mlJointsInputIndices.size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLJointsOutputIndices() const override {
        return {spec.mlJointsOutputIndices.data(), spec.mlJointsOutputIndices.size()};
    }

private:
    GraphSpec spec;
};

GraphReader::~GraphReader() = default;

// Two ML types, each one WeightedSum op-set of dependency-less ops that are `outputCount` floats wide. At 65535 wide,
// 65535 + 4 ops total 65539 x 65535 floats: type 0 alone is a valid uint32 total, the rig-wide sum wraps uint32.
class TwoTypeWeightedSumReader : public dna::FakeReader {
public:
    TwoTypeWeightedSumReader(std::uint16_t opCountType0, std::uint16_t opCountType1, std::uint32_t outputCount) :
        activeType0(opCountType0),
        activeType1(opCountType1),
        wsParams{outputCount} {
        std::iota(activeType0.begin(), activeType0.end(), static_cast<std::uint16_t>(0u));
        std::iota(activeType1.begin(), activeType1.end(), static_cast<std::uint16_t>(0u));
    }

    ~TwoTypeWeightedSumReader();

    std::uint16_t getRawControlCount() const override {
        return 1u;
    }

    std::uint16_t getMLControlCount() const override {
        return 1u;
    }

    std::uint16_t getLODCount() const override {
        return 1u;
    }

    std::uint16_t getMLTypeCount() const override {
        return 2u;
    }

    std::uint16_t getMLOperationSetCount(std::uint16_t /*unused*/) const override {
        return 1u;
    }

    std::uint16_t getMLOperationCount(std::uint16_t mlTypeIndex, std::uint16_t /*unused*/) const override {
        return static_cast<std::uint16_t>((mlTypeIndex == 0u) ? activeType0.size() : activeType1.size());
    }

    dna::MachineLearnedBehaviorOperationType getMLOperationType(std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/) const override {
        return dna::MachineLearnedBehaviorOperationType::WeightedSum;
    }

    rl4::ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/,
                                                                std::uint16_t /*unused*/) const override {
        return {wsParams.data(), wsParams.size()};
    }

    rl4::ConstArrayView<std::uint16_t> getMLOperationIndicesForLOD(std::uint16_t mlTypeIndex,
                                                                   std::uint16_t /*unused*/,
                                                                   std::uint16_t /*unused*/) const override {
        const auto& active = (mlTypeIndex == 0u) ? activeType0 : activeType1;
        return {active.data(), active.size()};
    }

private:
    std::vector<std::uint16_t> activeType0;
    std::vector<std::uint16_t> activeType1;
    std::vector<std::uint32_t> wsParams;
};

TwoTypeWeightedSumReader::~TwoTypeWeightedSumReader() = default;

class MLBWideGraphTest : public ::testing::TestWithParam<rl4::CalculationType> {
protected:
    rl4::MachineLearnedBehaviorEvaluator::Pointer createEvaluator(const dna::Reader& reader) {
        rl4::Configuration config = {};
        config.calculationType = GetParam();
        auto meta = rl4::RigMetadata::create(config, &reader, &memRes);
        return rl4::ml::cpu::Factory::create(config, meta.get(), &reader, &memRes);
    }

    rl4::ControlsInputInstance::Pointer createInputs(std::uint16_t rawControlCount, std::uint16_t mlControlCount) {
        auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0, rawControlCount, 0, mlControlCount, 0);
        rl4::Vector<rl4::ControlInitializer> initialValues;
        return inputInstanceFactory(initialValues, &memRes);
    }

    // Builds the graph, feeds every raw control `input`, evaluates once and returns the control buffer (empty on rejection).
    rl4::Vector<float> evaluateGraph(const GraphSpec& spec, float input) {
        GraphReader reader{spec};
        auto evaluator = createEvaluator(reader);
        if (!evaluator) {
            return rl4::Vector<float>{&memRes};
        }
        auto inputs = createInputs(spec.rawControlCount, spec.mlControlCount);
        auto inputBuffer = inputs->getInputBuffer();
        std::fill(inputBuffer.begin(), inputBuffer.begin() + spec.rawControlCount, input);
        auto outputs = evaluator->createInstance(&memRes);
        evaluator->calculate(inputs.get(), outputs.get(), 0u);
        return rl4::Vector<float>{inputBuffer.begin(), inputBuffer.end(), &memRes};
    }

    bool isRejected(const GraphSpec& spec) {
        GraphReader reader{spec};
        return createEvaluator(reader) == nullptr;
    }

protected:
    pma::AlignedMemoryResource memRes;
};

}  // namespace

// 32767 gathered inputs into a one-output layer need a 65534-float work buffer, the widest that fits the uint16 per-op
// width; the graph builds and computes sum(inputs) + bias exactly.
TEST_P(MLBWideGraphTest, InputAxisAtLimitComputes) {
    static constexpr std::uint16_t inputCount = 32767u;
    GraphReader reader{GraphSpec::singleLayer(inputCount, 1u)};
    auto evaluator = createEvaluator(reader);
    ASSERT_NE(evaluator, nullptr);

    auto inputs = createInputs(inputCount, 1u);
    auto inputBuffer = inputs->getInputBuffer();
    std::fill(inputBuffer.begin(), inputBuffer.begin() + inputCount, 1.0f / 1024.0f);
    auto outputs = evaluator->createInstance(&memRes);
    evaluator->calculate(inputs.get(), outputs.get(), 0u);
    // 32767 x 2^-10 is exactly representable and the linear kernel accumulates in float, so the sum is exact.
    ASSERT_NEAR(inputBuffer[inputCount], static_cast<float>(inputCount) / 1024.0f + GraphSpec::bias(0u), 1e-3f);
}

// 32769 inputs (and the 32768 boundary itself) need 65538 / 65536 floats, which do not fit the per-op width; the
// factory rejects the graph rather than narrowing the width.
TEST_P(MLBWideGraphTest, InputAxisBeyondLimitRejected) {
    GraphReader trigger{GraphSpec::singleLayer(32769u, 1u)};
    ASSERT_EQ(createEvaluator(trigger), nullptr);
    GraphReader boundary{GraphSpec::singleLayer(32768u, 1u)};
    ASSERT_EQ(createEvaluator(boundary), nullptr);
}

// 32764 output rows (already a multiple of the 4-row padding) from one input; work buffer 65528 floats.
TEST_P(MLBWideGraphTest, OutputAxisAtLimitComputes) {
    static constexpr std::uint16_t outputCount = 32764u;
    GraphReader reader{GraphSpec::singleLayer(1u, outputCount)};
    auto evaluator = createEvaluator(reader);
    ASSERT_NE(evaluator, nullptr);

    auto inputs = createInputs(1u, outputCount);
    auto inputBuffer = inputs->getInputBuffer();
    inputBuffer[0] = 0.5f;
    auto outputs = evaluator->createInstance(&memRes);
    evaluator->calculate(inputs.get(), outputs.get(), 0u);
    for (std::size_t i = {}; i < outputCount; ++i) {
        ASSERT_NEAR(inputBuffer[1u + i], 0.5f + GraphSpec::bias(i), 1e-3f) << "output " << i;
    }
}

// 32769 output rows pad to 32772 and need 65544 floats, past the per-op width.
TEST_P(MLBWideGraphTest, OutputAxisBeyondLimitRejected) {
    GraphReader trigger{GraphSpec::singleLayer(1u, 32769u)};
    ASSERT_EQ(createEvaluator(trigger), nullptr);
}

// Cumulative op offsets past uint16 (32768 + 32769 ops in one type): every op gets its own slot and evaluation runs.
TEST_P(MLBWideGraphTest, CumulativeOperationOffsetsBeyondUint16Evaluate) {
    ManyWeightedSumOpsReader reader;
    auto evaluator = createEvaluator(reader);
    ASSERT_NE(evaluator, nullptr);
    ASSERT_EQ(evaluator->getMLOperationSetCount(0u), 2u);
    ASSERT_EQ(evaluator->getMLOperationCount(0u, 0u), kManyOpsSet0);
    ASSERT_EQ(evaluator->getMLOperationCount(0u, 1u), kManyOpsSet1);

    auto outputs = evaluator->createInstance(&memRes);
    ASSERT_EQ(outputs->getMLOperationSetCount(0u), 2u);
    ASSERT_EQ(outputs->getMLOperationCount(0u, 0u), kManyOpsSet0);
    ASSERT_EQ(outputs->getMLOperationCount(0u, 1u), kManyOpsSet1);
    // Every op owns a distinct 2-float slot laid out contiguously in one work buffer.
    const auto* instance = static_cast<const rl4::ml::cpu::OutputInstance*>(outputs.get());
    const auto ptrs = instance->getWorkBufferPtrs(0u);
    const std::size_t totalOps = kManyOpsSet0 + kManyOpsSet1;
    ASSERT_EQ(ptrs.size(), totalOps);
    ASSERT_EQ(instance->getWorkBufferOffsetsPerOperationSet(0u)[2], totalOps);
    for (std::size_t i = 1u; i < ptrs.size(); ++i) {
        ASSERT_EQ(ptrs[i] - ptrs[i - 1u], 2) << "op " << i;
    }

    auto inputs = createInputs(1u, 1u);
    evaluator->calculate(inputs.get(), outputs.get(), 0u);
}

// Gather width is bounded by the network's work-buffer half (4 floats for a one-input network): one input computes,
// nine valid inputs are rejected.
TEST_P(MLBWideGraphTest, GatherWiderThanNetworkInputRejected) {
    auto control = GraphSpec::singleLayer(1u, 1u);
    const auto out = evaluateGraph(control, 0.5f);
    ASSERT_EQ(out.size(), 2u);
    ASSERT_NEAR(out[1], 0.5f, 1e-3f);
    auto trigger = GraphSpec::singleLayer(1u, 1u);
    trigger.rawControlCount = 9u;
    trigger.params[0][0].assign({0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u});  // valid controls, wider than the network
    trigger.params[2][0] = {9u};                                        // scatter stays on the (shifted) ML control
    ASSERT_TRUE(isRejected(trigger));
}

// A per-LOD row view must fit the layer: one row computes, five rows on a one-row (four padded) layer is rejected.
TEST_P(MLBWideGraphTest, LayerRowViewBeyondLayerRowsRejected) {
    auto control = GraphSpec::singleLayer(1u, 1u);
    control.params[1][0] = {0u, 1u};  // net 0, LOD 0 keeps 1 row
    const auto out = evaluateGraph(control, 0.5f);
    ASSERT_EQ(out.size(), 2u);
    ASSERT_NEAR(out[1], 0.5f, 1e-3f);
    auto trigger = GraphSpec::singleLayer(1u, 1u);
    trigger.params[1][0] = {0u, 5u};
    ASSERT_TRUE(isRejected(trigger));
}

// Scatter destinations are bounded by the control count: destination 1 of a two-control rig computes, 2 and 1024 are
// rejected.
TEST_P(MLBWideGraphTest, ScatterDestinationBeyondControlsRejected) {
    const auto control = GraphSpec::singleLayer(1u, 1u);  // scatters to control 1 of {0, 1}
    ASSERT_FALSE(isRejected(control));
    auto two = GraphSpec::singleLayer(1u, 1u);
    two.params[2][0] = {2u};
    ASSERT_TRUE(isRejected(two));
    auto far = GraphSpec::singleLayer(1u, 1u);
    far.params[2][0] = {1024u};
    ASSERT_TRUE(isRejected(far));
}

// One network output with two valid scatter destinations, plus quaternion-W ML joints metadata mapping the SECOND
// destination onto a qw attribute. Default initialization covers min(destinations, outputs): the graph builds and only
// the first destination is written.
TEST_P(MLBWideGraphTest, UnequalScatterAndOutputCountsFollowMinSemantics) {
    auto spec = GraphSpec::singleLayer(1u, 1u);
    spec.mlControlCount = 2u;
    spec.params[2][0] = {1u, 2u};  // two destinations for one output
    spec.mlJointsKeys = {static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationType)};
    spec.mlJointsValues = {static_cast<std::uint16_t>(dna::RotationRepresentation::Quaternion)};
    spec.mlJointsInputIndices = {1u, 2u};    // both destinations are ML joint outputs...
    spec.mlJointsOutputIndices = {0u, 16u};  // ...and the second one is a qw attribute (16 % 10 == 6)
    const auto out = evaluateGraph(spec, 0.5f);
    ASSERT_EQ(out.size(), 3u);
    ASSERT_NEAR(out[1], 0.5f, 1e-3f);
    ASSERT_EQ(out[2], 0.0f);
}

// MLP at index 0 plus a Gather at index 1 inside the MLP set, with active selector 1 kept. The non-MLP op stays a
// placeholder so serialized and compiled indices agree; evaluation matches the plain graph.
TEST_P(MLBWideGraphTest, MixedKindOperationSetKeepsSerializedSelectors) {
    using OpType = dna::MachineLearnedBehaviorOperationType;
    auto spec = GraphSpec::singleLayer(1u, 1u);
    spec.opTypes[1] = {OpType::MLP, OpType::Gather};
    spec.params[1] = {{0u}, {0u}};
    spec.depSets[1] = {{0u}, {}};
    spec.depOps[1] = {{0u}, {}};
    spec.activeOps[1] = {0u, 1u};
    const auto out = evaluateGraph(spec, 0.5f);
    ASSERT_EQ(out.size(), 2u);
    ASSERT_NEAR(out[1], 0.5f, 1e-3f);
}

// Active selectors are bounded by the compiled operation count: selector 0 of a one-operation graph computes,
// selector 1 is rejected.
TEST_P(MLBWideGraphTest, ActiveSelectorBeyondOperationCountRejected) {
    ASSERT_FALSE(isRejected(GraphSpec::singleLayer(1u, 1u)));
    auto trigger = GraphSpec::singleLayer(1u, 1u);
    trigger.activeOps[1] = {1u};
    ASSERT_TRUE(isRejected(trigger));
}

// Two ML types of 65535-wide WeightedSum ops whose rig-wide total (65539 x 65535 floats) exceeds uint32 while each
// type's total does not; rejected. The same graph two floats wide builds, lays out every slot and evaluates.
TEST_P(MLBWideGraphTest, WeightedSumGlobalTotalBeyondUint32Rejected) {
    TwoTypeWeightedSumReader trigger{65535u, 4u, 65535u};
    ASSERT_EQ(createEvaluator(trigger), nullptr);

    TwoTypeWeightedSumReader control{65535u, 4u, 2u};
    auto evaluator = createEvaluator(control);
    ASSERT_NE(evaluator, nullptr);
    auto outputs = evaluator->createInstance(&memRes);
    ASSERT_EQ(outputs->getMLOperationCount(0u, 0u), 65535u);
    ASSERT_EQ(outputs->getMLOperationCount(1u, 0u), 4u);
    // Both types share one work buffer: type 1's first slot follows type 0's 65535 two-float slots.
    const auto* instance = static_cast<const rl4::ml::cpu::OutputInstance*>(outputs.get());
    ASSERT_EQ(instance->getWorkBufferPtrs(0u).size(), 65535u);
    ASSERT_EQ(instance->getWorkBufferPtrs(1u).size(), 4u);
    ASSERT_EQ(instance->getWorkBufferPtrs(1u)[0] - instance->getWorkBufferPtrs(0u)[0], 2 * 65535);
    auto inputs = createInputs(1u, 1u);
    evaluator->calculate(inputs.get(), outputs.get(), 0u);
}

INSTANTIATE_TEST_SUITE_P(MLBWideGraphTestSuite,
                         MLBWideGraphTest,
                         ::testing::Values(rl4::CalculationType::AVX,
                                           rl4::CalculationType::SSE,
                                           rl4::CalculationType::NEON,
                                           rl4::CalculationType::Scalar));

