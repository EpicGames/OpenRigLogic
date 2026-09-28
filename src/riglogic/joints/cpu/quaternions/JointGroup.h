// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/utils/LODRegion.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/types/bpcm/FloatArray.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstddef>
#include <cstdint>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

struct JointGroup {
    // All non-zero values
    FloatArray values;
    // Sub-matrix col -> input vector
    Vector<std::uint16_t> inputIndices;
    // Sub-matrix row -> output vector
    Vector<std::uint16_t> outputIndices;
    // Output index boundaries for each LOD
    Vector<LODRegion> lods;
    // Matrix size
    std::uint32_t colCount;
    std::uint32_t rowCount;

    explicit JointGroup(MemoryResource* memRes) :
        values{memRes},
        inputIndices{memRes},
        outputIndices{memRes},
        lods{memRes},
        colCount{},
        rowCount{} {
    }

    template<class Archive>
    void serialize(Archive& archive) {
        archive(values, inputIndices, outputIndices, lods, colCount, rowCount);
    }
};

// Bounds each group's deserialized LOD regions and indices against its own buffers before the kernel walks them;
// BoundedInputArchive caps allocation sizes, not these scalars. False fails the restore. Always holds in-process.
template<typename T, typename TF512, typename TF256, typename TF128>
struct StorageValidator {

    bool operator()(const Vector<JointGroup>& jointGroups, const RigMetadata& metadata) {
        const std::size_t controlInputCount = metadata.getControlInputCount();
        for (const auto& group : jointGroups) {
            const std::size_t colCount = group.inputIndices.size();
            const std::size_t outputCount = group.outputIndices.size();
            const std::size_t paddedRowCount = group.rowCount;
            const std::size_t valueCount = group.values.template size<T>();
            // The kernel strides the value buffer by group.colCount per block; if it disagrees with
            // inputIndices.size() the value pointer runs past the validated extent.
            if (static_cast<std::size_t>(group.colCount) != colCount) {
                return false;
            }
            // rowCount is the block-padded row count gating both the padded LOD bounds and the value buffer;
            // it must never fall below the live (unpadded) rows.
            if (paddedRowCount < outputCount) {
                return false;
            }
            // The kernel reads colCount * paddedRowCount value floats; division form because the product of two
            // snapshot-derived terms could wrap size_t on 32-bit targets.
            if ((colCount != 0ul) && (paddedRowCount > (valueCount / colCount))) {
                return false;
            }
            if ((colCount == 0ul) && (paddedRowCount != 0ul)) {
                return false;
            }
            // Evaluation indexes lods[lod] up to metadata.lodCount, so the group needs at least that many regions.
            if (group.lods.size() < static_cast<std::size_t>(metadata.lodCount)) {
                return false;
            }
            // Input index values gather from the control input buffer (raw+psd+ml+rbf): inputs[inputIndices[col]].
            for (const auto inputIndex : group.inputIndices) {
                if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                    return false;
                }
            }
            // Output index values are scattered into the joint output buffer (jointAttributeCount).
            for (const auto outputIndex : group.outputIndices) {
                if (outputIndex >= metadata.jointAttributeCount) {
                    return false;
                }
            }
            for (const auto& region : group.lods) {
                if ((region.inputLODs.size > colCount) || (region.inputLODs.sizeAlignedTo4 > colCount) ||
                    (region.inputLODs.sizeAlignedTo8 > colCount)) {
                    return false;
                }
                // The unpadded LOD size gates the output scatter and live outputIndices reads.
                if (region.outputLODs.size > outputCount) {
                    return false;
                }
                // The kernel loops advance in whole blocks and overshoot an unaligned boundary by up to a block,
                // so every bound below is on the walk actually performed, never on the raw boundary fields.
                const std::uint64_t fullStep = TF256::size() * 4ull;
                const std::uint64_t halfStep = TF128::size() * 4ull;
                // Unlike BPCM, outputIndices is not padded (exactly outputCount entries) and the first loop
                // reads full blocks of it unmasked.
                const std::uint64_t secondLast = region.outputLODs.sizePaddedToSecondLastFullBlock;
                const std::uint64_t loop1End = (secondLast > 0u) ? (((secondLast + fullStep - 1u) / fullStep) * fullStep) : 0u;
                if (loop1End > outputCount) {
                    return false;
                }
                // The masked-block loop reads the live remainder (whole quaternions) at each block-strided cursor.
                const std::uint64_t paddedLast = region.outputLODs.sizePaddedToLastFullBlock;
                if (paddedLast > loop1End) {
                    const std::uint64_t loop2End = loop1End + (((paddedLast - loop1End) + fullStep - 1u) / fullStep) * fullStep;
                    const std::uint64_t maskedLive = ((region.outputLODs.size % fullStep) / 4u) * 4u;
                    if ((maskedLive > 0u) && ((loop2End - fullStep + maskedLive) > outputCount)) {
                        return false;
                    }
                }
                // The value pointer advances colCount per walked row across all three loops.
                if (blockWalkEnd(region, fullStep, halfStep) > paddedRowCount) {
                    return false;
                }
            }
        }
        return true;
    }
};

}  // namespace rl4
