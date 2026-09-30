// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/conditionaltable/ConditionalTable.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

// Shared by every ConditionalTable producer; bounds differ per producer, so the caller passes them.
// On restore the derived members (rowCount, intervalsRemaining, rangeMaps) are deserialized raw, bypassing the ctor,
// so this checks structure (parallel-array lengths vs rowCount, range-map row references), not just index values.
// rangeMaps are checked even for forward-only producers: a snapshot carries them regardless of who calls calculateReverse.
struct ConditionalTableValidator {
    static bool validate(const ConditionalTable& table, std::size_t inputBound, std::size_t outputBound) {
        const std::size_t rowCount = static_cast<std::size_t>(table.rowCount);

        // fill_n(outputs, outputCount, 0) / fill_n(inputs, inputCount, 0) must fit the destination buffers.
        if (table.outputCount > outputBound) {
            return false;
        }
        if (table.inputCount > inputBound) {
            return false;
        }

        // On restore the deserialized rowCount is independent of the arrays the row loops subscript for row < rowCount.
        if ((table.inputIndices.size() < rowCount) || (table.outputIndices.size() < rowCount) ||
            (table.fromValues.size() < rowCount) || (table.toValues.size() < rowCount) || (table.slopeValues.size() < rowCount) ||
            (table.cutValues.size() < rowCount) || (table.intervalsRemaining.size() < rowCount)) {
            return false;
        }

        // Index values must fit the runtime buffers the forward/reverse paths read and write.
        for (std::size_t row = 0ul; row < rowCount; ++row) {
            if (table.inputIndices[row] >= inputBound) {
                return false;
            }
            if (table.outputIndices[row] >= outputBound) {
                return false;
            }
            // The forward skip `row = uint16(row + intervalsRemaining[row])` truncates, so an oversized skip wraps the
            // cursor backward and calculate() loops forever; the builder's skips always stay inside the current interval.
            if (static_cast<std::size_t>(table.intervalsRemaining[row]) > (rowCount - 1ul - row)) {
                return false;
            }
        }

        // calculateReverse subscripts every table row a range map references; empty range maps are legitimate,
        // but each referenced row must be < rowCount.
        for (const auto& map : table.rangeMaps) {
            for (const auto& range : map.ranges) {
                for (const auto rowRef : range.rows) {
                    if (static_cast<std::size_t>(rowRef) >= rowCount) {
                        return false;
                    }
                }
            }
        }

        return true;
    }
};

}  // namespace rl4
