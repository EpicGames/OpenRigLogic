// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/conditionaltable/ConditionalTable.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/utils/Extd.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

namespace {

const float clampMin = 0.0f;
const float clampMax = 1.0f;

// Every row below this bound has a valid entry in each parallel array, so the evaluation paths subscript
// stored row indices without per-row range checks.
std::size_t computeRowCount(ConstArrayView<std::uint16_t> inputIndices,
                            ConstArrayView<std::uint16_t> outputIndices,
                            ConstArrayView<float> fromValues,
                            ConstArrayView<float> toValues,
                            ConstArrayView<float> slopeValues,
                            ConstArrayView<float> cutValues) {
    const std::size_t rows = std::min(
        {inputIndices.size(), outputIndices.size(), fromValues.size(), toValues.size(), slopeValues.size(), cutValues.size()});
    // Rows are indexed as uint16_t: saturate rather than let the narrowing cast wrap (65536 rows would read as empty).
    return std::min(rows, static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max()));
}

Vector<std::uint16_t> buildIntervalSkipMap(ConstArrayView<std::uint16_t> inputIndices,
                                           ConstArrayView<std::uint16_t> outputIndices,
                                           std::size_t rowCount,
                                           MemoryResource* memRes) {
    Vector<std::uint16_t> intervalsRemaining{rowCount, {}, memRes};
    for (std::size_t i = {}; i < rowCount;) {
        std::uint16_t intervalCount = 1u;
        const std::uint16_t currentInputIndex = inputIndices[i];
        const std::uint16_t currentOutputIndex = outputIndices[i];
        for (std::size_t j = i + 1ul;
             (j < rowCount) && (currentInputIndex == inputIndices[j]) && (currentOutputIndex == outputIndices[j]);
             ++j) {
            intervalsRemaining[j] = intervalCount++;
        }
        auto start = intervalsRemaining.data() + i;
        std::reverse(start, start + intervalCount);
        i += intervalCount;
    }
    return intervalsRemaining;
}

Vector<RangeMap> buildRangeMap(ConstArrayView<std::uint16_t> inputIndices,
                               ConstArrayView<float> fromValues,
                               ConstArrayView<float> toValues,
                               std::size_t rowCount,
                               MemoryResource* memRes) {
    if (rowCount == 0u) {
        return Vector<RangeMap>{memRes};
    }
    const std::size_t maxIndex = extd::maxOf(inputIndices);
    const std::size_t rangeMapCount = (maxIndex + 1ul);
    Vector<RangeMap> rangeMaps{rangeMapCount, RangeMap{memRes}, memRes};

    for (std::size_t i = {}; i < rowCount; ++i) {
        const std::size_t ri = inputIndices[i];
        auto& map = rangeMaps[ri];
        auto range = map.addRange(fromValues[i], toValues[i]);
        range->rows.push_back(static_cast<std::uint16_t>(i));
    }

    return rangeMaps;
}

}  // namespace

ConditionalTable::ConditionalTable(MemoryResource* memRes) :
    rowCount{},
    rangeMaps{memRes},
    intervalsRemaining{memRes},
    inputIndices{memRes},
    outputIndices{memRes},
    fromValues{memRes},
    toValues{memRes},
    slopeValues{memRes},
    cutValues{memRes},
    inputCount{},
    outputCount{} {
}

ConditionalTable::ConditionalTable(Vector<std::uint16_t>&& inputIndices_,
                                   Vector<std::uint16_t>&& outputIndices_,
                                   Vector<float>&& fromValues_,
                                   Vector<float>&& toValues_,
                                   Vector<float>&& slopeValues_,
                                   Vector<float>&& cutValues_,
                                   std::uint16_t inputCount_,
                                   std::uint16_t outputCount_,
                                   MemoryResource* memRes) :
    rowCount{static_cast<std::uint16_t>(computeRowCount(ConstArrayView<std::uint16_t>{inputIndices_},
                                                        ConstArrayView<std::uint16_t>{outputIndices_},
                                                        ConstArrayView<float>{fromValues_},
                                                        ConstArrayView<float>{toValues_},
                                                        ConstArrayView<float>{slopeValues_},
                                                        ConstArrayView<float>{cutValues_}))},
    rangeMaps{buildRangeMap(ConstArrayView<std::uint16_t>{inputIndices_},
                            ConstArrayView<float>{fromValues_},
                            ConstArrayView<float>{toValues_},
                            rowCount,
                            memRes)},
    intervalsRemaining{buildIntervalSkipMap(ConstArrayView<std::uint16_t>{inputIndices_},
                                            ConstArrayView<std::uint16_t>{outputIndices_},
                                            rowCount,
                                            memRes)},
    inputIndices{std::move(inputIndices_)},
    outputIndices{std::move(outputIndices_)},
    fromValues{std::move(fromValues_)},
    toValues{std::move(toValues_)},
    slopeValues{std::move(slopeValues_)},
    cutValues{std::move(cutValues_)},
    inputCount{inputCount_},
    outputCount{outputCount_} {
}

std::uint16_t ConditionalTable::getRowCount() const {
    // Not inputIndices.size(): only rowCount is guaranteed fully populated.
    return rowCount;
}

std::uint16_t ConditionalTable::getInputCount() const {
    return inputCount;
}

std::uint16_t ConditionalTable::getOutputCount() const {
    return outputCount;
}

ConstArrayView<std::uint16_t> ConditionalTable::getInputIndices() const {
    return inputIndices;
}

ConstArrayView<std::uint16_t> ConditionalTable::getOutputIndices() const {
    return outputIndices;
}

void ConditionalTable::calculateForward(const float* inputs, float* outputs, std::uint16_t requestedRowCount) const {
    std::fill_n(outputs, outputCount, 0.0f);

    const std::uint16_t effectiveRowCount = std::min(requestedRowCount, rowCount);
    for (std::uint16_t row = {}; row < effectiveRowCount; ++row) {
        const float inValue = inputs[inputIndices[row]];
        const float from = fromValues[row];
        const float to = toValues[row];
        if ((from <= inValue) && (inValue <= to)) {
            const std::uint16_t outIndex = outputIndices[row];
            const float slope = slopeValues[row];
            const float cut = cutValues[row];
            outputs[outIndex] += (slope * inValue + cut);
            row = static_cast<std::uint16_t>(row + intervalsRemaining[row]);
        }
    }

    for (std::size_t i = 0ul; i < outputCount; ++i) {
        outputs[i] = extd::clamp(outputs[i], clampMin, clampMax);
    }
}

void ConditionalTable::calculateForward(const float* inputs, float* outputs) const {
    calculateForward(inputs, outputs, static_cast<std::uint16_t>(outputIndices.size()));
}

void ConditionalTable::calculateReverse(float* inputs, const float* outputs, std::uint16_t requestedRowCount) const {
    std::fill_n(inputs, inputCount, 0.0f);

    const std::uint16_t effectiveRowCount = std::min(requestedRowCount, rowCount);
    auto isValidOutput = [this, outputs, effectiveRowCount](std::uint16_t row) {
        // LOD filter: rows at or beyond the active LOD's row count do not participate.
        if (row >= effectiveRowCount) {
            return false;
        }
        const std::uint16_t outIndex = outputIndices[row];
        const float from = fromValues[row];
        const float to = toValues[row];
        const float slope = slopeValues[row];
        const float cut = cutValues[row];
        const float outValue = outputs[outIndex];
        const float inValue = (outValue - cut) / slope;
        return (outValue != 0.0f) && ((from <= inValue) && (inValue <= to));
    };

    auto findRangeWithMostSolutions = [isValidOutput](ConstArrayView<Range> ranges) {
        // Reverse mapping is ambiguous: one output value can map back through several rows. Rows sharing an input
        // index are grouped by (from, to) range; the range with the most valid solutions wins, and its first valid row
        // is solved - rows keep their relative order, matching the forward pass, which also picks the first match.
        std::size_t maxSolutionIndex = {};
        std::size_t maxSolutionCount = {};
        for (std::size_t i = {}; i < ranges.size(); ++i) {
            const auto& range = ranges[i];
            const auto solutionCount =
                static_cast<std::size_t>(std::count_if(range.rows.begin(), range.rows.end(), isValidOutput));
            if (solutionCount > maxSolutionCount) {
                maxSolutionCount = solutionCount;
                maxSolutionIndex = i;
            }
        }
        return maxSolutionIndex;
    };

    auto solveInput = [this, inputs, outputs, effectiveRowCount](ConstArrayView<std::uint16_t> rows) {
        for (auto row : rows) {
            if (row < effectiveRowCount) {
                const std::uint16_t inIndex = inputIndices[row];
                const std::uint16_t outIndex = outputIndices[row];
                const float from = fromValues[row];
                const float to = toValues[row];
                const float slope = slopeValues[row];
                const float cut = cutValues[row];
                const float outValue = outputs[outIndex];
                const float inValue = (outValue - cut) / slope;
                if ((from <= inValue) && (inValue <= to)) {
                    inputs[inIndex] = inValue;
                    return true;
                }
            }
        }
        return false;
    };

    for (const auto& map : rangeMaps) {
        // rangeMaps is dense over input indices, so an input no row uses has an empty map; findRangeWithMostSolutions
        // returns 0 for it, which would subscript ranges[0].
        if (map.ranges.empty()) {
            continue;
        }
        const auto rangeIndex = findRangeWithMostSolutions(ConstArrayView<Range>{map.ranges});
        solveInput(ConstArrayView<std::uint16_t>{map.ranges[rangeIndex].rows});
    }
}

void ConditionalTable::calculateReverse(float* inputs, const float* outputs) const {
    calculateReverse(inputs, outputs, static_cast<std::uint16_t>(outputIndices.size()));
}

}  // namespace rl4
