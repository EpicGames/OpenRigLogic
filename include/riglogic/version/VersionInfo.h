// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/Defs.h"
#include "riglogic/types/Aliases.h"

namespace rl4 {

struct RLAPI_TYPE VersionInfo {
    RLAPI_MEMBER static int getMajorVersion();
    RLAPI_MEMBER static int getMinorVersion();
    RLAPI_MEMBER static int getPatchVersion();
    RLAPI_MEMBER static StringView getVersionString();
};

}  // namespace rl4
