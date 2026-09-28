// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "dna/Defs.h"
#include "dna/types/Aliases.h"

namespace dna {

struct DNAAPI_TYPE VersionInfo {
    DNAAPI_MEMBER static int getMajorVersion();
    DNAAPI_MEMBER static int getMinorVersion();
    DNAAPI_MEMBER static int getPatchVersion();
    DNAAPI_MEMBER static StringView getVersionString();
};

}  // namespace dna
