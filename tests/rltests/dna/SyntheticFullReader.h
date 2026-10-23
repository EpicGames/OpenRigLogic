// Copyright Epic Games, Inc. All Rights Reserved.

// Twin of tools/SyntheticFullReader.h (separate so tools/ and tests/ build independently). Keep the rig data
// identical: the tools copy becomes examples/Synthetic.dna, this copy pins its coverage contract; drift decouples them.

#pragma once

#include "rltests/dna/FakeReader.h"

#include <cstring>

namespace dna {

// A minimal synthetic rig whose data activates every RigLogic subsystem (all nine evaluator kinds build Concrete)
// under both the EulerAngles and Quaternions configurations.
//
// Control input buffer layout (getControlInputCount() == 22):
//   raw [0,8) | psd [8,10) | ml [10,18) | rbf [18,22)
class SyntheticFullReader : public FakeReader {
public:
    ~SyntheticFullReader();

    static StringView sv(const char* text) {
        return {text, std::strlen(text)};
    }

    // FakeReader reports format 2.3, which BinaryStreamWriter::setFrom copies into the output header and then silently
    // omits every layer newer than it (RBF ext needs 2.5, ML ext 2.6); report the newest format so all layers serialize.
    std::uint16_t getFileFormatGeneration() const override {
        return 2u;
    }

    std::uint16_t getFileFormatVersion() const override {
        return 8u;
    }

    StringView getName() const override {
        return sv("Synthetic");
    }

    CoordinateSystem getCoordinateSystem() const override {
        return {tdm::axis_dir::right, tdm::axis_dir::up, tdm::axis_dir::front};
    }

    // FakeReader's default rot_sign{} is zero, which is neither positive nor negative and fails
    // RigMetadata::validate() before any factory runs.
    RotationSign getRotationSign() const override {
        return {tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
    }

    std::uint16_t getLODCount() const override {
        return 4u;
    }

    std::uint16_t getGUIControlCount() const override {
        return 3u;
    }

    StringView getGUIControlName(std::uint16_t index) const override {
        static const char* names[] = {"gui0", "gui1", "gui2"};
        return (index < 3u) ? sv(names[index]) : StringView{};
    }

    std::uint16_t getRawControlCount() const override {
        return 8u;
    }

    // raw[4,8) is the driver quad of the RBF solvers and the twist/swing setup, read by nothing else. Its names pair
    // it with joint4 (the DNA3 conversion resolves a driver joint from the "<joint>.q*" control names), and the .qw
    // suffix gives raw7 an initial value of 1.0 (createInitialControlValues), so the neutral driver is the identity.
    StringView getRawControlName(std::uint16_t index) const override {
        static const char* names[] = {"ctrl0", "ctrl1", "ctrl2", "ctrl3", "joint4.qx", "joint4.qy", "joint4.qz", "joint4.qw"};
        return (index < 8u) ? sv(names[index]) : StringView{};
    }

    // joint0-2 are joint group / ML joints outputs, joint3 the twist output, joint4 the driver no evaluator writes,
    // joint5-8 the rows of the wide joint group, joint9 the swing output. The twist and the swing output to different
    // joints: on a shared one the evaluator's later write would replace the earlier.
    std::uint16_t getJointCount() const override {
        return 10u;
    }

    StringView getJointName(std::uint16_t index) const override {
        static const char* names[] =
            {"joint0", "joint1", "joint2", "joint3", "joint4", "joint5", "joint6", "joint7", "joint8", "joint9"};
        return (index < 10u) ? sv(names[index]) : StringView{};
    }

    std::uint16_t getJointIndexListCount() const override {
        return 4u;
    }

    ConstArrayView<std::uint16_t> getJointIndicesForLOD(std::uint16_t lod) const override {
        static const std::uint16_t full[] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u};
        static const std::uint16_t coarse[] = {0u, 1u, 2u, 4u, 5u};
        return (lod < 3u) ? ConstArrayView<std::uint16_t>{full, 10ul} : ConstArrayView<std::uint16_t>{coarse, 5ul};
    }

    std::uint16_t getJointParentIndex(std::uint16_t index) const override {
        static const std::uint16_t parents[] = {0u, 0u, 1u, 2u, 0u, 0u, 0u, 0u, 0u, 0u};
        return (index < 10u) ? parents[index] : std::uint16_t{};
    }

    std::uint16_t getBlendShapeChannelCount() const override {
        return 2u;
    }

    StringView getBlendShapeChannelName(std::uint16_t index) const override {
        static const char* names[] = {"bs0", "bs1"};
        return (index < 2u) ? sv(names[index]) : StringView{};
    }

    std::uint16_t getBlendShapeChannelIndexListCount() const override {
        return 4u;
    }

    ConstArrayView<std::uint16_t> getBlendShapeChannelIndicesForLOD(std::uint16_t lod) const override {
        static const std::uint16_t full[] = {0u, 1u};
        static const std::uint16_t coarse[] = {0u};
        return (lod < 2u) ? ConstArrayView<std::uint16_t>{full, 2ul} : ConstArrayView<std::uint16_t>{coarse, 1ul};
    }

    std::uint16_t getAnimatedMapCount() const override {
        return 2u;
    }

    StringView getAnimatedMapName(std::uint16_t index) const override {
        static const char* names[] = {"am0", "am1"};
        return (index < 2u) ? sv(names[index]) : StringView{};
    }

    std::uint16_t getAnimatedMapIndexListCount() const override {
        return 4u;
    }

    ConstArrayView<std::uint16_t> getAnimatedMapIndicesForLOD(std::uint16_t lod) const override {
        static const std::uint16_t full[] = {0u, 1u};
        static const std::uint16_t coarse[] = {0u};
        return (lod < 2u) ? ConstArrayView<std::uint16_t>{full, 2ul} : ConstArrayView<std::uint16_t>{coarse, 1ul};
    }

    std::uint16_t getMeshCount() const override {
        return 1u;
    }

    StringView getMeshName(std::uint16_t index) const override {
        return (index == 0u) ? sv("mesh0") : StringView{};
    }

    std::uint16_t getMeshIndexListCount() const override {
        return 4u;
    }

    ConstArrayView<std::uint16_t> getMeshIndicesForLOD(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {0u};
        return {v, 1ul};
    }

    // Neutral joint values: SoA, jointCount floats each; rotations in degrees.
    ConstArrayView<float> getNeutralJointTranslationXs() const override {
        static const float v[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        return {v, 10ul};
    }

    ConstArrayView<float> getNeutralJointTranslationYs() const override {
        static const float v[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        return {v, 10ul};
    }

    ConstArrayView<float> getNeutralJointTranslationZs() const override {
        static const float v[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        return {v, 10ul};
    }

    ConstArrayView<float> getNeutralJointRotationXs() const override {
        static const float v[] = {0.0f, 10.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        return {v, 10ul};
    }

    ConstArrayView<float> getNeutralJointRotationYs() const override {
        static const float v[] = {0.0f, 0.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        return {v, 10ul};
    }

    ConstArrayView<float> getNeutralJointRotationZs() const override {
        static const float v[] = {0.0f, 0.0f, 0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        return {v, 10ul};
    }

    // Identity mapping gui i -> raw i, plus constant rows pinning raw3 and raw7 to 1 across the GUI range, since the
    // mapping zero-fills every raw control it does not write and a zero quaternion normalizes to NaN: raw7 is the w
    // of the driver quad raw[4,8); raw3 reaches ML control 13, the qx of the ML-joints quad, through net0's
    // out[i] = in[i % 6].
    ConstArrayView<std::uint16_t> getGUIToRawInputIndices() const override {
        static const std::uint16_t v[] = {0u, 1u, 2u, 0u, 0u};
        return {v, 5ul};
    }

    ConstArrayView<std::uint16_t> getGUIToRawOutputIndices() const override {
        static const std::uint16_t v[] = {0u, 1u, 2u, 3u, 7u};
        return {v, 5ul};
    }

    ConstArrayView<float> getGUIToRawFromValues() const override {
        static const float v[] = {0.0f, 0.0f, 0.0f, -1.0f, -1.0f};
        return {v, 5ul};
    }

    ConstArrayView<float> getGUIToRawToValues() const override {
        static const float v[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        return {v, 5ul};
    }

    ConstArrayView<float> getGUIToRawSlopeValues() const override {
        static const float v[] = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f};
        return {v, 5ul};
    }

    ConstArrayView<float> getGUIToRawCutValues() const override {
        static const float v[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
        return {v, 5ul};
    }

    // psd8 = raw0*raw1, psd9 = raw2*raw3; row indices are absolute.
    std::uint16_t getPSDCount() const override {
        return 2u;
    }

    ConstArrayView<std::uint16_t> getPSDRowIndices() const override {
        static const std::uint16_t v[] = {8u, 8u, 9u, 9u};
        return {v, 4ul};
    }

    ConstArrayView<std::uint16_t> getPSDColumnIndices() const override {
        static const std::uint16_t v[] = {0u, 1u, 2u, 3u};
        return {v, 4ul};
    }

    ConstArrayView<float> getPSDValues() const override {
        static const float v[] = {1.0f, 1.0f, 1.0f, 1.0f};
        return {v, 4ul};
    }

    // Attr space is 9 per joint (tx..sz); variable attrs are the union of driven attrs per LOD.
    ConstArrayView<std::uint16_t> getJointVariableAttributeIndices(std::uint16_t lod) const override {
        static std::uint16_t lod01[19 + 36];
        static std::uint16_t lod2[19 + 32];
        static std::uint16_t lod3[7 + 8];
        static bool initialized = false;
        if (!initialized) {
            const std::uint16_t head[] =
                {3u, 4u, 5u, 9u, 12u, 13u, 14u, 18u, 19u, 20u, 21u, 22u, 23u, 30u, 31u, 32u, 84u, 85u, 86u};
            for (std::size_t i = {}; i < 19ul; ++i) {
                lod01[i] = head[i];
                lod2[i] = head[i];
                if (i < 7ul) {
                    lod3[i] = head[i];
                }
            }
            // Joint group 2's rows: joints 5-8, attribute indices 45-80, cut to 32 then 8 rows at the coarse LODs.
            for (std::size_t i = {}; i < 36ul; ++i) {
                const auto attr = static_cast<std::uint16_t>(45ul + i);
                lod01[19ul + i] = attr;
                if (i < 32ul) {
                    lod2[19ul + i] = attr;
                }
                if (i < 8ul) {
                    lod3[7ul + i] = attr;
                }
            }
            initialized = true;
        }
        if (lod < 2u) {
            return {lod01, 55ul};
        }
        if (lod == 2u) {
            return {lod2, 51ul};
        }
        return {lod3, 15ul};
    }

    std::uint16_t getJointGroupCount() const override {
        return 3u;
    }

    // Group 0: euler joint0 rotation rows (BPCM only). Group 1: quaternion joint1, one translation row (BPCM) plus a
    // contiguous rx,ry,rz triple (quaternion evaluator). Group 2: every attribute of euler joints 5-8, 36 rows x 5
    // columns, the one group wide enough for kernels that tile 32 rows at a time. Row counts shrink at coarse LODs to
    // hit unaligned block walks.
    ConstArrayView<std::uint16_t> getJointGroupLODs(std::uint16_t jointGroupIndex) const override {
        static const std::uint16_t g0[] = {3u, 3u, 2u, 2u};
        static const std::uint16_t g1[] = {4u, 4u, 4u, 1u};
        static const std::uint16_t g2[] = {36u, 36u, 32u, 8u};
        switch (jointGroupIndex) {
        case 0u:
            return {g0, 4ul};
        case 1u:
            return {g1, 4ul};
        default:
            return {g2, 4ul};
        }
    }

    ConstArrayView<std::uint16_t> getJointGroupInputIndices(std::uint16_t jointGroupIndex) const override {
        static const std::uint16_t g0[] = {0u, 1u};
        static const std::uint16_t g1[] = {1u, 8u};
        static const std::uint16_t g2[] = {0u, 1u, 2u, 3u, 9u};
        switch (jointGroupIndex) {
        case 0u:
            return {g0, 2ul};
        case 1u:
            return {g1, 2ul};
        default:
            return {g2, 5ul};
        }
    }

    ConstArrayView<std::uint16_t> getJointGroupOutputIndices(std::uint16_t jointGroupIndex) const override {
        static const std::uint16_t g0[] = {3u, 4u, 5u};
        static const std::uint16_t g1[] = {9u, 12u, 13u, 14u};
        static std::uint16_t g2[36];
        static bool initialized = false;
        if (!initialized) {
            for (std::size_t i = {}; i < 36ul; ++i) {
                g2[i] = static_cast<std::uint16_t>(45ul + i);
            }
            initialized = true;
        }
        switch (jointGroupIndex) {
        case 0u:
            return {g0, 3ul};
        case 1u:
            return {g1, 4ul};
        default:
            return {g2, 36ul};
        }
    }

    // rows * cols, row-major; every row and column holds a non-zero value so nothing is pruned. Group 2's values cycle
    // through 13 distinct multiples of 1/8, so no two adjacent rows or columns repeat.
    ConstArrayView<float> getJointGroupValues(std::uint16_t jointGroupIndex) const override {
        static const float g0[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
        static const float g1[] = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f};
        static float g2[36 * 5];
        static bool initialized = false;
        if (!initialized) {
            for (std::size_t i = {}; i < 180ul; ++i) {
                g2[i] = 0.125f * static_cast<float>((i % 13ul) + 1ul);
            }
            initialized = true;
        }
        switch (jointGroupIndex) {
        case 0u:
            return {g0, 6ul};
        case 1u:
            return {g1, 8ul};
        default:
            return {g2, 180ul};
        }
    }

    ConstArrayView<std::uint16_t> getJointGroupJointIndices(std::uint16_t jointGroupIndex) const override {
        static const std::uint16_t g0[] = {0u};
        static const std::uint16_t g1[] = {1u};
        static const std::uint16_t g2[] = {5u, 6u, 7u, 8u};
        switch (jointGroupIndex) {
        case 0u:
            return {g0, 1ul};
        case 1u:
            return {g1, 1ul};
        default:
            return {g2, 4ul};
        }
    }

    // Inputs are the PSD outputs, which also makes PSDNet's per-LOD registered-control lists non-empty.
    ConstArrayView<std::uint16_t> getBlendShapeChannelLODs() const override {
        static const std::uint16_t v[] = {2u, 2u, 1u, 1u};
        return {v, 4ul};
    }

    ConstArrayView<std::uint16_t> getBlendShapeChannelInputIndices() const override {
        static const std::uint16_t v[] = {8u, 9u};
        return {v, 2ul};
    }

    ConstArrayView<std::uint16_t> getBlendShapeChannelOutputIndices() const override {
        static const std::uint16_t v[] = {0u, 1u};
        return {v, 2ul};
    }

    // Rows 0-1 form an interval group on map0 (same input/output pair, adjacent ranges); row 2 drives map1.
    ConstArrayView<std::uint16_t> getAnimatedMapLODs() const override {
        static const std::uint16_t v[] = {3u, 3u, 1u, 1u};
        return {v, 4ul};
    }

    ConstArrayView<std::uint16_t> getAnimatedMapInputIndices() const override {
        static const std::uint16_t v[] = {0u, 0u, 1u};
        return {v, 3ul};
    }

    ConstArrayView<std::uint16_t> getAnimatedMapOutputIndices() const override {
        static const std::uint16_t v[] = {0u, 0u, 1u};
        return {v, 3ul};
    }

    ConstArrayView<float> getAnimatedMapFromValues() const override {
        static const float v[] = {0.0f, 0.5f, 0.0f};
        return {v, 3ul};
    }

    ConstArrayView<float> getAnimatedMapToValues() const override {
        static const float v[] = {0.5f, 1.0f, 1.0f};
        return {v, 3ul};
    }

    ConstArrayView<float> getAnimatedMapSlopeValues() const override {
        static const float v[] = {1.0f, 0.5f, 1.0f};
        return {v, 3ul};
    }

    ConstArrayView<float> getAnimatedMapCutValues() const override {
        static const float v[] = {0.0f, 0.25f, 0.0f};
        return {v, 3ul};
    }

    std::uint16_t getMLControlCount() const override {
        return 8u;
    }

    // ml6 ("jnt2.qw", buffer index 16) is the qw of the ML-joints rotation quad, so it both gets an
    // initial value of 1.0 and exercises getDefaultValues' quaternion-qw defaulting.
    StringView getMLControlName(std::uint16_t index) const override {
        static const char* names[] = {"ml.tx", "ml.ty", "ml.tz", "jnt2.qx", "jnt2.qy", "jnt2.qz", "jnt2.qw", "ml.extra"};
        return (index < 8u) ? sv(names[index]) : StringView{};
    }

    std::uint16_t getNeuralNetworkCount() const override {
        return 2u;
    }

    std::uint16_t getNeuralNetworkIndexListCount() const override {
        return 4u;
    }

    ConstArrayView<std::uint16_t> getNeuralNetworkIndicesForLOD(std::uint16_t lod) const override {
        static const std::uint16_t full[] = {0u, 1u};
        static const std::uint16_t coarse[] = {0u};
        return (lod < 3u) ? ConstArrayView<std::uint16_t>{full, 2ul} : ConstArrayView<std::uint16_t>{coarse, 1ul};
    }

    std::uint16_t getMeshRegionCount(std::uint16_t meshIndex) const override {
        return (meshIndex == 0u) ? std::uint16_t{2u} : std::uint16_t{};
    }

    StringView getMeshRegionName(std::uint16_t meshIndex, std::uint16_t regionIndex) const override {
        static const char* names[] = {"region0", "region1"};
        return ((meshIndex == 0u) && (regionIndex < 2u)) ? sv(names[regionIndex]) : StringView{};
    }

    // region0 masks net1 (the scattering MLP), region1 masks net0 - both op sets get live masks.
    ConstArrayView<std::uint16_t> getNeuralNetworkIndicesForMeshRegion(std::uint16_t meshIndex,
                                                                       std::uint16_t regionIndex) const override {
        static const std::uint16_t r0[] = {1u};
        static const std::uint16_t r1[] = {0u};
        if (meshIndex != 0u) {
            return {};
        }
        if (regionIndex == 0u) {
            return {r0, 1ul};
        }
        if (regionIndex == 1u) {
            return {r1, 1ul};
        }
        return {};
    }

    ConstArrayView<std::uint16_t> getNeuralNetworkInputIndices(std::uint16_t neuralNetIndex) const override {
        static const std::uint16_t net0[] = {0u, 1u, 2u, 3u, 8u, 9u};
        static const std::uint16_t net1[] = {0u, 1u, 2u, 3u, 8u, 9u, 8u, 9u};
        return (neuralNetIndex == 0u) ? ConstArrayView<std::uint16_t>{net0, 6ul} : ConstArrayView<std::uint16_t>{net1, 8ul};
    }

    ConstArrayView<std::uint16_t> getNeuralNetworkOutputIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {10u, 11u, 12u, 13u, 14u, 15u, 16u, 17u};
        return {v, 8ul};
    }

    std::uint16_t getNeuralNetworkLayerCount(std::uint16_t neuralNetIndex) const override {
        return (neuralNetIndex == 0u) ? std::uint16_t{1u} : std::uint16_t{2u};
    }

    ActivationFunction getNeuralNetworkLayerActivationFunction(std::uint16_t neuralNetIndex,
                                                               std::uint16_t layerIndex) const override {
        if (neuralNetIndex == 0u) {
            return ActivationFunction::relu;
        }
        return (layerIndex == 0u) ? ActivationFunction::leakyrelu : ActivationFunction::linear;
    }

    ConstArrayView<float> getNeuralNetworkLayerActivationFunctionParameters(std::uint16_t neuralNetIndex,
                                                                            std::uint16_t layerIndex) const override {
        static const float alpha[] = {0.01f};
        // leakyrelu requires its alpha parameter; the validator rejects it when absent.
        if ((neuralNetIndex == 1u) && (layerIndex == 0u)) {
            return {alpha, 1ul};
        }
        return {};
    }

    ConstArrayView<float> getNeuralNetworkLayerBiases(std::uint16_t /*unused*/, std::uint16_t /*unused*/) const override {
        static const float v[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        return {v, 8ul};
    }

    // outputCount = biases.size() = 8; inputCount = weights.size() / 8. Net0 layer0 is 8x6
    // (out[i] = in[i % 6]); net1's two layers are 8x8 identity.
    ConstArrayView<float> getNeuralNetworkLayerWeights(std::uint16_t neuralNetIndex, std::uint16_t /*unused*/) const override {
        static float net0[48];
        static float identity8[64];
        static bool initialized = false;
        if (!initialized) {
            for (std::size_t i = {}; i < 8ul; ++i) {
                net0[i * 6ul + (i % 6ul)] = 1.0f;
                identity8[i * 8ul + i] = 1.0f;
            }
            initialized = true;
        }
        return (neuralNetIndex == 0u) ? ConstArrayView<float>{net0, 48ul} : ConstArrayView<float>{identity8, 64ul};
    }

    // 5 op sets, one op each: Gather -> MLP(net0) -> WeightedSum -> MLP(net1) -> Scatter. Gather/Scatter fold into their
    // dependents at build; a 5-set shape also cannot match the DNA writer's 3-set heuristic, so the ext layer survives.
    std::uint16_t getMLTypeCount() const override {
        return 1u;
    }

    std::uint16_t getMLOperationSetCount(std::uint16_t mlTypeIndex) const override {
        return (mlTypeIndex == 0u) ? std::uint16_t{5u} : std::uint16_t{};
    }

    std::uint16_t getMLOperationCount(std::uint16_t mlTypeIndex, std::uint16_t mlOperationSetIndex) const override {
        return ((mlTypeIndex == 0u) && (mlOperationSetIndex < 5u)) ? std::uint16_t{1u} : std::uint16_t{};
    }

    MachineLearnedBehaviorOperationType getMLOperationType(std::uint16_t mlTypeIndex,
                                                           std::uint16_t mlOperationSetIndex,
                                                           std::uint16_t mlOperationIndex) const override {
        static const MachineLearnedBehaviorOperationType types[] = {
            MachineLearnedBehaviorOperationType::Gather,
            MachineLearnedBehaviorOperationType::MLP,
            MachineLearnedBehaviorOperationType::WeightedSum,
            MachineLearnedBehaviorOperationType::MLP,
            MachineLearnedBehaviorOperationType::Scatter,
        };
        if ((mlTypeIndex != 0u) || (mlOperationSetIndex >= 5u) || (mlOperationIndex != 0u)) {
            return {};
        }
        return types[mlOperationSetIndex];
    }

    ConstArrayView<std::uint32_t> getMLOperationParameters(std::uint16_t mlTypeIndex,
                                                           std::uint16_t mlOperationSetIndex,
                                                           std::uint16_t mlOperationIndex) const override {
        // Gather params: absolute control-buffer indices to read (raw[0,4) + psd).
        static const std::uint32_t gather[] = {0u, 1u, 2u, 3u, 8u, 9u};
        // MLP params: {netIndex}; net1 additionally carries per-LOD row counts (layerCount * lodCount
        // tail): layer0 full-width at every LOD, layer1 limited to 4 rows at LODs 2-3.
        static const std::uint32_t mlp0[] = {0u};
        static const std::uint32_t mlp1[] = {1u, 8u, 8u, 8u, 8u, 8u, 8u, 4u, 4u};
        // WeightedSum params: {outputCount, per-dependency float weight as IEEE-754 bits (1.0f)}.
        static const std::uint32_t wsum[] = {8u, 0x3F800000u};
        // Scatter params: absolute control-buffer indices to write (the ML section).
        static const std::uint32_t scatter[] = {10u, 11u, 12u, 13u, 14u, 15u, 16u, 17u};
        if ((mlTypeIndex != 0u) || (mlOperationIndex != 0u)) {
            return {};
        }
        switch (mlOperationSetIndex) {
        case 0u:
            return {gather, 6ul};
        case 1u:
            return {mlp0, 1ul};
        case 2u:
            return {wsum, 2ul};
        case 3u:
            return {mlp1, 9ul};
        case 4u:
            return {scatter, 8ul};
        default:
            return {};
        }
    }

    ConstArrayView<std::uint16_t> getMLOperationDependencyOperationSetIndices(std::uint16_t mlTypeIndex,
                                                                              std::uint16_t mlOperationSetIndex,
                                                                              std::uint16_t mlOperationIndex) const override {
        static const std::uint16_t deps[] = {0u, 1u, 2u, 3u};
        if ((mlTypeIndex != 0u) || (mlOperationIndex != 0u) || (mlOperationSetIndex == 0u) || (mlOperationSetIndex >= 5u)) {
            return {};
        }
        return {deps + (mlOperationSetIndex - 1u), 1ul};
    }

    ConstArrayView<std::uint16_t> getMLOperationDependencyOperationIndices(std::uint16_t mlTypeIndex,
                                                                           std::uint16_t mlOperationSetIndex,
                                                                           std::uint16_t mlOperationIndex) const override {
        static const std::uint16_t zero[] = {0u};
        if ((mlTypeIndex != 0u) || (mlOperationIndex != 0u) || (mlOperationSetIndex == 0u) || (mlOperationSetIndex >= 5u)) {
            return {};
        }
        return {zero, 1ul};
    }

    std::uint16_t getMLOperationIndexListCount(std::uint16_t mlTypeIndex, std::uint16_t mlOperationSetIndex) const override {
        return ((mlTypeIndex == 0u) && (mlOperationSetIndex < 5u)) ? std::uint16_t{4u} : std::uint16_t{};
    }

    ConstArrayView<std::uint16_t> getMLOperationIndicesForLOD(std::uint16_t mlTypeIndex,
                                                              std::uint16_t mlOperationSetIndex,
                                                              std::uint16_t lod) const override {
        static const std::uint16_t zero[] = {0u};
        if ((mlTypeIndex != 0u) || (mlOperationSetIndex >= 5u) || (lod >= 4u)) {
            return {};
        }
        return {zero, 1ul};
    }

    // ML joints drive joint2. Inputs are ML-section control indices; outputs index the ML model's 10-attrs-per-joint
    // space (tx,ty,tz at 20-22, qx..qw at 23-26). The rotation quad's control indices (13-16) must be consecutive (span-4).
    ConstArrayView<std::uint16_t> getMLJointsInputIndices() const override {
        static const std::uint16_t v[] = {10u, 11u, 12u, 13u, 14u, 15u, 16u};
        return {v, 7ul};
    }

    ConstArrayView<std::uint16_t> getMLJointsOutputIndices() const override {
        static const std::uint16_t v[] = {20u, 21u, 22u, 23u, 24u, 25u, 26u};
        return {v, 7ul};
    }

    ConstArrayView<std::uint16_t> getMLJointsParameterKeys() const override {
        static const std::uint16_t v[] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u};
        return {v, 10ul};
    }

    // Vector translation, Quaternion rotation, Vector scale; coord sys right/up/front, positive signs, xyz sequence -
    // all matching the DNA's own descriptor, so the change-of-basis transformers stay Noop.
    ConstArrayView<std::uint16_t> getMLJointsParameterValues() const override {
        static const std::uint16_t v[] = {0u, 1u, 0u, 1u, 2u, 4u, 1u, 1u, 1u, 0u};
        return {v, 10ul};
    }

    TranslationRepresentation getJointTranslationRepresentation(std::uint16_t /*unused*/) const override {
        return TranslationRepresentation::Vector;
    }

    RotationRepresentation getJointRotationRepresentation(std::uint16_t jointIndex) const override {
        return (jointIndex == 1u) ? RotationRepresentation::Quaternion : RotationRepresentation::EulerAngles;
    }

    ScaleRepresentation getJointScaleRepresentation(std::uint16_t /*unused*/) const override {
        return ScaleRepresentation::Vector;
    }

    // Identical input quads (the driver), so twist and swing merge into one setup carrying both halves; the twist
    // outputs onto joint3, the swing onto joint9.
    std::uint16_t getTwistCount() const override {
        return 1u;
    }

    TwistAxis getTwistSetupTwistAxis(std::uint16_t /*unused*/) const override {
        return TwistAxis::X;
    }

    ConstArrayView<std::uint16_t> getTwistInputControlIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {4u, 5u, 6u, 7u};
        return {v, 4ul};
    }

    ConstArrayView<std::uint16_t> getTwistOutputJointIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {3u};
        return {v, 1ul};
    }

    ConstArrayView<float> getTwistBlendWeights(std::uint16_t /*unused*/) const override {
        static const float v[] = {0.5f};
        return {v, 1ul};
    }

    std::uint16_t getSwingCount() const override {
        return 1u;
    }

    TwistAxis getSwingSetupTwistAxis(std::uint16_t /*unused*/) const override {
        return TwistAxis::X;
    }

    ConstArrayView<std::uint16_t> getSwingInputControlIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {4u, 5u, 6u, 7u};
        return {v, 4ul};
    }

    ConstArrayView<std::uint16_t> getSwingOutputJointIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {9u};
        return {v, 1ul};
    }

    ConstArrayView<float> getSwingBlendWeights(std::uint16_t /*unused*/) const override {
        static const float v[] = {0.75f};
        return {v, 1ul};
    }

    // Two solvers over the same two targets (the identity and a turn about X) with poses of their own:
    // Additive/Euclidean/fixed radius and Interpolative/Quaternion/automatic radius.
    std::uint16_t getRBFPoseCount() const override {
        return 4u;
    }

    StringView getRBFPoseName(std::uint16_t poseIndex) const override {
        static const char* names[] = {"poseA", "poseB", "poseC", "poseD"};
        return (poseIndex < 4u) ? sv(names[poseIndex]) : StringView{};
    }

    float getRBFPoseScale(std::uint16_t /*unused*/) const override {
        return 1.0f;
    }

    std::uint16_t getRBFPoseControlCount() const override {
        return 4u;
    }

    StringView getRBFPoseControlName(std::uint16_t poseControlIndex) const override {
        static const char* names[] = {"rbf0", "rbf1", "rbf2", "rbf3"};
        return (poseControlIndex < 4u) ? sv(names[poseControlIndex]) : StringView{};
    }

    // Pose outputs live in the RBF section of the control buffer, [18, 22), pose i at 18 + i: each solver's weights
    // land in controls of their own, so both stay observable at the LODs where both solvers run.
    ConstArrayView<std::uint16_t> getRBFPoseOutputControlIndices(std::uint16_t poseIndex) const override {
        static const std::uint16_t v[] = {18u, 19u, 20u, 21u};
        return (poseIndex < 4u) ? ConstArrayView<std::uint16_t>{v + poseIndex, 1ul} : ConstArrayView<std::uint16_t>{};
    }

    ConstArrayView<float> getRBFPoseOutputControlWeights(std::uint16_t /*unused*/) const override {
        static const float v[] = {1.0f};
        return {v, 1ul};
    }

    std::uint16_t getRBFSolverCount() const override {
        return 2u;
    }

    std::uint16_t getRBFSolverIndexListCount() const override {
        return 4u;
    }

    ConstArrayView<std::uint16_t> getRBFSolverIndicesForLOD(std::uint16_t lod) const override {
        static const std::uint16_t full[] = {0u, 1u};
        static const std::uint16_t coarse[] = {0u};
        return (lod < 3u) ? ConstArrayView<std::uint16_t>{full, 2ul} : ConstArrayView<std::uint16_t>{coarse, 1ul};
    }

    StringView getRBFSolverName(std::uint16_t index) const override {
        static const char* names[] = {"solverA", "solverB"};
        return (index < 2u) ? sv(names[index]) : StringView{};
    }

    // The driver quad = one quaternion input, satisfying the multiple-of-4 rule for solver1's
    // Quaternion distance method.
    ConstArrayView<std::uint16_t> getRBFSolverRawControlIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t v[] = {4u, 5u, 6u, 7u};
        return {v, 4ul};
    }

    ConstArrayView<std::uint16_t> getRBFSolverPoseIndices(std::uint16_t solverIndex) const override {
        static const std::uint16_t s0[] = {0u, 1u};
        static const std::uint16_t s1[] = {2u, 3u};
        return (solverIndex == 0u) ? ConstArrayView<std::uint16_t>{s0, 2ul} : ConstArrayView<std::uint16_t>{s1, 2ul};
    }

    // Exactly poseCount * inputCount floats: two distinct unit-quaternion targets per solver.
    ConstArrayView<float> getRBFSolverRawControlValues(std::uint16_t solverIndex) const override {
        static const float s0[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        static const float s1[] = {0.0f, 0.0f, 0.0f, 1.0f, 0.70710678f, 0.0f, 0.0f, 0.70710678f};
        return (solverIndex == 0u) ? ConstArrayView<float>{s0, 8ul} : ConstArrayView<float>{s1, 8ul};
    }

    RBFSolverType getRBFSolverType(std::uint16_t solverIndex) const override {
        return (solverIndex == 0u) ? RBFSolverType::Additive : RBFSolverType::Interpolative;
    }

    float getRBFSolverRadius(std::uint16_t /*unused*/) const override {
        return 45.0f;  // degrees; converted via the DNA's rotation unit
    }

    // FakeReader's default {} would be AutomaticRadius::On; solver0 wants the fixed radius above.
    AutomaticRadius getRBFSolverAutomaticRadius(std::uint16_t solverIndex) const override {
        return (solverIndex == 0u) ? AutomaticRadius::Off : AutomaticRadius::On;
    }

    float getRBFSolverWeightThreshold(std::uint16_t /*unused*/) const override {
        return 0.0f;
    }

    RBFDistanceMethod getRBFSolverDistanceMethod(std::uint16_t solverIndex) const override {
        return (solverIndex == 0u) ? RBFDistanceMethod::Euclidean : RBFDistanceMethod::Quaternion;
    }

    RBFNormalizeMethod getRBFSolverNormalizeMethod(std::uint16_t solverIndex) const override {
        return (solverIndex == 0u) ? RBFNormalizeMethod::OnlyNormalizeAboveOne : RBFNormalizeMethod::AlwaysNormalize;
    }

    // Gaussian for both: a linear kernel saturates solver1's interpolated weights to 0 or 1 under most unit-quaternion
    // drivers, leaving nothing between the targets to compare.
    RBFFunctionType getRBFSolverFunctionType(std::uint16_t /*unused*/) const override {
        return RBFFunctionType::Gaussian;
    }

    TwistAxis getRBFSolverTwistAxis(std::uint16_t solverIndex) const override {
        return (solverIndex == 0u) ? TwistAxis::X : TwistAxis::Y;
    }
};

}  // namespace dna
