// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

// Shared by the DNA factory path and the snapshot load() path. Bounds come from the rig metadata, never from
// counts carried alongside the data.
struct BlendShapesValidator {
    static bool validate(ConstArrayView<std::uint16_t> lods,
                         ConstArrayView<std::uint16_t> inputIndices,
                         ConstArrayView<std::uint16_t> outputIndices,
                         const RigMetadata& metadata) {
        // Exactly lodCount entries, not just <=: lods[lod] is read for any lod < lodCount, and that value is
        // calculate()'s loop bound.
        if (lods.size() != static_cast<std::size_t>(metadata.lodCount)) {
            return false;
        }
        for (const auto rowCount : lods) {
            if ((static_cast<std::size_t>(rowCount) > inputIndices.size()) ||
                (static_cast<std::size_t>(rowCount) > outputIndices.size())) {
                return false;
            }
        }

        const std::size_t controlInputCount = metadata.getControlInputCount();
        for (const auto inputIndex : inputIndices) {
            if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                return false;
            }
        }
        for (const auto outputIndex : outputIndices) {
            if (outputIndex >= metadata.blendShapeCount) {
                return false;
            }
        }

        return true;
    }
};

}  // namespace rl4
