// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "rltests/Defs.h"
#include "rltests/dna/FakeReader.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/Operation.h"
#include "riglogic/types/LODSpec.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstdint>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rltests {

namespace ml {

// Chained topology: Gather -> MLP(odd layers, intermediate dep) -> WeightedSum -> MLP(even layers, scatter)
// Purpose: exercise cross-op-set dep gather path and the odd-layer ping-pong normalization fix.
namespace chained {

using namespace rl4;

extern const std::uint16_t rawControlCount;
extern const std::uint16_t mlControlCount;
extern const std::uint16_t lodCount;

extern const Vector<Matrix<dna::MachineLearnedBehaviorOperationType>> mlOperationTypes;
extern const Matrix<Matrix<std::uint32_t>> mlOperationParameters;
extern const Matrix<Matrix<std::uint16_t>> mlDependencyOperationSetIndices;
extern const Matrix<Matrix<std::uint16_t>> mlDependencyOperationIndices;
extern const Matrix<Matrix<std::uint16_t>> mlOperationIndicesPerLOD;

extern const Matrix<dna::ActivationFunction> mlbNetActivationFunctions;
extern const Vector<Matrix<float>> mlbNetActivationFunctionParameters;
extern const Vector<Matrix<float>> mlbNetWeights;
extern const Vector<Matrix<float>> mlbNetBiases;

namespace input {
extern const Vector<float> values;
}  // namespace input

namespace output {
extern const Matrix<float> valuesPerLOD;
}  // namespace output

class CanonicalReader : public dna::FakeReader {
protected:
    // Canonical DNA layout has 5 op sets: Gather(0) -> MLP(1, intermediate dep) -> WeightedSum(2) ->
    // MLP(3, final) -> Scatter(4). Gather and Scatter are not standalone runtime ops: the factory folds
    // their params into the MLPs (Gather -> input indices, Scatter -> output control indices), so the
    // final scatter is performed by the set-3 MLP - hence scatterMlpOpSet == 3 and no constant for set 4.
    static constexpr std::uint16_t gatherOpSet = 0u;
    static constexpr std::uint16_t intermediateMlpOpSet = 1u;  // MLP feeding a downstream dep (no scatter)
    static constexpr std::uint16_t weightedSumOpSet = 2u;
    static constexpr std::uint16_t scatterMlpOpSet = 3u;  // final MLP; the folded Scatter op (set 4) supplies its output controls
    static constexpr std::uint16_t intermediateNetIndex = 0u;  // NN0, driven by the intermediate MLP
    static constexpr std::uint16_t scatterNetIndex = 1u;       // NN1, driven by the scatter MLP
    static constexpr std::uint16_t canonicalMeshIndex = 0u;

public:
    ~CanonicalReader();

    std::uint16_t getRawControlCount() const override {
        return rawControlCount;
    }

    std::uint16_t getLODCount() const override {
        return lodCount;
    }

    std::uint16_t getMLTypeCount() const override {
        return static_cast<std::uint16_t>(mlOperationTypes.size());
    }

    std::uint16_t getMLOperationSetCount(std::uint16_t mlTypeIndex) const override {
        return static_cast<std::uint16_t>(mlOperationTypes[mlTypeIndex].size());
    }

    std::uint16_t getMLOperationCount(std::uint16_t mlTypeIndex, std::uint16_t mlOperationSetIndex) const override {
        return static_cast<std::uint16_t>(mlOperationTypes[mlTypeIndex][mlOperationSetIndex].size());
    }

    dna::MachineLearnedBehaviorOperationType getMLOperationType(std::uint16_t mlTypeIndex,
                                                                std::uint16_t mlOperationSetIndex,
                                                                std::uint16_t mlOperationIndex) const override {
        return mlOperationTypes[mlTypeIndex][mlOperationSetIndex][mlOperationIndex];
    }

    ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t mlTypeIndex,
                                                           std::uint16_t mlOperationSetIndex,
                                                           std::uint16_t mlOperationIndex) const override {
        return mlOperationParameters[mlTypeIndex][mlOperationSetIndex][mlOperationIndex];
    }

    ConstArrayView<std::uint16_t> getMLOperationDependencyOperationSetIndices(std::uint16_t mlTypeIndex,
                                                                              std::uint16_t mlOperationSetIndex,
                                                                              std::uint16_t mlOperationIndex) const override {
        return mlDependencyOperationSetIndices[mlTypeIndex][mlOperationSetIndex][mlOperationIndex];
    }

    ConstArrayView<std::uint16_t> getMLOperationDependencyOperationIndices(std::uint16_t mlTypeIndex,
                                                                           std::uint16_t mlOperationSetIndex,
                                                                           std::uint16_t mlOperationIndex) const override {
        return mlDependencyOperationIndices[mlTypeIndex][mlOperationSetIndex][mlOperationIndex];
    }

    ConstArrayView<std::uint16_t> getMLOperationIndicesForLOD(std::uint16_t mlTypeIndex,
                                                              std::uint16_t mlOperationSetIndex,
                                                              std::uint16_t lod) const override {
        return mlOperationIndicesPerLOD[mlTypeIndex][mlOperationSetIndex][lod];
    }

    std::uint16_t getNeuralNetworkLayerCount(std::uint16_t neuralNetIndex) const override {
        return static_cast<std::uint16_t>(mlbNetWeights[neuralNetIndex].size());
    }

    dna::ActivationFunction getNeuralNetworkLayerActivationFunction(std::uint16_t neuralNetIndex,
                                                                    std::uint16_t layerIndex) const override {
        return mlbNetActivationFunctions[neuralNetIndex][layerIndex];
    }

    ConstArrayView<float> getNeuralNetworkLayerActivationFunctionParameters(std::uint16_t neuralNetIndex,
                                                                            std::uint16_t layerIndex) const override {
        return mlbNetActivationFunctionParameters[neuralNetIndex][layerIndex];
    }

    ConstArrayView<float> getNeuralNetworkLayerBiases(std::uint16_t neuralNetIndex, std::uint16_t layerIndex) const override {
        return mlbNetBiases[neuralNetIndex][layerIndex];
    }

    ConstArrayView<float> getNeuralNetworkLayerWeights(std::uint16_t neuralNetIndex, std::uint16_t layerIndex) const override {
        return mlbNetWeights[neuralNetIndex][layerIndex];
    }
};

// Assigns each neural net to its own mesh region so mask-based output attenuation can be tested:
// region 0 -> NN1 (final MLP with scatter outputs), region 1 -> NN0 (intermediate dep MLP).
class MaskedCanonicalReader : public CanonicalReader {
public:
    ~MaskedCanonicalReader();

    std::uint16_t getMeshCount() const override {
        return 1u;
    }

    std::uint16_t getMeshRegionCount(std::uint16_t meshIndex) const override {
        return (meshIndex == canonicalMeshIndex) ? meshRegionCount : static_cast<std::uint16_t>(0u);
    }

    ConstArrayView<std::uint16_t> getNeuralNetworkIndicesForMeshRegion(std::uint16_t meshIndex,
                                                                       std::uint16_t regionIndex) const override {
        static const std::uint16_t netIndicesPerRegion[] = {scatterNetIndex, intermediateNetIndex};
        if ((meshIndex != canonicalMeshIndex) || (regionIndex >= meshRegionCount)) {
            return {};
        }
        return {netIndicesPerRegion + regionIndex, 1ul};
    }

protected:
    static constexpr std::uint16_t meshRegionCount = 2u;  // region 0 -> NN1, region 1 -> NN0
};

// Injects a layer-less MLP op (neural net 2, zero layers) at index 0 of the first MLP op set (set 1),
// shifting the real NN0 op to index 1. The factory must keep a placeholder for the layer-less op so that
// LOD lists and cross-set dependency op indices (which reference original DNA indices) stay aligned.
class MixedOpSetReader : public CanonicalReader {
public:
    ~MixedOpSetReader();

    std::uint16_t getMLOperationCount(std::uint16_t mlTypeIndex, std::uint16_t mlOperationSetIndex) const override {
        if (mlOperationSetIndex == intermediateMlpOpSet) {
            return mlpOpCount;
        }
        return CanonicalReader::getMLOperationCount(mlTypeIndex, mlOperationSetIndex);
    }

    dna::MachineLearnedBehaviorOperationType getMLOperationType(std::uint16_t mlTypeIndex,
                                                                std::uint16_t mlOperationSetIndex,
                                                                std::uint16_t mlOperationIndex) const override {
        if (mlOperationSetIndex == intermediateMlpOpSet) {
            return dna::MachineLearnedBehaviorOperationType::MLP;
        }
        return CanonicalReader::getMLOperationType(mlTypeIndex, mlOperationSetIndex, mlOperationIndex);
    }

    ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t mlTypeIndex,
                                                           std::uint16_t mlOperationSetIndex,
                                                           std::uint16_t mlOperationIndex) const override {
        if (mlOperationSetIndex == intermediateMlpOpSet) {
            static const std::uint32_t netIndexPerOp[] = {layerlessNetIndex, intermediateNetIndex};
            return {netIndexPerOp + mlOperationIndex, 1ul};
        }
        return CanonicalReader::getMLOperationParameters(mlTypeIndex, mlOperationSetIndex, mlOperationIndex);
    }

    ConstArrayView<std::uint16_t> getMLOperationDependencyOperationSetIndices(std::uint16_t mlTypeIndex,
                                                                              std::uint16_t mlOperationSetIndex,
                                                                              std::uint16_t mlOperationIndex) const override {
        if (mlOperationSetIndex == intermediateMlpOpSet) {
            if (mlOperationIndex == placeholderOpIndex) {
                return {};
            }
            static const std::uint16_t depSets[] = {gatherOpSet};
            return {depSets, 1ul};
        }
        return CanonicalReader::getMLOperationDependencyOperationSetIndices(mlTypeIndex, mlOperationSetIndex, mlOperationIndex);
    }

    ConstArrayView<std::uint16_t> getMLOperationDependencyOperationIndices(std::uint16_t mlTypeIndex,
                                                                           std::uint16_t mlOperationSetIndex,
                                                                           std::uint16_t mlOperationIndex) const override {
        if (mlOperationSetIndex == intermediateMlpOpSet) {
            if (mlOperationIndex == placeholderOpIndex) {
                return {};
            }
            static const std::uint16_t depOps[] = {0u};  // the sole gather op
            return {depOps, 1ul};
        }
        if (mlOperationSetIndex == weightedSumOpSet) {
            // The weighted sum depends on the real MLP, which now sits at the shifted op index.
            static const std::uint16_t depOps[] = {realMlpOpIndex};
            return {depOps, 1ul};
        }
        return CanonicalReader::getMLOperationDependencyOperationIndices(mlTypeIndex, mlOperationSetIndex, mlOperationIndex);
    }

    ConstArrayView<std::uint16_t> getMLOperationIndicesForLOD(std::uint16_t mlTypeIndex,
                                                              std::uint16_t mlOperationSetIndex,
                                                              std::uint16_t lod) const override {
        if (mlOperationSetIndex == intermediateMlpOpSet) {
            // Both ops active per LOD, so the runtime skip of the placeholder is exercised too.
            static const std::uint16_t opIndices[] = {placeholderOpIndex, realMlpOpIndex};
            return {opIndices, 2ul};
        }
        return CanonicalReader::getMLOperationIndicesForLOD(mlTypeIndex, mlOperationSetIndex, lod);
    }

    std::uint16_t getNeuralNetworkLayerCount(std::uint16_t neuralNetIndex) const override {
        if (neuralNetIndex == layerlessNetIndex) {
            return 0u;
        }
        return CanonicalReader::getNeuralNetworkLayerCount(neuralNetIndex);
    }

protected:
    static constexpr std::uint16_t layerlessNetIndex = 2u;  // NN2, injected placeholder (zero layers)
    static constexpr std::uint16_t mlpOpCount = 2u;         // placeholder op + real MLP op
    static constexpr std::uint16_t placeholderOpIndex = 0u;
    static constexpr std::uint16_t realMlpOpIndex = 1u;  // real NN0 op, shifted down by the placeholder
};

// The WeightedSum's outputCount (12) exceeds its dependency NN0's true output width (8) - malformed data
// that makes the WS read past NN0's outputs. NN1's first layer becomes 12-in x 8-out, where inputs [8..11]
// feed outputs [0..3] with weight 1, so any non-zero garbage in the over-read region visibly corrupts the
// final outputs. With the factory's tail-zeroing defense, the over-read region blends zeros and the final
// outputs match the canonical expectations exactly.
class WSOverreadReader : public CanonicalReader {
public:
    ~WSOverreadReader();

    ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t mlTypeIndex,
                                                           std::uint16_t mlOperationSetIndex,
                                                           std::uint16_t mlOperationIndex) const override {
        if (mlOperationSetIndex == weightedSumOpSet) {
            static const std::uint32_t wsParams[] = {overreadOutputCount, weightOneBits};
            return {wsParams, 2ul};
        }
        return CanonicalReader::getMLOperationParameters(mlTypeIndex, mlOperationSetIndex, mlOperationIndex);
    }

    ConstArrayView<float> getNeuralNetworkLayerWeights(std::uint16_t neuralNetIndex, std::uint16_t layerIndex) const override {
        if ((neuralNetIndex == scatterNetIndex) && (layerIndex == 0u)) {
            // 8 outputs x 12 inputs, row-major: output i = input i + input (8 + i) for i < 4, identity above.
            static const float weights[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            return {weights, 96ul};
        }
        return CanonicalReader::getNeuralNetworkLayerWeights(neuralNetIndex, layerIndex);
    }

protected:
    static constexpr std::uint32_t overreadOutputCount = 12u;    // > NN0's true width of 8 -> WS reads past its outputs
    static constexpr std::uint32_t weightOneBits = 0x3F800000u;  // 1.0f
};

// Combines WSOverreadReader's malformed WeightedSum (outputCount 12 > NN0's true width 8) with a mesh
// region mask on NN0, the over-read intermediate dep. Proves the mask and tail-zeroing features compose:
// the WeightedSum's over-read tail reads zeros whether NN0 is masked off from the very first evaluation
// (zero-initialized instance buffers), evaluated unmasked (trailing memset), or masked off afterwards
// (the zero-weight path never writes past the output count, preserving the previously zeroed tail).
class MaskedWSOverreadReader : public WSOverreadReader {
public:
    ~MaskedWSOverreadReader();

    std::uint16_t getMeshCount() const override {
        return 1u;
    }

    std::uint16_t getMeshRegionCount(std::uint16_t meshIndex) const override {
        return (meshIndex == canonicalMeshIndex) ? meshRegionCount : static_cast<std::uint16_t>(0u);
    }

    ConstArrayView<std::uint16_t> getNeuralNetworkIndicesForMeshRegion(std::uint16_t meshIndex,
                                                                       std::uint16_t regionIndex) const override {
        static const std::uint16_t netIndices[] = {intermediateNetIndex};  // region 0 -> NN0 (the over-read intermediate dep)
        if ((meshIndex != canonicalMeshIndex) || (regionIndex != 0u)) {
            return {};
        }
        return {netIndices, 1ul};
    }

protected:
    static constexpr std::uint16_t meshRegionCount = 1u;  // single masked region over NN0
};

// The final MLP (NN1) limits its last layer's output count per LOD: 8 rows at LOD 0, 4 rows at LOD 1.
// Combined with the mesh-region masks of MaskedCanonicalReader, this pins down that both the masked
// (weight == 0 defaults, weight != 1 attenuation) and unmasked scatter paths write exactly the current
// LOD's output count and never touch control slots beyond it.
class LODLimitedMaskedReader : public MaskedCanonicalReader {
public:
    ~LODLimitedMaskedReader();

    ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t mlTypeIndex,
                                                           std::uint16_t mlOperationSetIndex,
                                                           std::uint16_t mlOperationIndex) const override {
        if (mlOperationSetIndex == scatterMlpOpSet) {
            // [netIndex, layer0: rowsAtLOD0, rowsAtLOD1, layer1: rowsAtLOD0, rowsAtLOD1]
            static const std::uint32_t mlpParams[] = {scatterNetIndex, 8u, 8u, 8u, 4u};
            return {mlpParams, 5ul};
        }
        return MaskedCanonicalReader::getMLOperationParameters(mlTypeIndex, mlOperationSetIndex, mlOperationIndex);
    }
};

}  // namespace chained

}  // namespace ml

}  // namespace rltests
