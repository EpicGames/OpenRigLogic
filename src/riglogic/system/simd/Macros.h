// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// TRiMD's scoped FP-model pragmas (TRIMD_PRECISE_FP_BEGIN/END, TRIMD_FAST_FP_BEGIN/END) for TUs that open the region
// BEFORE trimd is parsed. trimd/Macros.h includes nothing, so it cannot reach trimd/Platform.h ahead of SIMD.h.
#include <trimd/Macros.h>
