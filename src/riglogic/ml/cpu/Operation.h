// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/NeuralNet.h"
#include "riglogic/ml/cpu/layers/LayerEvaluator.h"
#include "riglogic/ml/cpu/layers/LeakyReLULayerEvaluator.h"
#include "riglogic/ml/cpu/layers/LinearLayerEvaluator.h"
#include "riglogic/ml/cpu/layers/ReLULayerEvaluator.h"
#include "riglogic/ml/cpu/layers/SigmoidLayerEvaluator.h"
#include "riglogic/ml/cpu/layers/TanHLayerEvaluator.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/utils/Extd.h"
#include "riglogic/utils/Macros.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <cassert>
#include <cstring>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

namespace ml {

namespace cpu {

enum class OperationSetType : std::uint16_t {
    MLP = 0,
    WeightedSum8 = 1,
    WeightedSum16 = 2,
    WeightedSum24 = 3,
    WeightedSum32 = 4,
    WeightedSum40 = 5,
    WeightedSum48 = 6,
    WeightedSum56 = 7,
    WeightedSum64 = 8,
    WeightedSum = WeightedSum64  // default alias: largest block size
};

template<typename Archive>
void load(Archive& archive, OperationSetType& value) {
    std::uint16_t tmp = {};
    archive(tmp);
    value = static_cast<OperationSetType>(tmp);
}

template<typename Archive>
void save(Archive& archive, OperationSetType& value) {
    archive(static_cast<std::uint16_t>(value));
}

// Bounds an OperationSet validates its ops against, shared by the build() and load() paths.
// bufferSizesForType[opSetIdx][opIdx] is the op's full work-buffer width (2*halfSize for MLP, output width for
// WeightedSum); its per-set size() is the op count, which bounds cross-set dependency (opSetIdx, opIdx) pairs.
struct MLBehaviorBounds {
    std::size_t controlInputCount;
    std::size_t meshRegionCount;
    std::size_t lodCount;
    std::size_t opSetIndex;                           // index of the op-set being validated within its ML type
    const Matrix<std::uint16_t>* bufferSizesForType;  // [opSetIdx][opIdx] -> work-buffer width, whole ML type
};

struct OperationSet {
    using Pointer = UniqueInstance<OperationSet>::PointerType;

    virtual ~OperationSet();

    virtual void execute(ConstArrayView<std::uint16_t> activeOpIndices,
                         std::uint16_t lod,
                         ConstArrayView<float> masks,
                         ArrayView<float> inputBuffer,
                         std::size_t opSetWorkBufferOffset,
                         ConstArrayView<float*> workBufferPtrs,
                         ConstArrayView<std::uint16_t> workBufferHalfSizes,
                         ConstArrayView<std::uint32_t> workBufferOffsetsPerOperationSet) const = 0;

    virtual OperationSetType getType() const = 0;

    // Reject data that would make execute() read or write out of bounds. Returns false on the first violation.
    virtual bool validate(const MLBehaviorBounds& bounds) const = 0;

    virtual void load(BoundedInputArchive& archive) = 0;
    virtual void save(terse::BinaryOutputArchive<BoundedIOStream>& archive) = 0;
};

struct Dependency {
    std::uint16_t opSetIdx;
    std::uint16_t opIdx;

    template<class Archive>
    void serialize(Archive& archive) {
        archive(opSetIdx, opIdx);
    }
};

// A cross-op-set (opSetIdx, opIdx) dependency must address a real op in the same ML type.
inline bool isValidDependency(const Dependency& dep, const Matrix<std::uint16_t>& bufferSizesForType) {
    return (static_cast<std::size_t>(dep.opSetIdx) < bufferSizesForType.size()) &&
           (static_cast<std::size_t>(dep.opIdx) < bufferSizesForType[dep.opSetIdx].size());
}

struct MLPOperationData {
    NeuralNet neuralNet;
    Vector<Dependency> inputDeps;  // cross-operation-set (opSetIdx, opIdx) pairs; empty if inputs come from controls
    Vector<std::uint16_t> inputControlIndices;
    Vector<std::uint16_t> outputControlIndices;
    Vector<std::uint16_t> outputCounts;        // per-dep output count for the gather phase
    Vector<std::uint16_t> outputCountsPerLOD;  // precomputed from layers.back().weights.rows[lod].size
    Vector<float> defaultValues;
    // Padding floats a WeightedSum consumer may over-read past this op's outputs; 0 for well-formed DNA.
    std::uint16_t tailZeroCount;

    explicit MLPOperationData(MemoryResource* memRes) :
        neuralNet{memRes},
        inputDeps{memRes},
        inputControlIndices{memRes},
        outputControlIndices{memRes},
        outputCounts{memRes},
        outputCountsPerLOD{memRes},
        defaultValues{memRes},
        tailZeroCount{} {
    }
};

struct WeightedSumOperationData {
    Vector<Dependency> inputDeps;  // cross-operation-set (opSetIdx, opIdx) pairs
    Vector<float> weights;
    std::uint16_t outputCount;

    explicit WeightedSumOperationData(MemoryResource* memRes) :
        inputDeps{memRes},
        weights{memRes},
        outputCount{} {
    }
};

struct OperationSetData {
    OperationSetType type;
    Vector<WeightedSumOperationData> wsOps;
    Vector<MLPOperationData> mlpOps;
    bool hasMasks;

    explicit OperationSetData(MemoryResource* memRes) :
        type{},
        wsOps{memRes},
        mlpOps{memRes},
        hasMasks{false} {
    }

    bool empty() const {
        // A set holding only layer-less placeholder MLP ops (kept for op-index alignment) counts as empty.
        return wsOps.empty() &&
               std::all_of(mlpOps.begin(), mlpOps.end(), [](const MLPOperationData& op) { return op.neuralNet.layers.empty(); });
    }
};

template<typename T, typename TF256, typename TF128>
struct MLPOperationSet : OperationSet {
    Vector<MLPOperationData> ops;
    Vector<const T*> cachedLayerDataPtrs;
    bool hasMasks;

    explicit MLPOperationSet(MemoryResource* memRes) :
        ops{memRes},
        cachedLayerDataPtrs{memRes},
        hasMasks{false} {
    }

    explicit MLPOperationSet(Vector<MLPOperationData>&& ops_, bool hasMasks_, MemoryResource* memRes) :
        ops{std::move(ops_)},
        cachedLayerDataPtrs{memRes},
        hasMasks{hasMasks_} {
        cachedLayerDataPtrs.reserve(ops.size());
        for (const auto& op : ops) {
            cachedLayerDataPtrs.push_back(op.neuralNet.flatData.template data<T>());
        }
    }

    void execute(ConstArrayView<std::uint16_t> activeOpIndices,
                 std::uint16_t lod,
                 ConstArrayView<float> masks,
                 ArrayView<float> inputBuffer,
                 std::size_t opSetWorkBufferOffset,
                 ConstArrayView<float*> workBufferPtrs,
                 ConstArrayView<std::uint16_t> workBufferHalfSizes,
                 ConstArrayView<std::uint32_t> workBufferOffsetsPerOperationSet) const override {

        // This op's ping-pong buffer is workBufferPtrs[opSetWorkBufferOffset + opIdx], two halves of workBufferHalfSizes[same]
        // swapped per layer; a dep's buffer is workBufferPtrs[workBufferOffsetsPerOperationSet[dep.opSetIdx] + dep.opIdx].
        float* pInput = inputBuffer.data();
        for (std::size_t k = {}; k < activeOpIndices.size(); ++k) {
            const auto opIdx = activeOpIndices[k];
            const auto& op = ops[opIdx];

            // Prefetch next op's weight blob before mask+gather so the full iteration hides the miss.
            if (k + 1u < activeOpIndices.size()) {
                TF256::prefetchT0(cachedLayerDataPtrs[activeOpIndices[k + 1u]]);
            }

            if (op.neuralNet.layers.empty()) {
                continue;
            }

            float* pBuf = workBufferPtrs[opSetWorkBufferOffset + opIdx];
            float weight = 1.0f;
            if (hasMasks) {
                static constexpr std::uint32_t kNoMask = static_cast<std::uint32_t>(-1);
                weight = (op.neuralNet.maskIndex != kNoMask) ? masks[op.neuralNet.maskIndex] : 1.0f;
                if (weight == 0.0f) {
                    if (op.outputControlIndices.empty()) {
                        std::memcpy(pBuf, op.defaultValues.data(), op.defaultValues.size() * sizeof(float));
                    } else {
                        const auto* outIndices = op.outputControlIndices.data();
                        const auto outputCount = static_cast<std::size_t>(op.outputCountsPerLOD[lod]);
                        const auto limit = std::min(outputCount, op.outputControlIndices.size());
                        for (std::size_t i = {}; i < limit; ++i) {
                            pInput[outIndices[i]] = op.defaultValues[i];
                        }
                    }
                    continue;
                }
            }

            if (!op.inputControlIndices.empty()) {
                const auto count = op.inputControlIndices.size();
                const auto* inIndices = op.inputControlIndices.data();
                for (std::size_t i = {}; i < count; ++i) {
                    pBuf[i] = pInput[inIndices[i]];
                }
            } else {
                for (std::size_t i = {}, offset = {}; i < op.inputDeps.size(); ++i) {
                    const auto dep = op.inputDeps[i];
                    const float* depData =
                        workBufferPtrs[static_cast<std::size_t>(workBufferOffsetsPerOperationSet[dep.opSetIdx]) + dep.opIdx];
                    const auto count = op.outputCounts[i];
                    std::memcpy(pBuf + offset, depData, count * sizeof(float));
                    offset += count;
                }
            }

            const auto halfSize = static_cast<std::size_t>(workBufferHalfSizes[opSetWorkBufferOffset + opIdx]);
            auto buf1 = ArrayView<float>{pBuf, halfSize};
            auto buf2 = ArrayView<float>{pBuf + halfSize, halfSize};
            const T* flatPtr = op.neuralNet.flatData.template data<T>();
            for (const auto& layer : op.neuralNet.layers) {
                const T* weights = flatPtr + layer.weightOffset;
                const T* biases = flatPtr + layer.biasOffset;
                const float* activationParams = layer.activationFunctionParameters.data();
                switch (layer.activationFunction) {
                case dna::ActivationFunction::linear:
                    calculateBlock4<T, TF256, TF128, LinearActivationFunction>(weights,
                                                                               biases,
                                                                               activationParams,
                                                                               layer.weights.cols,
                                                                               layer.weights.rows[lod],
                                                                               buf1,
                                                                               buf2);
                    break;
                case dna::ActivationFunction::relu:
                    calculateBlock4<T, TF256, TF128, ReLUActivationFunction>(weights,
                                                                             biases,
                                                                             activationParams,
                                                                             layer.weights.cols,
                                                                             layer.weights.rows[lod],
                                                                             buf1,
                                                                             buf2);
                    break;
                case dna::ActivationFunction::leakyrelu:
                    calculateBlock4<T, TF256, TF128, LeakyReLUActivationFunction>(weights,
                                                                                  biases,
                                                                                  activationParams,
                                                                                  layer.weights.cols,
                                                                                  layer.weights.rows[lod],
                                                                                  buf1,
                                                                                  buf2);
                    break;
                case dna::ActivationFunction::tanh:
                    calculateBlock4<T, TF256, TF128, TanHActivationFunction>(weights,
                                                                             biases,
                                                                             activationParams,
                                                                             layer.weights.cols,
                                                                             layer.weights.rows[lod],
                                                                             buf1,
                                                                             buf2);
                    break;
                case dna::ActivationFunction::sigmoid:
                    calculateBlock4<T, TF256, TF128, SigmoidActivationFunction>(weights,
                                                                                biases,
                                                                                activationParams,
                                                                                layer.weights.cols,
                                                                                layer.weights.rows[lod],
                                                                                buf1,
                                                                                buf2);
                    break;
                }
                std::swap(buf1, buf2);
            }
            const float* result = buf1.data();

            // Attenuate outputs by the mask weight: on/off for ML joint rigs, fractional for blend-shape-channel rigs.
            const auto outputCount = static_cast<std::uint32_t>(op.outputCountsPerLOD[lod]);
            if (!op.outputControlIndices.empty()) {
                const auto outIndicesCount = static_cast<std::uint32_t>(op.outputControlIndices.size());
                const auto limit = std::min(outputCount, outIndicesCount);
                const auto* outIndices = op.outputControlIndices.data();
                for (std::uint32_t i = {}; i < limit; ++i) {
                    pInput[outIndices[i]] = result[i] * weight;
                }
            } else if (weight != 1.0f) {
                // Intermediate-dependency MLP: scale into pBuf, where downstream gathers read; also folds in an odd layer count.
                for (std::size_t i = {}; i < outputCount; ++i) {
                    pBuf[i] = result[i] * weight;
                }
            } else if (result != pBuf) {
                // An odd layer count left the result in the upper half; downstream gathers always read pBuf.
                std::memcpy(pBuf, result, outputCount * sizeof(float));
            }
            if (op.tailZeroCount != 0u) {
                // A WeightedSum consumer reads past this op's output count; zero the over-read region so it blends zeros.
                std::memset(pBuf + outputCount, 0, static_cast<std::size_t>(op.tailZeroCount) * sizeof(float));
            }
        }
    }

    OperationSetType getType() const override {
        return OperationSetType::MLP;
    }

    bool validate(const MLBehaviorBounds& bounds) const override {
        static constexpr std::uint32_t kNoMask = static_cast<std::uint32_t>(-1);
        const auto& bufferSizesForType = *bounds.bufferSizesForType;
        // execute() and OutputInstance index this set's ops and its bufferSizes row in lockstep.
        if (ops.size() != bufferSizesForType[bounds.opSetIndex].size()) {
            return false;
        }
        for (std::size_t opIdx = {}; opIdx < ops.size(); ++opIdx) {
            const auto& op = ops[opIdx];
            // Layer-less placeholder ops keep op-index alignment and are skipped by execute(); nothing to bound.
            if (op.neuralNet.layers.empty()) {
                continue;
            }

            // masks[maskIndex] is read when hasMasks; the mask buffer has meshRegionCount entries.
            if (hasMasks && (op.neuralNet.maskIndex != kNoMask) &&
                (static_cast<std::size_t>(op.neuralNet.maskIndex) >= bounds.meshRegionCount)) {
                return false;
            }

            // outputCountsPerLOD is indexed by lod throughout execute().
            if (op.outputCountsPerLOD.size() < bounds.lodCount) {
                return false;
            }

            const std::size_t bufferSize = static_cast<std::size_t>(bufferSizesForType[bounds.opSetIndex][opIdx]);
            const std::size_t halfSize = bufferSize / 2u;

            // The factory lays the weight blob out self-consistently, but a snapshot deserializes every field
            // independently, so offsets and matrix dims may point past flatData.
            {
                const std::size_t flatSize = op.neuralNet.flatData.template size<T>();
                for (const auto& layer : op.neuralNet.layers) {
                    if (layer.weights.rows.size() < bounds.lodCount) {
                        return false;
                    }
                    // execute()'s switch has no default, so an out-of-range enum would silently no-op the layer;
                    // leakyrelu also reads activationFunctionParameters[0].
                    switch (layer.activationFunction) {
                    case dna::ActivationFunction::linear:
                    case dna::ActivationFunction::relu:
                    case dna::ActivationFunction::tanh:
                    case dna::ActivationFunction::sigmoid:
                        break;
                    case dna::ActivationFunction::leakyrelu:
                        if (layer.activationFunctionParameters.empty()) {
                            return false;
                        }
                        break;
                    default:
                        return false;
                    }
                    // Reads span the PADDED matrix (rows*cols weights, rows biases) from their offsets and must fit
                    // flatData; size_t math avoids the uint32 overflow in Extent::size().
                    const std::size_t paddedRows = static_cast<std::size_t>(layer.weights.padded.rows);
                    const std::size_t paddedCols = static_cast<std::size_t>(layer.weights.padded.cols);
                    const std::size_t weightSpan = paddedRows * paddedCols;
                    if ((layer.weightOffset > flatSize) || (weightSpan > flatSize - layer.weightOffset)) {
                        return false;
                    }
                    if ((layer.biasOffset > flatSize) || (paddedRows > flatSize - layer.biasOffset)) {
                        return false;
                    }
                    // Weights/biases are read in whole TF128 lane groups via aligned loads and the builder places
                    // both offsets on that grid; a deserialized offset off it faults the first load.
                    constexpr std::size_t alignElems = TF128::size();
                    if (((layer.weightOffset % alignElems) != 0ul) || ((layer.biasOffset % alignElems) != 0ul)) {
                        return false;
                    }
                    // The per-LOD strides drive the pointer walk, so they must not exceed the padded dimensions.
                    if (static_cast<std::size_t>(layer.weights.cols.size) > paddedCols) {
                        return false;
                    }
                    // cols.size is also the INPUT extent calculateBlock4 reads from the halfSize-wide ping-pong half;
                    // paddedCols above bounds it only against the weight blob.
                    if (static_cast<std::size_t>(layer.weights.cols.size) > halfSize) {
                        return false;
                    }
                    // The inner loops step WHOLE 4/8-column groups to these boundaries; the weight walk stays in lockstep
                    // with the cols.size row stride only if each boundary is step-aligned and within cols.size.
                    if (((layer.weights.cols.sizePaddedToLastFullBlock % 4u) != 0u) ||
                        (layer.weights.cols.sizePaddedToLastFullBlock > layer.weights.cols.size)) {
                        return false;
                    }
                    if (((layer.weights.cols.sizePaddedToSecondLastFullBlock % 8u) != 0u) ||
                        (layer.weights.cols.sizePaddedToSecondLastFullBlock > layer.weights.cols.size)) {
                        return false;
                    }
                    for (const auto& rowView : layer.weights.rows) {
                        // calculateBlock4 walks whole 8-row then 4-row blocks, overshooting an unaligned boundary by up
                        // to a block, so the WALK, not the raw fields, must fit the spans above and the ping-pong half.
                        std::uint64_t rowWalk = 0u;
                        if (rowView.sizePaddedToLastFullBlock > 0u) {
                            rowWalk = ((static_cast<std::uint64_t>(rowView.sizePaddedToLastFullBlock) + 7u) / 8u) * 8u;
                        }
                        if (rowView.size > rowWalk) {
                            rowWalk += (((rowView.size - rowWalk) + 3u) / 4u) * 4u;
                        }
                        if ((rowWalk > paddedRows) || (rowWalk > halfSize)) {
                            return false;
                        }
                    }
                }
            }

            // Gather writes into buf1 (halfSize wide): control reads or the concatenated dependency outputs.
            if (!op.inputControlIndices.empty()) {
                if (op.inputControlIndices.size() > halfSize) {
                    return false;
                }
                for (const auto controlIndex : op.inputControlIndices) {
                    if (static_cast<std::size_t>(controlIndex) >= bounds.controlInputCount) {
                        return false;
                    }
                }
            } else {
                if (op.outputCounts.size() < op.inputDeps.size()) {
                    return false;
                }
                std::size_t gatherTotal = {};
                for (std::size_t di = {}; di < op.inputDeps.size(); ++di) {
                    const auto& dep = op.inputDeps[di];
                    if (!isValidDependency(dep, bufferSizesForType)) {
                        return false;
                    }
                    // execute() memcpys outputCounts[di] floats FROM the dependency's buffer, so it must fit the SOURCE width.
                    if (static_cast<std::size_t>(op.outputCounts[di]) >
                        static_cast<std::size_t>(bufferSizesForType[dep.opSetIdx][dep.opIdx])) {
                        return false;
                    }
                    gatherTotal += static_cast<std::size_t>(op.outputCounts[di]);
                }
                if (gatherTotal > halfSize) {
                    return false;
                }
            }

            // Per-LOD output width must fit the ping-pong halves the layers write into.
            for (const auto perLOD : op.outputCountsPerLOD) {
                if (static_cast<std::size_t>(perLOD) > halfSize) {
                    return false;
                }
            }

            // The widest per-LOD output bounds both the scatter span and the tail-zero memset below.
            std::size_t maxOutputCount = {};
            for (const auto perLOD : op.outputCountsPerLOD) {
                maxOutputCount = std::max(maxOutputCount, static_cast<std::size_t>(perLOD));
            }

            // Scatter writes pInput[outputControlIndices[i]] for i < min(outputCountsPerLOD[lod], size); the mask-zero
            // path writes defaultValues over the same span.
            if (!op.outputControlIndices.empty()) {
                for (const auto controlIndex : op.outputControlIndices) {
                    if (static_cast<std::size_t>(controlIndex) >= bounds.controlInputCount) {
                        return false;
                    }
                }
                const std::size_t scatterLimit = std::min(maxOutputCount, op.outputControlIndices.size());
                if (op.defaultValues.size() < scatterLimit) {
                    return false;
                }
            } else if ((op.defaultValues.size() > halfSize) || (op.defaultValues.size() != maxOutputCount)) {
                // Mask-zero path memcpys defaultValues into buf and skips the tail-zero memset, so the buffer stays clean
                // only when defaults cover exactly the output extent; a shorter array leaks stale data to the consumer.
                return false;
            }

            // Tail-zero memset writes pBuf[outputCount .. outputCount + tailZeroCount) into the full buffer.
            if (maxOutputCount + static_cast<std::size_t>(op.tailZeroCount) > bufferSize) {
                return false;
            }
        }
        return true;
    }

    void load(BoundedInputArchive& archive) override {
        std::uint32_t count = {};
        archive(count);
        // count is a raw scalar, not a container length, so BoundedInputArchive does not gate it; cap it against the
        // stream size so a hostile snapshot cannot drive reserve() into a bad_alloc.
        count = static_cast<std::uint32_t>(archive.boundSize(count));
        auto memRes = ops.get_allocator().getMemoryResource();
        ops.reserve(count);
        hasMasks = false;
        for (std::uint32_t i = {}; i < count; ++i) {
            MLPOperationData entry{memRes};
            archive(entry.neuralNet,
                    entry.inputDeps,
                    entry.inputControlIndices,
                    entry.outputControlIndices,
                    entry.outputCounts,
                    entry.defaultValues,
                    entry.tailZeroCount);
            // rows is deserialized separately from layers, so a hostile snapshot can present layers without any
            // per-LOD row views; require both before indexing lastRows.
            if (!entry.neuralNet.layers.empty() && !entry.neuralNet.layers.back().weights.rows.empty()) {
                const auto& lastRows = entry.neuralNet.layers.back().weights.rows;
                entry.outputCountsPerLOD.resize(lastRows.size());
                // Must mirror the factory: intermediate MLPs use the last layer's full original row count for all LODs
                // (not lastRows[0].size); defaultValues is sized from that count and validate() demands equality.
                if (entry.outputControlIndices.empty()) {
                    std::fill(entry.outputCountsPerLOD.begin(),
                              entry.outputCountsPerLOD.end(),
                              static_cast<std::uint16_t>(entry.neuralNet.layers.back().weights.original.rows));
                } else {
                    for (std::size_t li = {}; li < lastRows.size(); ++li) {
                        entry.outputCountsPerLOD[li] = static_cast<std::uint16_t>(lastRows[li].size);
                    }
                }
            }
            static constexpr std::uint32_t kNoMask = static_cast<std::uint32_t>(-1);
            if (entry.neuralNet.maskIndex != kNoMask) {
                hasMasks = true;
            }
            ops.push_back(std::move(entry));
            cachedLayerDataPtrs.push_back(ops.back().neuralNet.flatData.template data<T>());
        }
    }

    void save(terse::BinaryOutputArchive<BoundedIOStream>& archive) override {
        archive(static_cast<std::uint32_t>(ops.size()));
        for (auto& op : ops) {
            archive(op.neuralNet,
                    op.inputDeps,
                    op.inputControlIndices,
                    op.outputControlIndices,
                    op.outputCounts,
                    op.defaultValues,
                    op.tailZeroCount);
        }
    }
};

template<std::size_t Count, std::size_t Index = 0>
struct InvokeN {
    template<typename Fn>
    void operator()(Fn func) {
        func(Index);
        InvokeN<Count, Index + 1>()(func);
    }
};

template<std::size_t Count>
struct InvokeN<Count, Count> {
    template<typename Fn>
    void operator()(Fn func) {
        RL_UNUSED(func);
    }
};

template<std::size_t BlockSize, typename TF256, typename TF128>
struct WeightedSumOperationSet : OperationSet {
    Vector<WeightedSumOperationData> ops;

    explicit WeightedSumOperationSet(MemoryResource* memRes) :
        ops{memRes} {
    }

    explicit WeightedSumOperationSet(Vector<WeightedSumOperationData>&& ops_, MemoryResource* /*memRes*/) :
        ops{std::move(ops_)} {
    }

    void execute(ConstArrayView<std::uint16_t> activeOpIndices,
                 std::uint16_t lod,
                 ConstArrayView<float> masks,
                 ArrayView<float> inputBuffer,
                 std::size_t opSetWorkBufferOffset,
                 ConstArrayView<float*> workBufferPtrs,
                 ConstArrayView<std::uint16_t> workBufferHalfSizes,
                 ConstArrayView<std::uint32_t> workBufferOffsetsPerOperationSet) const override {

        // Outputs accumulate in BlockSize-wide chunks of blockCount SIMD registers; a scalar loop handles the remainder.
        RL_UNUSED(lod);
        RL_UNUSED(masks);
        RL_UNUSED(inputBuffer);
        RL_UNUSED(workBufferHalfSizes);

        constexpr std::size_t blockCount = BlockSize / TF256::size();

        for (const auto opIdx : activeOpIndices) {
            const auto& op = ops[opIdx];
            if (op.inputDeps.empty()) {
                continue;
            }

            float* pBuf = workBufferPtrs[opSetWorkBufferOffset + opIdx];
            const auto outputCount = static_cast<std::size_t>(op.outputCount);
            const auto alignedCount = outputCount - (outputCount % BlockSize);

            for (std::size_t elem = {}; elem < alignedCount; elem += BlockSize) {
                TF256 sums[blockCount] = {};
                for (std::size_t di = {}; di < op.inputDeps.size(); ++di) {
                    const auto depBufOffset = workBufferOffsetsPerOperationSet[op.inputDeps[di].opSetIdx];
                    const float* pDepBuf = workBufferPtrs[static_cast<std::size_t>(depBufOffset) + op.inputDeps[di].opIdx];
                    TF256 blocks[blockCount];
                    InvokeN<blockCount>()(
                        [&](std::size_t bi) { blocks[bi] = TF256::fromUnalignedSource(pDepBuf + elem + TF256::size() * bi); });
                    const TF256 weight{op.weights[di]};
                    InvokeN<blockCount>()([&](std::size_t bi) { sums[bi] += blocks[bi] * weight; });
                }
                InvokeN<blockCount>()([&](std::size_t bi) { sums[bi].unalignedStore(pBuf + elem + TF256::size() * bi); });
            }

            if (alignedCount != outputCount) {
                std::fill(pBuf + alignedCount, pBuf + outputCount, 0.0f);
                for (std::size_t di = {}; di < op.inputDeps.size(); ++di) {
                    const float* pDep =
                        workBufferPtrs[static_cast<std::size_t>(workBufferOffsetsPerOperationSet[op.inputDeps[di].opSetIdx]) +
                                       op.inputDeps[di].opIdx];
                    const float w = op.weights[di];
                    for (std::size_t elem = alignedCount; elem < outputCount; ++elem) {
                        pBuf[elem] += w * pDep[elem];
                    }
                }
            }
        }
    }

    bool validate(const MLBehaviorBounds& bounds) const override {
        const auto& bufferSizesForType = *bounds.bufferSizesForType;
        if (ops.size() != bufferSizesForType[bounds.opSetIndex].size()) {
            return false;
        }
        for (std::size_t opIdx = {}; opIdx < ops.size(); ++opIdx) {
            const auto& op = ops[opIdx];
            if (op.inputDeps.empty()) {
                continue;  // execute() skips dependency-less ops.
            }
            // Reads op.weights[di] for every dependency.
            if (op.weights.size() < op.inputDeps.size()) {
                return false;
            }
            // Writes pBuf[elem] for elem < outputCount into this op's own buffer.
            const std::size_t outputCount = static_cast<std::size_t>(op.outputCount);
            if (outputCount > static_cast<std::size_t>(bufferSizesForType[bounds.opSetIndex][opIdx])) {
                return false;
            }
            // Reads pDepBuf[elem] for elem < outputCount from every dependency's buffer.
            for (const auto& dep : op.inputDeps) {
                if (!isValidDependency(dep, bufferSizesForType) ||
                    (outputCount > static_cast<std::size_t>(bufferSizesForType[dep.opSetIdx][dep.opIdx]))) {
                    return false;
                }
            }
        }
        return true;
    }

    OperationSetType getType() const override {
        // BlockSize / 8 avoids MSVC C6326 on a constant switch; WeightedSum8..64 serialize as 1..8.
        static_assert((BlockSize >= 8u) && (BlockSize <= 64u) && (BlockSize % 8u == 0u),
                      "WeightedSumOperationSet block size must be a multiple of 8 in [8, 64].");
        static_assert(static_cast<std::uint16_t>(OperationSetType::WeightedSum8) == 1u,
                      "WeightedSum8 must serialize as 1 for the BlockSize / 8 mapping.");
        static_assert(static_cast<std::uint16_t>(OperationSetType::WeightedSum64) == 8u,
                      "WeightedSum64 must serialize as 8 for the BlockSize / 8 mapping.");
        return static_cast<OperationSetType>(BlockSize / 8u);
    }

    void load(BoundedInputArchive& archive) override {
        std::uint32_t count = {};
        archive(count);
        // Raw scalar, not gated by BoundedInputArchive; cap against the stream size before reserve().
        count = static_cast<std::uint32_t>(archive.boundSize(count));
        auto memRes = ops.get_allocator().getMemoryResource();
        ops.reserve(count);
        for (std::uint32_t i = {}; i < count; ++i) {
            WeightedSumOperationData entry{memRes};
            archive(entry.inputDeps, entry.weights, entry.outputCount);
            ops.push_back(std::move(entry));
        }
    }

    void save(terse::BinaryOutputArchive<BoundedIOStream>& archive) override {
        archive(static_cast<std::uint32_t>(ops.size()));
        for (auto& op : ops) {
            archive(op.inputDeps, op.weights, op.outputCount);
        }
    }
};

template<typename T, typename TF512, typename TF256, typename TF128>
struct OperationSetFactory {
    using BasePointer = UniqueInstance<OperationSet>::PointerType;

    BasePointer operator()(OperationSetData&& data, MemoryResource* memRes) {
        switch (data.type) {
        case (OperationSetType::MLP): {

            using OpSet = MLPOperationSet<T, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.mlpOps), data.hasMasks, memRes);
        }
        case OperationSetType::WeightedSum8: {
            using OpSet = WeightedSumOperationSet<8, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum16: {
            using OpSet = WeightedSumOperationSet<16, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum24: {
            using OpSet = WeightedSumOperationSet<24, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum32: {
            using OpSet = WeightedSumOperationSet<32, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum40: {
            using OpSet = WeightedSumOperationSet<40, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum48: {
            using OpSet = WeightedSumOperationSet<48, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum56: {
            using OpSet = WeightedSumOperationSet<56, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        case OperationSetType::WeightedSum64:
        default: {
            using OpSet = WeightedSumOperationSet<64, TF256, TF128>;
            return UniqueInstance<OpSet, OperationSet>::with(memRes).create(std::move(data.wsOps), memRes);
        }
        }
    }
};

// Precise/Fast arms live in dedicated TUs. Neither returns null: a Fast TU without relaxed-FP support delegates
// to the Precise arm itself, so createOperationSet picks exactly one arm and moves `data` exactly once.
UniqueInstance<OperationSet>::PointerType createPreciseOperationSet(const Configuration& config,
                                                                    OperationSetData&& data,
                                                                    MemoryResource* memRes);

#ifdef RL_BUILD_WITH_FAST
UniqueInstance<OperationSet>::PointerType createFastOperationSet(const Configuration& config,
                                                                 OperationSetData&& data,
                                                                 MemoryResource* memRes);
#endif  // RL_BUILD_WITH_FAST

inline UniqueInstance<OperationSet>::PointerType createOperationSet(const Configuration& config,
                                                                    OperationSetData&& data,
                                                                    MemoryResource* memRes) {
#ifdef RL_BUILD_WITH_FAST
    if (config.floatingPointModel == FloatingPointModel::Fast) {
        return createFastOperationSet(config, std::move(data), memRes);
    }
#endif  // RL_BUILD_WITH_FAST
    return createPreciseOperationSet(config, std::move(data), memRes);
}

}  // namespace cpu

}  // namespace ml

}  // namespace rl4
