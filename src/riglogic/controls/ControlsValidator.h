// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/conditionaltable/ConditionalTable.h"
#include "riglogic/conditionaltable/ConditionalTableValidator.h"
#include "riglogic/controls/ControlsInputInstance.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstddef>

namespace rl4 {

// Shared by the DNA factory path (reject -> nullptr) and the snapshot load() path (reject -> markMalformed).
// The GUI-to-raw table is mapped both directions at runtime with the same index-to-bound pairing (inputIndices ->
// guiControlCount, outputIndices -> controlInputCount), so one ConditionalTableValidator call covers both.
// initialValues are applied as inputBuffer[index] = value with inputBuffer sized controlInputCount.
struct ControlsValidator {
    static bool validate(const ConditionalTable& guiToRawMapping,
                         ConstArrayView<ControlInitializer> initialValues,
                         const RigMetadata& metadata) {
        const std::size_t guiControlCount = static_cast<std::size_t>(metadata.guiControlCount);
        const std::size_t controlInputCount = metadata.getControlInputCount();

        if (!ConditionalTableValidator::validate(guiToRawMapping, guiControlCount, controlInputCount)) {
            return false;
        }

        for (const auto& initializer : initialValues) {
            if (static_cast<std::size_t>(initializer.index) >= controlInputCount) {
                return false;
            }
        }

        return true;
    }
};

}  // namespace rl4
