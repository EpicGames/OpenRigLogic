// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "dna/Configuration.h"
#include "dna/Defs.h"
#include "dna/Reader.h"
#include "dna/types/Aliases.h"

namespace dna {

class DNAAPI_TYPE StreamReader : public Reader {
public:
    DNAAPI_MEMBER static const sc::StatusCode SignatureMismatchError;
    DNAAPI_MEMBER static const sc::StatusCode VersionMismatchError;
    DNAAPI_MEMBER static const sc::StatusCode InvalidDataError;
    DNAAPI_MEMBER static const sc::StatusCode InvalidConfigError;

public:
    DNAAPI_MEMBER ~StreamReader() override;
    /**
       @brief read data from stream into internal structures.
    */
    virtual void read() = 0;
};

}  // namespace dna
