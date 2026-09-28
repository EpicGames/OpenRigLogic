// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/conditionaltable/ConditionalTable.h"
#include "riglogic/conditionaltable/ConditionalTableValidator.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

// Shared by the DNA factory path (raw index arrays) and the restore path (deserialized ConditionalTable).
// Bounds come from the rig metadata, never from counts carried alongside the data.
struct AnimatedMapsValidator {
    static bool validate(ConstArrayView<std::uint16_t> lods, const ConditionalTable& conditionals, const RigMetadata& metadata) {
        // Exactly lodCount entries, not just <=: lods[lod] is read for any lod < lodCount.
        if (lods.size() != static_cast<std::size_t>(metadata.lodCount)) {
            return false;
        }
        for (const auto rowCount : lods) {
            if (static_cast<std::size_t>(rowCount) > conditionals.getOutputIndices().size()) {
                return false;
            }
        }
        return ConditionalTableValidator::validate(conditionals, metadata.getControlInputCount(), metadata.animatedMapCount);
    }

    static bool validate(ConstArrayView<std::uint16_t> lods,
                         ConstArrayView<std::uint16_t> inputIndices,
                         ConstArrayView<std::uint16_t> outputIndices,
                         std::uint16_t outputCount,
                         const RigMetadata& metadata) {
        if (lods.size() != static_cast<std::size_t>(metadata.lodCount)) {
            return false;
        }
        for (const auto rowCount : lods) {
            if ((static_cast<std::size_t>(rowCount) > inputIndices.size()) ||
                (static_cast<std::size_t>(rowCount) > outputIndices.size())) {
                return false;
            }
        }

        // calculateForward zeroes outputCount floats of the metadata-sized output buffer, independent of indices.
        if (outputCount > metadata.animatedMapCount) {
            return false;
        }

        const std::size_t controlInputCount = metadata.getControlInputCount();
        for (const auto inputIndex : inputIndices) {
            if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                return false;
            }
        }
        for (const auto outputIndex : outputIndices) {
            if (outputIndex >= metadata.animatedMapCount) {
                return false;
            }
        }

        return true;
    }
};

}  // namespace rl4
