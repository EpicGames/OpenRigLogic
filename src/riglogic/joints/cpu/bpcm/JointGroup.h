// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/joints/cpu/utils/LODRegion.h"
#include "riglogic/types/PaddedBlockView.h"

#include <cstdint>

namespace rl4 {

namespace bpcm {

struct JointGroup {
    std::uint32_t valuesOffset;
    std::uint32_t inputIndicesOffset;
    std::uint32_t outputIndicesOffset;
    std::uint32_t lodsOffset;
    std::uint32_t outputRotationIndicesOffset;
    std::uint32_t outputRotationLODsOffset;
    std::uint32_t valuesSize;
    std::uint32_t colCount;
    std::uint32_t rowCount;
    // Block height this group's storage was optimized for; selects the kernel width.
    std::uint32_t blockHeight;

    template<class Archive>
    void serialize(Archive& archive) {
        archive(valuesOffset,
                inputIndicesOffset,
                outputIndicesOffset,
                lodsOffset,
                outputRotationIndicesOffset,
                outputRotationLODsOffset,
                valuesSize,
                colCount,
                rowCount,
                blockHeight);
    }
};

}  // namespace bpcm

}  // namespace rl4
