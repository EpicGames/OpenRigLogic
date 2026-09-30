// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/bpcm/JointGroup.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/SIMD.h"
#include "riglogic/types/bpcm/FloatArray.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstddef>
#include <cstdint>
#include <type_traits>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

namespace bpcm {

// The vector type the strategy runs with: the widest NATIVE one of {TF512, TF256, TF128} (RTI passes trimd::fallback
// wrappers for widths a backend lacks). Builder and validator must mirror this choice; block strides derive from it.
template<typename TF512, typename TF256, typename TF128>
using TBPCMVec = typename std::conditional<
    std::is_same<TF512, trimd::fallback::T512<TF256>>::value,
    typename std::conditional<std::is_same<TF256, trimd::fallback::T256<TF128>>::value, TF128, TF256>::type,
    TF512>::type;

struct JointStorage {
    FloatArray values;
    AlignedVector<std::uint16_t> inputIndices;
    AlignedVector<std::uint16_t> outputIndices;
    Vector<LODRegion> lodRegions;
    // Start index of each rotation triple, for conversion to quaternions
    Vector<std::uint16_t> outputRotationIndices;
    Vector<std::uint16_t> outputRotationLODs;
    Vector<JointGroup> jointGroups;
    // Unpadded output rows per LOD over all groups; a LOD with none is skipped entirely
    Vector<std::uint32_t> outputRowsPerLOD;

    explicit JointStorage(MemoryResource* memRes) :
        values{memRes},
        inputIndices{memRes},
        outputIndices{memRes},
        lodRegions{memRes},
        outputRotationIndices{memRes},
        outputRotationLODs{memRes},
        jointGroups{memRes},
        outputRowsPerLOD{memRes} {
    }

    template<class Archive>
    void serialize(Archive& archive) {
        archive(values,
                inputIndices,
                outputIndices,
                lodRegions,
                outputRotationIndices,
                outputRotationLODs,
                jointGroups,
                outputRowsPerLOD);
    }
};

struct JointGroupView {
    void* values;
    std::uint32_t colCount;
    std::uint32_t rowCount;
    std::uint32_t blockHeight;
    std::uint16_t* inputIndices;
    std::uint16_t* outputIndices;
    std::uint16_t* outputRotationIndices;
    std::uint16_t* outputRotationLODs;
    LODRegion* lods;
};

// Checks every group's deserialized offsets/extents against the containers before StorageSnapshot forms raw pointers
// from them: BoundedInputArchive bounds allocation sizes, not these scalars. Always holds for in-process storage.
template<typename T, typename TF512, typename TF256, typename TF128>
struct StorageValidator {

    bool operator()(const JointStorage& storage, const RigMetadata& metadata, RotationType rotationType) {
        const std::size_t groupCount = storage.jointGroups.size();
        if (groupCount == 0ul) {
            return true;
        }
        // Each group owns lodCount regions, so lodCount is derivable from container sizes rather than a trusted scalar.
        if ((storage.lodRegions.size() % groupCount) != 0ul) {
            return false;
        }
        const std::size_t lodCount = storage.lodRegions.size() / groupCount;
        // Evaluation indexes lods[lod] up to metadata.lodCount, so each group needs at least that many regions.
        if (lodCount < static_cast<std::size_t>(metadata.lodCount)) {
            return false;
        }
        if ((storage.outputRotationLODs.size() % groupCount) != 0ul) {
            return false;
        }
        const std::size_t rotationLODCount = storage.outputRotationLODs.size() / groupCount;
        // Quaternion-output adapters index outputRotationLODs[lod] up to metadata.lodCount; the Noop adapter for Euler
        // rigs never reads the rotation containers, so emptiness is legitimate only there (per-group checks no-op on empty).
        if (rotationType != RotationType::EulerAngles) {
            if (rotationLODCount < static_cast<std::size_t>(metadata.lodCount)) {
                return false;
            }
        } else if (!storage.outputRotationLODs.empty() && (rotationLODCount < static_cast<std::size_t>(metadata.lodCount))) {
            return false;
        }

        const std::size_t valueCount = storage.values.template size<T>();
        const std::size_t controlInputCount = metadata.getControlInputCount();
        for (const auto& group : storage.jointGroups) {
            const std::size_t colCount = group.colCount;
            const std::size_t rowCount = group.rowCount;
            // Subtraction/division rather than addition/multiplication so a 32-bit size_t cannot wrap two uint32_t terms
            // and let a crafted snapshot pass. The strategy reads at most colCount * rowCount values (rowCount is padded).
            if (static_cast<std::size_t>(group.valuesOffset) > valueCount) {
                return false;
            }
            const std::size_t valuesAvailable = valueCount - group.valuesOffset;
            if ((colCount != 0ul) && ((rowCount > (valuesAvailable / colCount)))) {
                return false;
            }
            // colCount == 0 skips the values-extent check, leaving rowCount unconstrained; the builder never emits such a group.
            if ((colCount == 0ul) && (rowCount != 0ul)) {
                return false;
            }
            if ((static_cast<std::size_t>(group.inputIndicesOffset) > storage.inputIndices.size()) ||
                (colCount > (storage.inputIndices.size() - group.inputIndicesOffset))) {
                return false;
            }
            if ((static_cast<std::size_t>(group.outputIndicesOffset) > storage.outputIndices.size()) ||
                (rowCount > (storage.outputIndices.size() - group.outputIndicesOffset))) {
                return false;
            }
            // Input indices gather from the control input buffer (raw+psd+ml+rbf); the kernel reads them for col up to
            // the per-LOD sizes, all <= colCount.
            const std::uint16_t* inIndices = storage.inputIndices.data() + group.inputIndicesOffset;
            for (std::size_t col = 0ul; col < colCount; ++col) {
                if (static_cast<std::size_t>(inIndices[col]) >= controlInputCount) {
                    return false;
                }
            }
            // Output index values are scattered into the joint output buffer (jointAttributeCount).
            const std::uint16_t* outIndices = storage.outputIndices.data() + group.outputIndicesOffset;
            for (std::size_t row = 0ul; row < rowCount; ++row) {
                if (outIndices[row] >= metadata.jointAttributeCount) {
                    return false;
                }
            }
            if ((static_cast<std::size_t>(group.lodsOffset) > storage.lodRegions.size()) ||
                (lodCount > (storage.lodRegions.size() - group.lodsOffset))) {
                return false;
            }
            if ((static_cast<std::size_t>(group.outputRotationLODsOffset) > storage.outputRotationLODs.size()) ||
                (rotationLODCount > (storage.outputRotationLODs.size() - group.outputRotationLODsOffset))) {
                return false;
            }
            // Rotation indices are deduplicated per group (no fixed count); the adapter walks outputRotationIndices[row] for
            // row < outputRotationLODs[lod], so bound the offset, then each LOD's row count against the remaining indices.
            if (static_cast<std::size_t>(group.outputRotationIndicesOffset) > storage.outputRotationIndices.size()) {
                return false;
            }
            const std::size_t rotationIndicesAvailable = storage.outputRotationIndices.size() - group.outputRotationIndicesOffset;
            const std::uint16_t* rotationLODs = storage.outputRotationLODs.data() + group.outputRotationLODsOffset;
            std::size_t maxRotationRows = 0ul;
            for (std::size_t lod = 0ul; lod < rotationLODCount; ++lod) {
                if (rotationLODs[lod] > rotationIndicesAvailable) {
                    return false;
                }
                if (static_cast<std::size_t>(rotationLODs[lod]) > maxRotationRows) {
                    maxRotationRows = rotationLODs[lod];
                }
            }
            // The adapter writes outputs[rotationStartIndex + 0..3] for every rotation row any LOD reaches, so each must
            // leave room for four components in the joint output buffer. rotationStartIndex is uint16, so + 3 cannot overflow.
            const std::uint16_t* rotationIndices = storage.outputRotationIndices.data() + group.outputRotationIndicesOffset;
            for (std::size_t row = 0ul; row < maxRotationRows; ++row) {
                if ((static_cast<std::size_t>(rotationIndices[row]) + 3ul) >= metadata.jointAttributeCount) {
                    return false;
                }
            }
            // Any unrecognized blockHeight - including a hostile one - falls through to the TF128 kernel in the strategy,
            // so the walk bound must mirror that selection rather than assume the widest native vector.
            std::uint64_t halfStep = TF128::size();
            if (group.blockHeight == 2u * static_cast<std::uint32_t>(TF512::size())) {
                halfStep = TF512::size();
            } else if (group.blockHeight == 2u * static_cast<std::uint32_t>(TF256::size())) {
                halfStep = TF256::size();
            }
            const std::uint64_t fullStep = 2ull * halfStep;
            // Per-LOD sub-sizes drive kernel loop bounds; they must stay within the group's col/row extents.
            const LODRegion* lods = storage.lodRegions.data() + group.lodsOffset;
            for (std::size_t lod = 0ul; lod < lodCount; ++lod) {
                const LODRegion& region = lods[lod];
                if ((region.inputLODs.size > colCount) || (region.inputLODs.sizeAlignedTo4 > colCount) ||
                    (region.inputLODs.sizeAlignedTo8 > colCount)) {
                    return false;
                }
                // The kernel's unrolled column loops step whole 4/8-column groups below these boundaries, so an unaligned
                // boundary is overshot by up to a step past the extents. The builder rounds them DOWN from size.
                if (((region.inputLODs.sizeAlignedTo4 % 4u) != 0u) || (region.inputLODs.sizeAlignedTo4 > region.inputLODs.size)) {
                    return false;
                }
                if (((region.inputLODs.sizeAlignedTo8 % 8u) != 0u) || (region.inputLODs.sizeAlignedTo8 > region.inputLODs.size)) {
                    return false;
                }
                // Bound the walk's end, not the raw boundary fields, by the padded row extent that pins both the
                // outputIndices reads and the value buffer.
                if (blockWalkEnd(region, fullStep, halfStep) > static_cast<std::uint64_t>(rowCount)) {
                    return false;
                }
            }
        }
        return true;
    }
};

template<typename T, typename TF512, typename TF256, typename TF128>
struct StorageSnapshot {

    Vector<JointGroupView> operator()(JointStorage& storage, MemoryResource* memRes) {
        Vector<JointGroupView> snapshot{storage.jointGroups.size(), {}, memRes};
        for (std::size_t i = 0ul; i < storage.jointGroups.size(); ++i) {
            const auto& jointGroup = storage.jointGroups[i];
            snapshot[i].values = storage.values.data<T>() + jointGroup.valuesOffset;
            snapshot[i].colCount = jointGroup.colCount;
            snapshot[i].rowCount = jointGroup.rowCount;
            snapshot[i].blockHeight = jointGroup.blockHeight;
            snapshot[i].inputIndices = storage.inputIndices.data() + jointGroup.inputIndicesOffset;
            snapshot[i].outputIndices = storage.outputIndices.data() + jointGroup.outputIndicesOffset;
            snapshot[i].outputRotationIndices = storage.outputRotationIndices.data() + jointGroup.outputRotationIndicesOffset;
            snapshot[i].outputRotationLODs = storage.outputRotationLODs.data() + jointGroup.outputRotationLODsOffset;
            snapshot[i].lods = storage.lodRegions.data() + jointGroup.lodsOffset;
        }
        return snapshot;
    }
};

}  // namespace bpcm

}  // namespace rl4
