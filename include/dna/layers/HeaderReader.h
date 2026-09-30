// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "dna/Defs.h"

#include <cstdint>

namespace dna {

/**
    @brief Read-only accessors to the header data associated with a rig.
    @warning
        Implementors should inherit from Reader itself and not this class.
*/
class DNAAPI_TYPE HeaderReader {
protected:
    DNAAPI_MEMBER virtual ~HeaderReader();

public:
    virtual std::uint16_t getFileFormatGeneration() const = 0;
    virtual std::uint16_t getFileFormatVersion() const = 0;
};

}  // namespace dna
