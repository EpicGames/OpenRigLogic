// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef _MSC_VER
    #pragma warning(disable : 4503)
#endif

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/CPUMachineLearnedBehaviorOutputInstance.h"
#include "riglogic/ml/cpu/MLBehaviorValidator.h"
#include "riglogic/ml/cpu/NeuralNet.h"
#include "riglogic/ml/cpu/Operation.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/SIMD.h"
#include "riglogic/types/LODSpec.h"

#include <algorithm>

// The shared ML-behavior validator (DNA factory path + snapshot load() path) checks the constructed OperationSet chain
// and its work-buffer sizing against the rig's control/mesh/LOD counts; each test perturbs one field and asserts rejection.

namespace mlbehaviorvalidatortest {

using OpSet = rl4::ml::cpu::MLPOperationSet<float, trimd::scalar::F256, trimd::scalar::F128>;

template<std::size_t N>
rl4::Vector<std::uint16_t> makeU16(const std::uint16_t (&values)[N], rl4::MemoryResource* memRes) {
    return rl4::Vector<std::uint16_t>{values, values + N, memRes};
}

// A container holding the whole ML-behavior runtime state so a test can perturb any field before validating.
struct MLBehaviorHarness {
    explicit MLBehaviorHarness(rl4::MemoryResource* memRes) :
        lods{memRes},
        mlOperations{memRes},
        bufferSizes{memRes},
        meshRegionCount{1u} {
    }

    rl4::Matrix<rl4::LODSpec<std::uint16_t>> lods;
    rl4::Matrix<rl4::ml::cpu::OperationSet::Pointer> mlOperations;
    rl4::Vector<rl4::Matrix<std::uint16_t>> bufferSizes;
    std::uint32_t meshRegionCount;

    OpSet* opSet() const {
        return static_cast<OpSet*>(mlOperations[0][0].get());
    }

    bool validate(const rl4::RigMetadata& meta) const {
        return rl4::ml::cpu::MLBehaviorValidator::validate(lods, mlOperations, bufferSizes, meshRegionCount, meta);
    }
};

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes,
                              std::uint16_t lodCount,
                              std::uint16_t controlCount,
                              std::uint16_t mlTypeCount = 1u) {
    rl4::RigMetadata meta{memRes};
    meta.lodCount = lodCount;
    meta.rawControlCount = controlCount;  // controlInputCount = raw + psd + ml + rbf; the rest stay zero
    meta.mlTypeCount = mlTypeCount;
    return meta;
}

// One well-formed layer with builder-shaped geometry (rows padded to 4 with block-8/pad-4 per-LOD views, cols rounded
// DOWN to 4/8): the validator bounds the kernel's block-strided walk, which a hand-rolled unaligned view would fail.
rl4::ml::cpu::NeuralNetLayer makeLayer(rl4::MemoryResource* memRes,
                                       std::uint16_t lodCount,
                                       std::uint32_t liveRows,
                                       std::uint32_t paddedRows,
                                       std::uint32_t paddedCols) {
    rl4::ml::cpu::NeuralNetLayer layer{memRes};
    layer.activationFunction = dna::ActivationFunction::linear;
    layer.weights.original = rl4::Extent{liveRows, paddedCols};
    layer.weights.padded = rl4::Extent{paddedRows, paddedCols};
    layer.weights.cols = rl4::PaddedBlockView{paddedCols, paddedCols - (paddedCols % 4u), paddedCols - (paddedCols % 8u)};
    layer.weights.rows.resize(lodCount);
    for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
        layer.weights.rows[lod] = rl4::PaddedBlockView{liveRows, paddedRows, 8u, 4u};
    }
    layer.weightOffset = 0u;
    layer.biasOffset = static_cast<std::size_t>(paddedRows) * static_cast<std::size_t>(paddedCols);
    return layer;
}

// One MLP op: one layer (so it is not a placeholder), one gathered input, two scattered outputs. The ping-pong halves
// hold the PADDED row extent, mirroring the factory (the kernel writes whole 4/8-row blocks).
rl4::ml::cpu::MLPOperationData makeMLPOp(rl4::MemoryResource* memRes, std::uint16_t lodCount, std::uint16_t outputCount) {
    const std::uint16_t gatherIndices[] = {0u};       // one control input; keeps gather within halfSize
    const std::uint16_t scatterIndices[] = {0u, 1u};  // two control outputs
    const std::uint32_t paddedRows = ((static_cast<std::uint32_t>(outputCount) + 3u) / 4u) * 4u;
    const std::uint32_t paddedCols = 1u;
    rl4::ml::cpu::MLPOperationData op{memRes};
    op.neuralNet.maskIndex = static_cast<std::uint32_t>(-1);  // kNoMask
    op.neuralNet.layers.push_back(makeLayer(memRes, lodCount, outputCount, paddedRows, paddedCols));
    op.neuralNet.flatData.resize<float>(static_cast<std::size_t>(paddedRows) * paddedCols + paddedRows);
    op.inputControlIndices = makeU16(gatherIndices, memRes);
    op.outputControlIndices = makeU16(scatterIndices, memRes);
    op.outputCountsPerLOD = rl4::Vector<std::uint16_t>{lodCount, outputCount, memRes};
    op.defaultValues = rl4::Vector<float>{outputCount, 0.0f, memRes};
    op.tailZeroCount = 0u;
    return op;
}

// Assembles a harness with a single MLP op-set holding one op, all counts internally consistent.
MLBehaviorHarness makeValidHarness(rl4::MemoryResource* memRes, std::uint16_t lodCount, std::uint16_t outputCount) {
    // halfSize == the padded row extent, mirroring the factory's work-buffer sizing.
    const std::uint16_t bufferSizeValues[] = {static_cast<std::uint16_t>(((outputCount + 3u) / 4u) * 4u * 2u)};
    const std::uint16_t activeOp[] = {0u};

    MLBehaviorHarness h{memRes};

    // bufferSizes[type][set][op]
    h.bufferSizes.resize(1u);
    h.bufferSizes[0].resize(1u);
    h.bufferSizes[0][0] = makeU16(bufferSizeValues, memRes);

    // lods[type][set] - LODSpec has no default ctor, so build the inner vector from an explicit element.
    h.lods.resize(1u);
    h.lods[0] = rl4::Vector<rl4::LODSpec<std::uint16_t>>{1u, rl4::LODSpec<std::uint16_t>{memRes}, memRes};
    h.lods[0][0].count = 1u;
    h.lods[0][0].indicesPerLOD.resize(lodCount);
    for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
        h.lods[0][0].indicesPerLOD[lod] = makeU16(activeOp, memRes);
    }

    // mlOperations[type][set]
    rl4::Vector<rl4::ml::cpu::MLPOperationData> ops{memRes};
    ops.push_back(makeMLPOp(memRes, lodCount, outputCount));
    auto opSet = rl4::UniqueInstance<OpSet, rl4::ml::cpu::OperationSet>::with(memRes).create(std::move(ops), false, memRes);
    h.mlOperations.resize(1u);
    h.mlOperations[0].push_back(std::move(opSet));

    return h;
}

// Appends an MLP op-set of `opCount` layer-less placeholder ops (execute() and validate() skip them) to ML type `typeIdx`
// (created if absent), each claiming `bufferSize` floats: cardinality and total-size checks without real networks.
void appendPlaceholderSet(MLBehaviorHarness& h,
                          std::size_t typeIdx,
                          std::size_t opCount,
                          std::uint16_t bufferSize,
                          std::uint16_t lodCount,
                          rl4::MemoryResource* memRes) {
    while (h.bufferSizes.size() <= typeIdx) {
        h.bufferSizes.push_back(rl4::Matrix<std::uint16_t>{memRes});
        h.mlOperations.push_back(rl4::Vector<rl4::ml::cpu::OperationSet::Pointer>{memRes});
        h.lods.push_back(rl4::Vector<rl4::LODSpec<std::uint16_t>>{memRes});
    }
    h.bufferSizes[typeIdx].push_back(rl4::Vector<std::uint16_t>{opCount, bufferSize, memRes});

    rl4::LODSpec<std::uint16_t> lodSpec{memRes};
    lodSpec.count = static_cast<std::uint16_t>(std::min<std::size_t>(opCount, 0xFFFFu));
    lodSpec.indicesPerLOD.resize(lodCount);  // no active ops - nothing to execute, nothing to index
    h.lods[typeIdx].push_back(std::move(lodSpec));

    rl4::Vector<rl4::ml::cpu::MLPOperationData> ops{memRes};
    ops.reserve(opCount);
    for (std::size_t i = {}; i < opCount; ++i) {
        rl4::ml::cpu::MLPOperationData placeholder{memRes};
        placeholder.neuralNet.maskIndex = static_cast<std::uint32_t>(-1);  // kNoMask
        ops.push_back(std::move(placeholder));
    }
    h.mlOperations[typeIdx].push_back(
        rl4::UniqueInstance<OpSet, rl4::ml::cpu::OperationSet>::with(memRes).create(std::move(ops), false, memRes));
}

using WSOpSet = rl4::ml::cpu::WeightedSumOperationSet<64, trimd::scalar::F256, trimd::scalar::F128>;

// Appends a WeightedSum op-set of `opCount` dependency-less ops, each `outputCount` wide, to ML type `typeIdx`.
// WeightedSum buffers are exactly outputCount floats, so this is the op kind that can demand 65535 floats per slot.
void appendWeightedSumSet(MLBehaviorHarness& h,
                          std::size_t typeIdx,
                          std::size_t opCount,
                          std::uint16_t outputCount,
                          std::uint16_t lodCount,
                          rl4::MemoryResource* memRes) {
    while (h.bufferSizes.size() <= typeIdx) {
        h.bufferSizes.push_back(rl4::Matrix<std::uint16_t>{memRes});
        h.mlOperations.push_back(rl4::Vector<rl4::ml::cpu::OperationSet::Pointer>{memRes});
        h.lods.push_back(rl4::Vector<rl4::LODSpec<std::uint16_t>>{memRes});
    }
    h.bufferSizes[typeIdx].push_back(rl4::Vector<std::uint16_t>{opCount, outputCount, memRes});

    rl4::LODSpec<std::uint16_t> lodSpec{memRes};
    lodSpec.count = static_cast<std::uint16_t>(std::min<std::size_t>(opCount, 0xFFFFu));
    lodSpec.indicesPerLOD.resize(lodCount);
    h.lods[typeIdx].push_back(std::move(lodSpec));

    rl4::Vector<rl4::ml::cpu::WeightedSumOperationData> ops{memRes};
    ops.reserve(opCount);
    for (std::size_t i = {}; i < opCount; ++i) {
        rl4::ml::cpu::WeightedSumOperationData op{memRes};
        op.outputCount = outputCount;
        ops.push_back(std::move(op));
    }
    h.mlOperations[typeIdx].push_back(
        rl4::UniqueInstance<WSOpSet, rl4::ml::cpu::OperationSet>::with(memRes).create(std::move(ops), memRes));
}

}  // namespace mlbehaviorvalidatortest

TEST(MLBehaviorValidatorTest, AcceptsWellFormedData) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_TRUE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsIntermediateDefaultsNarrowerThanOutputs) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    // An intermediate op (no scatter indices) with full-width defaults is valid...
    h.opSet()->ops[0].outputControlIndices.clear();
    ASSERT_TRUE(h.validate(meta));
    // ...but narrower deserialized defaults would leak the previous frame's work buffer into the
    // WeightedSum consumer on the mask-zero path.
    h.opSet()->ops[0].defaultValues.pop_back();
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsOperationCountBeyondOps) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // getMLOperationCount() returns this scalar verbatim as the public single-op calculate() loop bound.
    h.lods[0][0].count = 2u;  // the set holds one op
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsMisalignedWeightOffset) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // Slack so the span checks still pass; the offset itself breaks the aligned-load requirement.
    auto& net = h.opSet()->ops[0].neuralNet;
    net.flatData.resize<float>(net.flatData.size<float>() + 8ul);
    net.layers[0].weightOffset = 1u;
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsScatterIndexBeyondControlCount) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 1u);  // controlInputCount 1, but scatter targets index 1
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsGatherIndexBeyondControlCount) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // Point the gather at a control index past the buffer while keeping the list within the work-buffer half.
    const std::uint16_t badGather[] = {0u, 9u};
    h.opSet()->ops[0].inputControlIndices = mlbehaviorvalidatortest::makeU16(badGather, &memRes);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);  // controlInputCount 4, gather index 9 is out of range
    ASSERT_FALSE(h.validate(meta));
}

// Nine valid control indices gathered into an op whose ping-pong half holds four floats: the index domain is fine, it
// is the COUNT that overruns buf1 in execute()'s gather loop.
TEST(MLBehaviorValidatorTest, RejectsGatherWidthBeyondWorkBufferHalf) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 16u);  // every index below is a valid control
    ASSERT_TRUE(h.validate(meta));
    const std::uint16_t nineInputs[] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u};
    h.opSet()->ops[0].inputControlIndices = mlbehaviorvalidatortest::makeU16(nineInputs, &memRes);
    ASSERT_FALSE(h.validate(meta));
    const std::uint16_t fourInputs[] = {0u, 1u, 2u, 3u};  // exactly the half width
    h.opSet()->ops[0].inputControlIndices = mlbehaviorvalidatortest::makeU16(fourInputs, &memRes);
    ASSERT_TRUE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsOutputWidthBeyondWorkBufferHalf) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // outputCountsPerLOD claims 8 outputs but the ping-pong half only holds 2.
    h.opSet()->ops[0].outputCountsPerLOD = rl4::Vector<std::uint16_t>{2u, static_cast<std::uint16_t>(8u), &memRes};
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsMaskIndexBeyondMeshRegionCount) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // Enable masks and point at a region the mask buffer (meshRegionCount == 1) does not have.
    h.opSet()->hasMasks = true;
    h.opSet()->ops[0].neuralNet.maskIndex = 5u;
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsActiveOpIndexBeyondOpCount) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // A LOD list references op 3, but the set holds only one op.
    const std::uint16_t badActive[] = {3u};
    h.lods[0][0].indicesPerLOD[0] = mlbehaviorvalidatortest::makeU16(badActive, &memRes);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsFewerLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 1u, 2u);  // one LOD built
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);   // metadata expects two
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsTailZeroOverflow) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // outputCount (2) + tailZeroCount would memset past the full buffer (bufferSize 4).
    h.opSet()->ops[0].tailZeroCount = 8u;
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsFewerLayerRowViewsThanLODCount) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // A layer's per-LOD row views (indexed by lod in execute()) are shorter than the LOD count.
    h.opSet()->ops[0].neuralNet.layers[0].weights.rows.resize(1u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsUnknownActivationFunction) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // An out-of-range activation enum would fall through the execute() switch and propagate stale buffer data.
    h.opSet()->ops[0].neuralNet.layers[0].activationFunction = static_cast<dna::ActivationFunction>(99);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsLeakyReLUWithoutParameter) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // leakyrelu dereferences activationFunctionParameters[0], which is empty here.
    h.opSet()->ops[0].neuralNet.layers[0].activationFunction = dna::ActivationFunction::leakyrelu;
    h.opSet()->ops[0].neuralNet.layers[0].activationFunctionParameters.clear();
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, AcceptsLeakyReLUWithParameter) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // leakyrelu with its slope parameter present is well-formed.
    auto& layer = h.opSet()->ops[0].neuralNet.layers[0];
    layer.activationFunction = dna::ActivationFunction::leakyrelu;
    layer.activationFunctionParameters = rl4::Vector<float>{1u, 0.01f, &memRes};
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_TRUE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsWeightOffsetPastFlatData) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // weightOffset points past the end of flatData, so the padded weight span reads out of bounds.
    h.opSet()->ops[0].neuralNet.layers[0].weightOffset = 1000u;
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsWeightSpanBeyondFlatData) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // Padded weight matrix (rows*cols) claims more floats than flatData holds, even at offset 0.
    h.opSet()->ops[0].neuralNet.layers[0].weights.padded = rl4::Extent{64u, 64u};
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsBiasOffsetPastFlatData) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // biasOffset + paddedRows biases spill past flatData.
    h.opSet()->ops[0].neuralNet.layers[0].biasOffset = 1000u;
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsColStrideBeyondPaddedCols) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // The input block view stride exceeds the padded column count that bounds the pointer walk.
    auto& layer = h.opSet()->ops[0].neuralNet.layers[0];
    layer.weights.cols = rl4::PaddedBlockView{99u, 99u, 99u};
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

TEST(MLBehaviorValidatorTest, RejectsRowStrideBeyondPaddedRows) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // A per-LOD output block view walks past the padded row count.
    auto& layer = h.opSet()->ops[0].neuralNet.layers[0];
    layer.weights.rows[0] = rl4::PaddedBlockView{99u, 99u, 99u};
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_FALSE(h.validate(meta));
}

// 32768 + 32769 ops in one type push the running per-set op offsets past 65535; the graph is valid and OutputInstance
// lays it out exactly.
TEST(MLBehaviorValidatorTest, AcceptsCumulativeOperationOffsetsBeyondUint16) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 0u, 32768u, 2u, 2u, &memRes);
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 0u, 32769u, 2u, 2u, &memRes);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_TRUE(h.validate(meta));

    rl4::ml::cpu::OutputInstance instance{h.bufferSizes, h.meshRegionCount, &memRes};
    ASSERT_EQ(instance.getMLOperationSetCount(0u), 3u);
    ASSERT_EQ(instance.getMLOperationCount(0u, 0u), 1u);
    ASSERT_EQ(instance.getMLOperationCount(0u, 1u), 32768u);
    ASSERT_EQ(instance.getMLOperationCount(0u, 2u), 32769u);
    const auto offsets = instance.getWorkBufferOffsetsPerOperationSet(0u);
    ASSERT_EQ(offsets.size(), 4u);
    ASSERT_EQ(offsets[3], 1u + 32768u + 32769u);
    const auto ptrs = instance.getWorkBufferPtrs(0u);
    ASSERT_EQ(ptrs.size(), 1u + 32768u + 32769u);
    // Each placeholder owns 2 floats behind the first op's 8-float buffer (outputCount 2 -> 4 padded rows x 2 halves).
    ASSERT_EQ(ptrs[ptrs.size() - 1u] - ptrs[0], static_cast<std::ptrdiff_t>(8u + 2u * (32768u + 32769u - 1u)));
}

// Op indices are uint16 in Dependency, in the per-LOD active lists and in getMLOperationCount(), so a set cannot hold
// more than 65535 ops; a deserialized bufferSizes row longer than that must be rejected rather than narrowed.
TEST(MLBehaviorValidatorTest, RejectsOperationCountBeyondUint16) {
    pma::AlignedMemoryResource memRes;
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    auto atLimit = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    mlbehaviorvalidatortest::appendPlaceholderSet(atLimit, 0u, 65535u, 0u, 2u, &memRes);
    ASSERT_TRUE(atLimit.validate(meta));
    auto beyondLimit = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    // The helper caps the LODSpec count at 65535; a count BELOW the op total is not a rejection reason (only a count
    // above it is), so the uint16 op-count cap is the sole check that can fail this graph.
    mlbehaviorvalidatortest::appendPlaceholderSet(beyondLimit, 0u, 65536u, 0u, 2u, &memRes);
    ASSERT_FALSE(beyondLimit.validate(meta));
}

// Set indices are likewise uint16 (Dependency::opSetIdx, getMLOperationSetCount()).
TEST(MLBehaviorValidatorTest, RejectsOperationSetCountBeyondUint16) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    for (std::size_t i = 1u; i < 65535u; ++i) {
        mlbehaviorvalidatortest::appendPlaceholderSet(h, 0u, 0u, 0u, 2u, &memRes);
    }
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    ASSERT_TRUE(h.validate(meta));
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 0u, 0u, 0u, 2u, &memRes);
    ASSERT_FALSE(h.validate(meta));
}

// The workBuffer is one allocation for the whole rig, so the ceiling applies to the sum over all types: two types each
// well under kMaxWorkBufferFloats (600 x 65535 = 39.3M floats) are rejected together (78.6M).
TEST(MLBehaviorValidatorTest, RejectsWorkBufferTotalBeyondCeilingAcrossTypes) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 1u, 600u, 65535u, 2u, &memRes);
    ASSERT_TRUE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u)));
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 2u, 600u, 65535u, 2u, &memRes);
    ASSERT_FALSE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 3u)));
}

// Exact boundary of the ceiling: a total of precisely kMaxWorkBufferFloats is accepted, one float more is rejected.
TEST(MLBehaviorValidatorTest, AcceptsWorkBufferTotalUpToCeiling) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    // Set 0 already holds 8 floats (outputCount 2 -> 4 padded rows x 2 halves) and the mask buffer one float per mesh
    // region; fill the rest up to the cap exactly.
    constexpr std::size_t ceiling = rl4::ml::cpu::kMaxWorkBufferFloats;  // constant fits size_t on every target
    const std::size_t remaining = ceiling - 8u - h.meshRegionCount;
    const std::size_t fullOps = remaining / 65535u;
    const std::uint16_t tail = static_cast<std::uint16_t>(remaining % 65535u);
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 0u, fullOps, 65535u, 2u, &memRes);
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 0u, 1u, tail, 2u, &memRes);
    ASSERT_TRUE(h.validate(meta));
    h.bufferSizes[0].back()[0] = static_cast<std::uint16_t>(tail + 1u);
    ASSERT_FALSE(h.validate(meta));
}

// A two-type WeightedSum graph whose buffers sum to exactly 2^32 + 65535 floats, which a uint32 accumulator would wrap
// to 65535. The identical graph with one-float buffers is accepted, so the rig-wide total is the only rejection reason.
TEST(MLBehaviorValidatorTest, RejectsWeightedSumGlobalTotalWrappingUint32) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);  // 8 floats in type 0
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 3u);
    mlbehaviorvalidatortest::appendWeightedSumSet(h, 1u, 65535u, 1u, 2u, &memRes);
    mlbehaviorvalidatortest::appendWeightedSumSet(h, 2u, 3u, 1u, 2u, &memRes);
    ASSERT_TRUE(h.validate(meta));

    std::fill(h.bufferSizes[1][0].begin(), h.bufferSizes[1][0].end(), static_cast<std::uint16_t>(65535u));
    h.bufferSizes[2][0][0] = 65535u;
    h.bufferSizes[2][0][1] = 65535u;
    h.bufferSizes[2][0][2] = 65528u;
    std::uint64_t total = {};
    for (const auto& perType : h.bufferSizes) {
        for (const auto& perSet : perType) {
            for (const auto size : perSet) {
                total += size;
            }
        }
    }
    ASSERT_EQ(total, (std::uint64_t{1} << 32) + 65535u);
    ASSERT_FALSE(h.validate(meta));
}

// WeightedSum variant of the cross-type ceiling: each type alone is well under the cap, together they exceed it.
TEST(MLBehaviorValidatorTest, RejectsWeightedSumTotalBeyondCeilingAcrossTypes) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    mlbehaviorvalidatortest::appendWeightedSumSet(h, 1u, 600u, 65535u, 2u, &memRes);
    ASSERT_TRUE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u)));
    mlbehaviorvalidatortest::appendWeightedSumSet(h, 2u, 600u, 65535u, 2u, &memRes);
    ASSERT_FALSE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 3u)));
}

// RigLogic iterates ML types up to the metadata count and indexes the evaluator's per-type containers with it.
TEST(MLBehaviorValidatorTest, RejectsTypeCountDisagreeingWithMetadata) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    ASSERT_TRUE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 1u)));
    ASSERT_FALSE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u)));
    mlbehaviorvalidatortest::appendPlaceholderSet(h, 1u, 1u, 2u, 2u, &memRes);
    ASSERT_FALSE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 1u)));
    ASSERT_TRUE(h.validate(mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u)));
}

// The mask buffer is sized from meshRegionCount, a raw scalar on the snapshot path; it shares the instance ceiling.
TEST(MLBehaviorValidatorTest, RejectsMeshRegionCountBeyondCeiling) {
    pma::AlignedMemoryResource memRes;
    auto h = mlbehaviorvalidatortest::makeValidHarness(&memRes, 2u, 2u);
    auto meta = mlbehaviorvalidatortest::makeMetadata(&memRes, 2u, 4u);
    h.meshRegionCount = static_cast<std::uint32_t>(rl4::ml::cpu::kMaxWorkBufferFloats - 8u);  // exactly at the cap with set 0
    ASSERT_TRUE(h.validate(meta));
    h.meshRegionCount += 1u;
    ASSERT_FALSE(h.validate(meta));
    h.meshRegionCount = 0x71717171u;  // a 7.6 GB mask buffer
    ASSERT_FALSE(h.validate(meta));
}
