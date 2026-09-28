// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/psdnet/PSDNetImpl.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

// Shared by the DNA factory path and the snapshot load() path. On the factory path the builder derives psds/offsets
// from consistent data but the index VALUES still need bounding; on the restore path everything is deserialized raw.
struct PSDNetValidator {
    static bool validate(const Matrix<std::uint16_t>& inputLODs,
                         const Matrix<std::uint16_t>& outputLODs,
                         ConstArrayView<std::uint16_t> inputIndicesPerPSD,
                         ConstArrayView<PSD> psds,
                         std::uint16_t psdMinIndex,
                         std::uint16_t psdMaxIndex,
                         const RigMetadata& metadata) {
        const std::size_t controlInputCount = metadata.getControlInputCount();
        const std::size_t lodCount = static_cast<std::size_t>(metadata.lodCount);

        // calculate() indexes inputLODs[lod]/outputLODs[lod] for lod up to the rig's LOD count.
        if ((inputLODs.size() < lodCount) || (outputLODs.size() < lodCount)) {
            return false;
        }

        // Each PSD's [offset, offset + size) walks inputIndicesPerPSD; offset and size are independent deserialized
        // size_t values, so bound the sum against the container without trusting either.
        for (const auto& psd : psds) {
            if ((psd.offset > inputIndicesPerPSD.size()) || (psd.size > (inputIndicesPerPSD.size() - psd.offset))) {
                return false;
            }
        }

        // Every value walked out of inputIndicesPerPSD indexes the clamp/control buffer.
        for (const auto inputIndex : inputIndicesPerPSD) {
            if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                return false;
            }
        }

        // Per-LOD input indices read/write the clamp/control buffer directly.
        for (std::size_t lod = 0ul; lod < lodCount; ++lod) {
            for (const auto inputIndex : inputLODs[lod]) {
                if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                    return false;
                }
            }
        }

        // Output indices subscript psds[outputIndex - psdMinIndex] and write inputs[outputIndex]; each must fall in
        // [psdMinIndex, psdMaxIndex], map to a psds entry, and stay within the control buffer.
        for (std::size_t lod = 0ul; lod < lodCount; ++lod) {
            for (const auto outputIndex : outputLODs[lod]) {
                if ((outputIndex < psdMinIndex) || (outputIndex > psdMaxIndex)) {
                    return false;
                }
                if (static_cast<std::size_t>(outputIndex) >= controlInputCount) {
                    return false;
                }
                if ((static_cast<std::size_t>(outputIndex) - static_cast<std::size_t>(psdMinIndex)) >= psds.size()) {
                    return false;
                }
            }
        }

        return true;
    }
};

}  // namespace rl4
