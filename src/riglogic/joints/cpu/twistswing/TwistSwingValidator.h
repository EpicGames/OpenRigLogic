// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/twistswing/TwistSwingSetup.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

// Shared by the DNA builder and snapshot load() paths. Input indices address the control input buffer,
// output indices the joint attribute output buffer.
struct TwistSwingValidator {
    static bool validate(const Vector<TwistSwingSetup>& setups, const RigMetadata& metadata) {
        const std::size_t controlInputCount =
            static_cast<std::size_t>(metadata.rawControlCount) + static_cast<std::size_t>(metadata.psdControlCount) +
            static_cast<std::size_t>(metadata.mlControlCount) + static_cast<std::size_t>(metadata.rbfControlCount);
        // calculate() indexes a fixed 3-entry function-pointer table by twist axis and calls through it; an
        // out-of-range axis is an out-of-bounds read followed by an indirect call.
        constexpr std::size_t twistAxisCount = 3ul;
        for (const auto& setup : setups) {
            if ((static_cast<std::size_t>(setup.swingTwistAxis) >= twistAxisCount) ||
                (static_cast<std::size_t>(setup.twistTwistAxis) >= twistAxisCount)) {
                return false;
            }

            // A populated input quad has exactly 4 components; each blend weight expands to one output quad.
            const bool swingInputValid = setup.swingInputIndices.empty() || (setup.swingInputIndices.size() == 4ul);
            const bool twistInputValid = setup.twistInputIndices.empty() || (setup.twistInputIndices.size() == 4ul);
            const bool swingOutputValid = (setup.swingBlendWeights.size() * 4ul) == setup.swingOutputIndices.size();
            const bool twistOutputValid = (setup.twistBlendWeights.size() * 4ul) == setup.twistOutputIndices.size();
            if (!swingInputValid || !twistInputValid || !swingOutputValid || !twistOutputValid) {
                return false;
            }
            // invTwist is populated only by an input quad (merged setups legitimately carry twist outputs with
            // just the swing quad); twist blend weights with no quad at all would blend an uninitialized rotation.
            if (!setup.twistBlendWeights.empty() && setup.swingInputIndices.empty() && setup.twistInputIndices.empty()) {
                return false;
            }
            for (const auto inputIndex : setup.swingInputIndices) {
                if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                    return false;
                }
            }
            for (const auto inputIndex : setup.twistInputIndices) {
                if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                    return false;
                }
            }
            for (const auto outputIndex : setup.swingOutputIndices) {
                if (outputIndex >= metadata.jointAttributeCount) {
                    return false;
                }
            }
            for (const auto outputIndex : setup.twistOutputIndices) {
                if (outputIndex >= metadata.jointAttributeCount) {
                    return false;
                }
            }
        }
        return true;
    }
};

}  // namespace rl4
