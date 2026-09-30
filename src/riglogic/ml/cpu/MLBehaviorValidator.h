// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/ml/cpu/Operation.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/types/LODSpec.h"

#include <cstddef>
#include <cstdint>
#include <limits>

namespace rl4 {

namespace ml {

namespace cpu {

// Ceiling on the floats one ML output instance allocates (work buffer plus mask buffer); both sizes are DNA- or
// snapshot-supplied scalars a small file can inflate into GBs.
constexpr std::uint64_t kMaxWorkBufferFloats = std::uint64_t{1} << 26;

// Shared by the build() and load() paths. Each concrete set bounds its own ops via validate(MLBehaviorBounds); this
// drives the per-type/set walk and checks the cross-cutting invariants (container lockstep, active-op indices).
struct MLBehaviorValidator {
    static bool validate(const Matrix<LODSpec<std::uint16_t>>& lods,
                         const Matrix<OperationSet::Pointer>& mlOperations,
                         const Vector<Matrix<std::uint16_t>>& bufferSizes,
                         std::uint32_t meshRegionCount,
                         const RigMetadata& metadata) {
        const std::size_t mlTypeCount = mlOperations.size();
        // The three per-type containers are indexed in lockstep (mlOperations[t][s], lods[t][s], bufferSizes[t][s]),
        // and the public API iterates types up to the metadata's count.
        if ((lods.size() != mlTypeCount) || (bufferSizes.size() != mlTypeCount) ||
            (static_cast<std::size_t>(metadata.mlTypeCount) != mlTypeCount)) {
            return false;
        }

        MLBehaviorBounds bounds{};
        bounds.controlInputCount = metadata.getControlInputCount();
        bounds.meshRegionCount = static_cast<std::size_t>(meshRegionCount);
        bounds.lodCount = static_cast<std::size_t>(metadata.lodCount);

        // Set and op indices are uint16 everywhere; capping both keeps per-type op totals within uint32.
        constexpr std::size_t kMaxIndex = static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max());
        // One workBuffer serves the whole rig, so sum across all types; the mask buffer (one float per mesh region)
        // comes out of the same allocation budget.
        std::uint64_t totalWorkBufferFloats = static_cast<std::uint64_t>(meshRegionCount);
        if (totalWorkBufferFloats > kMaxWorkBufferFloats) {
            return false;
        }

        for (std::size_t typeIdx = {}; typeIdx < mlTypeCount; ++typeIdx) {
            const auto& opSets = mlOperations[typeIdx];
            const auto& lodsForType = lods[typeIdx];
            const auto& bufferSizesForType = bufferSizes[typeIdx];
            const std::size_t opSetCount = opSets.size();
            // OutputInstance sizes workBufferOffsetsPerOperationSet / lods / bufferSizes by op-set in lockstep, and the
            // per-LOD active-op lists index the op-set's ops.
            if ((lodsForType.size() != opSetCount) || (bufferSizesForType.size() != opSetCount)) {
                return false;
            }
            if (opSetCount > kMaxIndex) {
                return false;
            }

            bounds.bufferSizesForType = &bufferSizesForType;

            for (std::size_t opSetIdx = {}; opSetIdx < opSetCount; ++opSetIdx) {
                // Every active-op index a LOD list can hand execute() must address a real op in this set.
                const auto& indicesPerLOD = lodsForType[opSetIdx].indicesPerLOD;
                if (indicesPerLOD.size() < bounds.lodCount) {
                    return false;
                }
                const std::size_t opCount = bufferSizesForType[opSetIdx].size();
                if (opCount > kMaxIndex) {
                    return false;
                }
                for (const auto bufferSize : bufferSizesForType[opSetIdx]) {
                    totalWorkBufferFloats += static_cast<std::uint64_t>(bufferSize);
                }
                if (totalWorkBufferFloats > kMaxWorkBufferFloats) {
                    return false;
                }
                // getMLOperationCount() hands this deserialized scalar to callers as the loop bound for the
                // public single-op calculate(), whose index execute() subscripts ops with.
                if (static_cast<std::size_t>(lodsForType[opSetIdx].count) > opCount) {
                    return false;
                }
                for (const auto& activeOpIndices : indicesPerLOD) {
                    for (const auto opIdx : activeOpIndices) {
                        if (static_cast<std::size_t>(opIdx) >= opCount) {
                            return false;
                        }
                    }
                }

                // Each set validates its own ops (it owns their private data); nullptr means a load-path type/enum went
                // wrong before the ops were even populated.
                bounds.opSetIndex = opSetIdx;
                const OperationSet* opSet = opSets[opSetIdx].get();
                if ((opSet == nullptr) || !opSet->validate(bounds)) {
                    return false;
                }
            }
        }

        return true;
    }
};

}  // namespace cpu

}  // namespace ml

}  // namespace rl4
