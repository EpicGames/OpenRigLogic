// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Sole entry point to trimd: the TRIMD_* configuration is derived from RL_* here, so a TU reaching trimd any other
// way compiles it differently. Detect.h (runtime-detection gate) and Macros.h (pragma macros) are the only other
// sanctioned trimd includes, and both are safe to reach first.
#include "riglogic/system/simd/Detect.h"

#if defined(RL_BUILD_WITH_AVX512F)
    #if defined(RL_BUILD_WITH_HALF_FLOATS) && !defined(TRIMD_ENABLE_F16C)
        #define TRIMD_ENABLE_F16C
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #if !defined(TRIMD_ENABLE_AVX512F)
        #define TRIMD_ENABLE_AVX512F
    #endif
    #if !defined(TRIMD_ENABLE_AVX)
        #define TRIMD_ENABLE_AVX
    #endif
    #if !defined(TRIMD_ENABLE_SSE)
        #define TRIMD_ENABLE_SSE
    #endif
#endif  // RL_BUILD_WITH_AVX512F

#if defined(RL_BUILD_WITH_AVX2)
    #if defined(RL_BUILD_WITH_HALF_FLOATS) && !defined(TRIMD_ENABLE_F16C)
        #define TRIMD_ENABLE_F16C
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #if !defined(TRIMD_ENABLE_AVX)
        #define TRIMD_ENABLE_AVX
    #endif
    #if !defined(TRIMD_ENABLE_SSE)
        #define TRIMD_ENABLE_SSE
    #endif
#endif  // RL_BUILD_WITH_AVX2

#if defined(RL_BUILD_WITH_AVX)
    #if defined(RL_BUILD_WITH_HALF_FLOATS) && !defined(TRIMD_ENABLE_F16C)
        #define TRIMD_ENABLE_F16C
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #if !defined(TRIMD_ENABLE_AVX)
        #define TRIMD_ENABLE_AVX
    #endif
    #if !defined(TRIMD_ENABLE_SSE)
        #define TRIMD_ENABLE_SSE
    #endif
#endif  // RL_BUILD_WITH_AVX

#if defined(RL_BUILD_WITH_SSE) && !defined(TRIMD_ENABLE_SSE)
    #if defined(RL_BUILD_WITH_HALF_FLOATS) && !defined(TRIMD_ENABLE_F16C)
        #define TRIMD_ENABLE_F16C
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #define TRIMD_ENABLE_SSE
#endif  // RL_BUILD_WITH_SSE

#if defined(RL_BUILD_WITH_NEON) && !defined(TRIMD_ENABLE_NEON)
    #if defined(RL_BUILD_WITH_HALF_FLOATS) && !defined(TRIMD_ENABLE_NEON_FP16)
        #define TRIMD_ENABLE_NEON_FP16
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #define TRIMD_ENABLE_NEON
#endif  // RL_BUILD_WITH_NEON

#if defined(RL_BUILD_WITH_FMA) && !defined(TRIMD_ENABLE_FMA)
    #define TRIMD_ENABLE_FMA
#endif  // RL_BUILD_WITH_FMA

#if defined(RL_BUILD_WITH_FAST) && !defined(TRIMD_ENABLE_FAST_FP)
    #define TRIMD_ENABLE_FAST_FP
#endif  // RL_BUILD_WITH_FAST

#include <trimd/TRiMD.h>
