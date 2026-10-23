// Copyright Epic Games, Inc. All Rights Reserved.

#include "trimdtests/Defs.h"

#include "trimd/TRiMD.h"

TEST(CPUFeatureDetection, Platform) {
    trimd::CPUFeatures features = trimd::getCPUFeatures();
    if (features.NEON) {
        // If NEON is detected, none of the x86 CPU features should be available
        ASSERT_FALSE(features.SSE);
        ASSERT_FALSE(features.SSE2);
        ASSERT_FALSE(features.SSE3);
        ASSERT_FALSE(features.SSSE3);
        ASSERT_FALSE(features.SSE41);
        ASSERT_FALSE(features.SSE42);
        ASSERT_FALSE(features.AVX);
        ASSERT_FALSE(features.AVX2);
        ASSERT_FALSE(features.AVX512F);
        ASSERT_FALSE(features.F16C);
        ASSERT_FALSE(features.FMA3);
    } else {
        // If NEON is not detected, x86 features might be found (depending on hardware)
        // But FP16 should not be available without NEON
        ASSERT_FALSE(features.FP16);
        // AVX-512F implies AVX2 implies AVX implies SSE (CPU feature hierarchy)
        if (features.AVX512F) {
            ASSERT_TRUE(features.AVX2);
        }
        if (features.AVX2) {
            ASSERT_TRUE(features.AVX);
        }
        if (features.AVX) {
            ASSERT_TRUE(features.SSE);
        }
    }
}

#if defined(TRIMD_PLATFORM_X86) && defined(TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION)
// Pins the implication only: a reported AVX-family feature must be backed by OSXSAVE and the matching XCR0
// state bits. The negative path (OS with XSAVE disabled) cannot be forced on CI hardware, so the #UD this
// defends against is not reproduced here.
TEST(CPUFeatureDetection, AVXFamilyRequiresOSXSAVE) {
    const trimd::CPUFeatures features = trimd::getCPUFeatures();
    int info[4] = {};
    trimd::cpuidex(info, 1, 0);
    const bool osxsave = (info[2] & (1 << 27)) != 0;
    const unsigned long long xcr0 = osxsave ? trimd::xgetbv(0) : 0ull;
    const bool ymmEnabled = (xcr0 & 0x6ull) == 0x6ull;
    if (features.AVX || features.F16C || features.FMA3 || features.AVX2) {
        ASSERT_TRUE(osxsave);
        ASSERT_TRUE(ymmEnabled);
    }
    if (features.AVX512F) {
    #if defined(__APPLE__)
        // ZMM state is enabled lazily by XNU, so only the eager YMM bits are checked there.
        ASSERT_TRUE(ymmEnabled);
    #else
        ASSERT_EQ(xcr0 & 0xE6ull, 0xE6ull);
    #endif
    }
}
#endif  // TRIMD_PLATFORM_X86 && TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION
