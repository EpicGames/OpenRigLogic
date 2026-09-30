// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/types/BoundedInputArchive.h"

#include <cstdint>

namespace rl4 {

// The flag members must not be streamed as raw bool: forming a bool object from a wire byte other than
// 0/1 is UB before any validator can run, so the load direction checks the byte first (writers only ever
// emit 0/1, so anything else is corruption). Same wire layout as a bool - a single byte.

using ConfigurationFlag = bool Configuration::*;

// Function-local static: an inline variable would need C++17.
inline ConstArrayView<ConfigurationFlag> configurationFlags() {
    static const ConfigurationFlag flags[] = {&Configuration::loadJoints,
                                              &Configuration::loadBlendShapes,
                                              &Configuration::loadAnimatedMaps,
                                              &Configuration::loadMachineLearnedBehavior,
                                              &Configuration::loadRBFBehavior,
                                              &Configuration::loadTwistSwingBehavior};
    return {flags, sizeof(flags) / sizeof(flags[0])};
}

inline void serializeFlags(BoundedInputArchive& archive, Configuration& config) {
    for (auto flag : configurationFlags()) {
        std::uint8_t wire = {};
        archive(wire);
        if (wire > 1u) {
            archive.markMalformed();
        }
        config.*flag = (wire != 0u);
    }
}

inline void serializeFlags(terse::BinaryOutputArchive<BoundedIOStream>& archive, Configuration& config) {
    for (auto flag : configurationFlags()) {
        std::uint8_t wire = static_cast<std::uint8_t>((config.*flag) ? 1u : 0u);
        archive(wire);
    }
}

// Kept a template: terse detects free serializers by probing serialize(T&, T&), which only a generic
// first parameter satisfies. The direction split happens in serializeFlags at instantiation.
template<class Archive>
void serialize(Archive& archive, Configuration& config) {
    archive(config.calculationType, config.floatingPointType, config.floatingPointModel);
    serializeFlags(archive, config);
    archive(config.translationType,
            config.rotationType,
            config.scaleType,
            config.translationPruningThreshold,
            config.rotationPruningThreshold,
            config.scalePruningThreshold);
}

}  // namespace rl4
