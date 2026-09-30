// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/types/PaddedBlockView.h"

#include <cstdint>

namespace rl4 {

struct ColumnLOD {
    std::uint32_t size;
    std::uint32_t sizeAlignedTo4;
    std::uint32_t sizeAlignedTo8;

    ColumnLOD() = default;
    explicit ColumnLOD(std::uint32_t size_) :
        size{size_},
        sizeAlignedTo4{size - (size % 4u)},
        sizeAlignedTo8{size - (size % 8u)} {
    }

    ColumnLOD(std::uint32_t size_, std::uint32_t sizeAlignedTo4_, std::uint32_t sizeAlignedTo8_) :
        size{size_},
        sizeAlignedTo4{sizeAlignedTo4_},
        sizeAlignedTo8{sizeAlignedTo8_} {
    }

    template<class Archive>
    void serialize(Archive& archive) {
        archive(size, sizeAlignedTo4, sizeAlignedTo8);
    }
};

using RowLOD = PaddedBlockView;

struct LODRegion {
    ColumnLOD inputLODs;
    RowLOD outputLODs;

    template<class Archive>
    void serialize(Archive& archive) {
        archive(inputLODs, outputLODs);
    }
};

// Final output-cursor position of the kernels' block-strided walk over one LOD region: each loop advances by a WHOLE
// step while cursor < boundary, so an unaligned boundary is overshot by up to a step and validators must bound by this
// walk, not the raw fields. 64-bit so hostile 32-bit values cannot wrap; both steps are lane counts, never zero.
inline std::uint64_t blockWalkEnd(const LODRegion& region, std::uint64_t fullStep, std::uint64_t halfStep) {
    std::uint64_t cursor = 0u;
    if (region.outputLODs.sizePaddedToSecondLastFullBlock > cursor) {
        cursor += (((region.outputLODs.sizePaddedToSecondLastFullBlock - cursor) + fullStep - 1u) / fullStep) * fullStep;
    }
    if (region.outputLODs.sizePaddedToLastFullBlock > cursor) {
        cursor += (((region.outputLODs.sizePaddedToLastFullBlock - cursor) + fullStep - 1u) / fullStep) * fullStep;
    }
    if (region.outputLODs.size > cursor) {
        cursor += (((region.outputLODs.size - cursor) + halfStep - 1u) / halfStep) * halfStep;
    }
    return cursor;
}

}  // namespace rl4
