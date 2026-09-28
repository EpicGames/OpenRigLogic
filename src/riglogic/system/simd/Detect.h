// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// This is where trimd/Platform.h first enters a RigLogic translation unit (SIMD.h includes this header ahead of
// <trimd/TRiMD.h>, which includes Platform.h too, but Platform.h is include-guarded so the first inclusion decides).
// Its inline getCPUFeatures() body is chosen by TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION at that moment (CPUID probe vs
// an all-false stub), so the gate is set here, ahead of the include, and holds whether this header is reached via
// SIMD.h or on its own.
#ifndef RL_DISABLE_RUNTIME_FEATURE_DETECTION
    #ifndef TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION
        #define TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION
    #endif  // TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION
#endif      // RL_DISABLE_RUNTIME_FEATURE_DETECTION

#include "trimd/Platform.h"

#if defined(RL_AUTODETECT_SSE) && !defined(RL_BUILD_WITH_SSE)
    #if defined(TRIMD_HAS_SSE)
        #define RL_BUILD_WITH_SSE 1
        #if defined(RL_AUTODETECT_HALF_FLOATS) && !defined(RL_BUILD_WITH_HALF_FLOATS) && defined(TRIMD_HAS_F16C)
            #define RL_BUILD_WITH_HALF_FLOATS 1
        #endif
    #endif
#endif  // RL_AUTODETECT_SSE

#if defined(RL_AUTODETECT_AVX) && !defined(RL_BUILD_WITH_AVX)
    #if defined(TRIMD_HAS_AVX)
        #define RL_BUILD_WITH_AVX 1
        #if defined(RL_AUTODETECT_HALF_FLOATS) && !defined(RL_BUILD_WITH_HALF_FLOATS) && defined(TRIMD_HAS_F16C)
            #define RL_BUILD_WITH_HALF_FLOATS 1
        #endif
    #endif
#endif  // RL_AUTODETECT_AVX

#if defined(RL_AUTODETECT_AVX2) && !defined(RL_BUILD_WITH_AVX2)
    #if defined(TRIMD_HAS_AVX2)
        #define RL_BUILD_WITH_AVX2 1
        #if defined(RL_AUTODETECT_HALF_FLOATS) && !defined(RL_BUILD_WITH_HALF_FLOATS) && defined(TRIMD_HAS_F16C)
            #define RL_BUILD_WITH_HALF_FLOATS 1
        #endif
    #endif
#endif  // RL_AUTODETECT_AVX2

#if defined(RL_AUTODETECT_AVX512F) && !defined(RL_BUILD_WITH_AVX512F)
    #if defined(TRIMD_HAS_AVX512F)
        #define RL_BUILD_WITH_AVX512F 1
        #if defined(RL_AUTODETECT_HALF_FLOATS) && !defined(RL_BUILD_WITH_HALF_FLOATS) && defined(TRIMD_HAS_F16C)
            #define RL_BUILD_WITH_HALF_FLOATS 1
        #endif
    #endif
#endif  // RL_AUTODETECT_AVX512F

#if defined(RL_AUTODETECT_FMA) && !defined(RL_BUILD_WITH_FMA)
    #if defined(TRIMD_HAS_FMA)
        #define RL_BUILD_WITH_FMA 1
    #endif
#endif  // RL_AUTODETECT_FMA

#if defined(RL_AUTODETECT_NEON) && !defined(RL_BUILD_WITH_NEON)
    #if defined(TRIMD_HAS_NEON)
        #define RL_BUILD_WITH_NEON 1
        #if defined(RL_AUTODETECT_HALF_FLOATS) && !defined(RL_BUILD_WITH_HALF_FLOATS)
            // Half-float support compiles whenever NEON is available; the actual
            // feature check is performed at runtime.
            #define RL_BUILD_WITH_HALF_FLOATS 1
        #endif
    #endif
#endif  // RL_AUTODETECT_NEON
