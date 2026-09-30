// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

namespace ml {

// Guards both the DNA build() path and the snapshot load() path. Input indices are bounded by getControlInputCount(),
// output indices by jointAttributeCount. The caller passes the exact rotation spans (quaternion 4, Euler 3) of the
// adapter/transformer it selects; an over-wide span would reject Euler rigs whose rotation base sits 3 below the end.
struct MLJointsValidator {
    static bool validate(const Matrix<std::uint16_t>& inputIndices,
                         const Matrix<std::uint16_t>& outputIndices,
                         const Matrix<std::uint16_t>& inputRotationBaseIndices,
                         const Matrix<std::uint16_t>& outputRotationBaseIndices,
                         const Matrix<std::uint16_t>& uniqueTranslationBaseIndices,
                         const Matrix<std::uint16_t>& uniqueRotationBaseIndices,
                         const Matrix<std::uint16_t>& uniqueScaleBaseIndices,
                         std::size_t inputRotationSpan,
                         std::size_t outputRotationSpan,
                         std::size_t uniqueRotationSpan,
                         const RigMetadata& metadata) {
        const std::size_t controlInputCount = metadata.getControlInputCount();
        const std::size_t jointAttributeCount = static_cast<std::size_t>(metadata.jointAttributeCount);
        const std::size_t lodCount = static_cast<std::size_t>(metadata.lodCount);

        // calculate(lod) subscripts all seven matrices by lod in lockstep, so each must cover the rig's LOD count.
        if ((inputIndices.size() < lodCount) || (outputIndices.size() < lodCount) ||
            (inputRotationBaseIndices.size() < lodCount) || (outputRotationBaseIndices.size() < lodCount) ||
            (uniqueTranslationBaseIndices.size() < lodCount) || (uniqueRotationBaseIndices.size() < lodCount) ||
            (uniqueScaleBaseIndices.size() < lodCount)) {
            return false;
        }

        // Iterate exactly the LOD range the evaluator dereferences; bounding by inputIndices.size() would let a snapshot
        // with extra rows drive this loop past the shorter sibling matrices.
        for (std::size_t lod = {}; lod < lodCount; ++lod) {
            // Main gather/scatter loop runs to inputIndices[lod].size() and reads outputIndices[lod] at the same index.
            if (inputIndices[lod].size() > outputIndices[lod].size()) {
                return false;
            }
            for (const auto inputIndex : inputIndices[lod]) {
                if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                    return false;
                }
            }
            for (const auto outputIndex : outputIndices[lod]) {
                if (static_cast<std::size_t>(outputIndex) >= jointAttributeCount) {
                    return false;
                }
            }

            // The rotation adapter walks the two lists in parallel, reading/writing span components from each base.
            if (inputRotationBaseIndices[lod].size() != outputRotationBaseIndices[lod].size()) {
                return false;
            }
            if (!withinSpan(inputRotationBaseIndices[lod], inputRotationSpan, controlInputCount) ||
                !withinSpan(outputRotationBaseIndices[lod], outputRotationSpan, jointAttributeCount)) {
                return false;
            }

            // Coordinate-system transformers read/write inputBuffer at the unique base indices over a component span.
            if (!withinSpan(uniqueTranslationBaseIndices[lod], 3u, controlInputCount) ||
                !withinSpan(uniqueRotationBaseIndices[lod], uniqueRotationSpan, controlInputCount) ||
                !withinSpan(uniqueScaleBaseIndices[lod], 3u, controlInputCount)) {
                return false;
            }
        }

        return true;
    }

private:
    // baseIndex + span <= count for every baseIndex, evaluated without overflow.
    static bool withinSpan(const Vector<std::uint16_t>& baseIndices, std::size_t span, std::size_t count) {
        if (span > count) {
            return baseIndices.empty();
        }
        const std::size_t limit = count - span;
        for (const auto baseIndex : baseIndices) {
            if (static_cast<std::size_t>(baseIndex) > limit) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace ml

}  // namespace rl4
