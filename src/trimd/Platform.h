// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #define TRIMD_PLATFORM_ARM 1
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__IA32__) || defined(_M_IX86) || defined(__X86__) ||      \
    defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__amd64__) || defined(__amd64) ||                    \
    defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
    #define TRIMD_PLATFORM_X86 1
#endif

// Compile-time ISA detection. Each TRIMD_HAS_* is defined to 1 when the consumer
// TU's baseline includes the extension (per compiler predefines set by the build
// flags), undefined otherwise. Distinct from TRIMD_ENABLE_*: those are consumer
// opt-ins controlling what the library exposes; TRIMD_HAS_* report what the
// compiler is actually allowed to emit in this TU. Useful for downstream
// autodetection layers that want to pick TRIMD_ENABLE_* defaults from the build
// environment instead of carrying their own predefine plumbing.
#if defined(TRIMD_PLATFORM_X86)
    // GCC/Clang set __SSE__/__SSE2__ under -msse/-msse2.
    // MSVC has no __SSE__ predefine but x64 mandates SSE2
    // on x86-32 MSVC, _M_IX86_FP >= 1 means /arch:SSE or higher
    #if defined(__SSE__) || defined(_M_X64) || defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
        #define TRIMD_HAS_SSE 1
    #endif
    #if defined(__AVX__)
        #define TRIMD_HAS_AVX 1
    #endif
    #if defined(__AVX2__)
        #define TRIMD_HAS_AVX2 1
    #endif
    #if defined(__AVX512F__)
        #define TRIMD_HAS_AVX512F 1
    #endif
    // F16C: GCC/Clang set __F16C__ under -mf16c. MSVC has no separate macro but emits F16C
    // intrinsics under /arch:AVX (_MSC_VER >= 1700 gates intrinsic availability)
    #if defined(__F16C__) || (defined(_MSC_VER) && _MSC_VER >= 1700 && defined(__AVX__))
        #define TRIMD_HAS_F16C 1
    #endif
    // FMA3: GCC/Clang set __FMA__ under -mfma.
    // MSVC has no separate macro but emits FMA3 codegen under /arch:AVX2, so __AVX2__ implies FMA there.
    #if defined(__FMA__) || (defined(_MSC_VER) && defined(__AVX2__))
        #define TRIMD_HAS_FMA 1
    #endif
#endif  // TRIMD_PLATFORM_X86

#if defined(TRIMD_PLATFORM_ARM)
    // ACLE: __ARM_NEON is set when NEON/AdvSIMD is the active codegen target.
    // MSVC ARM64 always has NEON (it's part of the aarch64 baseline).
    #if defined(__ARM_NEON) || defined(_M_ARM64) || defined(_M_ARM64EC)
        #define TRIMD_HAS_NEON 1
    #endif
    // ACLE: __ARM_FEATURE_FP16_VECTOR_ARITHMETIC is set by GCC/Clang under
    // -march=armv8.2-a+fp16 / -mfpu=neon-fp-armv8 -mfp16-format=ieee.
    #if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
        #define TRIMD_HAS_NEON_FP16 1
    #endif
    // ARM FMA: aarch64 has FMA (vfmaq_f32) mandatory in AdvSIMD; ARMv7 only with
    // -mfpu=neon-vfpv4 / neon-fp-armv8, which sets __ARM_FEATURE_FMA via ACLE.
    #if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC) || defined(__ARM_FEATURE_FMA)
        #define TRIMD_HAS_FMA 1
    #endif
#endif  // TRIMD_PLATFORM_ARM

#if !defined(TRIMD_TARGET_F16C)
    #if defined(__clang__) || defined(__GNUC__)
        #define TRIMD_TARGET_F16C __attribute__((target("f16c")))
    #else
        #define TRIMD_TARGET_F16C
    #endif
#endif

#if !defined(TRIMD_TARGET_NEON_FP16)
    // GCC spells the AArch64 extension "+fp16"; clang before 18 accepts only the LLVM feature name.
    #if defined(__clang__)
        #define TRIMD_TARGET_NEON_FP16 __attribute__((target("fullfp16")))
    #elif defined(__GNUC__)
        #define TRIMD_TARGET_NEON_FP16 __attribute__((target("+fp16")))
    #else
        #define TRIMD_TARGET_NEON_FP16
    #endif
#endif

#if !defined(TRIMD_TARGET_FMA)
    #if defined(__clang__) || defined(__GNUC__)
        // Lets a single function use FMA3 instructions even if the surrounding TU was
        // compiled without `-mfma`. MSVC has no per-function arch attribute, so on MSVC
        // the surrounding TU must use `/arch:AVX2`.
        #define TRIMD_TARGET_FMA __attribute__((target("fma")))
    #else
        #define TRIMD_TARGET_FMA
    #endif
#endif

#if !defined(TRIMD_TARGET_AVX512F)
    #if defined(__clang__) || defined(__GNUC__)
        // Same idea as TRIMD_TARGET_FMA, but for AVX-512F. Lets a single function use
        // 512-bit ZMM intrinsics even if the surrounding TU was compiled without
        // -mavx512f. MSVC has no per-function arch attribute, so on MSVC the surrounding
        // TU must use /arch:AVX512.
        #define TRIMD_TARGET_AVX512F __attribute__((target("avx512f")))
    #else
        #define TRIMD_TARGET_AVX512F
    #endif
#endif

namespace trimd {

// Symbol-mangling tag for the SIMD scalar-vector templates (TF128, TF256). The
// `Fast` instantiation shares the source body with `Precise`; the distinction
// exists so a single binary can host both a strict-FP arm (built without
// `/fp:fast`) and a relaxed-FP arm (built with `/fp:fast` + `/arch:AVX2`) and
// runtime-dispatch between them on detected CPU capability. Surface types
// (`F128`, `F256`, `F128Fast`, `F256Fast`) alias the two specializations so
// non-template call sites don't see the tag.
enum class FPModel {
    Precise,
    Fast
};

struct CPUFeatures {
    // ARM
    bool NEON;
    bool FP16;
    bool FMA;  // ARM VFPv4 fused multiply-add; mandatory on aarch64, optional on ARMv7
    // X86
    bool SSE;
    bool SSE2;
    bool SSE3;
    bool SSSE3;
    bool SSE41;
    bool SSE42;
    bool AVX;
    bool F16C;
    bool FMA3;  // x86 3-operand FMA - Haswell+ (Intel), Piledriver+ (AMD)
    bool AVX2;
    bool AVX512F;
};

}  // namespace trimd

#ifdef TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION

    #ifdef TRIMD_PLATFORM_X86
        #ifdef _MSC_VER
            #include <intrin.h>
        #endif  // _MSC_VER
    #endif      // TRIMD_PLATFORM_X86

    #ifdef TRIMD_PLATFORM_ARM
        #if defined(__APPLE__)
            #include <mach/machine.h>
            #include <sys/sysctl.h>
            #include <sys/types.h>
        #elif defined(WIN32)
            #include "trimd/PlatformWindows.h"
        #else
            #include <sys/auxv.h>
        #endif
    #endif  // TRIMD_PLATFORM_ARM

namespace trimd {

    #ifdef TRIMD_PLATFORM_X86
        #ifdef _MSC_VER
static void cpuidex(int info[4], int functionid, int subfunctionid) {
    __cpuidex(info, functionid, subfunctionid);
}
        #else
// Based on https://github.com/ispc/ispc/blob/main/builtins/dispatch.c
static void cpuidex(int info[4], int functionid, int subfunctionid) {
    __asm__ __volatile__("cpuid"
                         : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3])
                         : "0"(functionid), "2"(subfunctionid));
}
        #endif

inline CPUFeatures getCPUFeatures() {
    CPUFeatures flags = {};
    int info[4];
    cpuidex(info, 1, 0);
    flags.SSE = (info[3] & (1 << 25)) != 0;
    flags.SSE2 = (info[3] & (1 << 26)) != 0;
    flags.SSE3 = (info[2] & (1 << 0)) != 0;
    flags.SSSE3 = (info[2] & (1 << 9)) != 0;
    flags.SSE41 = (info[2] & (1 << 19)) != 0;
    flags.SSE42 = (info[2] & (1 << 20)) != 0;
    flags.AVX = (info[2] & (1 << 28)) != 0;
    flags.F16C = (info[2] & (1 << 29)) != 0;
    flags.FMA3 = (info[2] & (1 << 12)) != 0;
    // AVX2 and AVX-512F live in CPUID leaf 7, subleaf 0 (EBX bits)
    int info7[4] = {};
    cpuidex(info7, 7, 0);
    flags.AVX2 = (info7[1] & (1 << 5)) != 0;
    flags.AVX512F = (info7[1] & (1 << 16)) != 0;
    return flags;
}
    #endif  // TRIMD_PLATFORM_X86

    #ifdef TRIMD_PLATFORM_ARM
        #if defined(__APPLE__)
static bool getsysctlflag(const char* name) {
    unsigned int enabled = {};
    size_t enabledSize = sizeof(enabled);
    return sysctlbyname(name, &enabled, &enabledSize, nullptr, 0) == 0 && enabled;
}

inline CPUFeatures getCPUFeatures() {
    CPUFeatures flags = {};
    flags.NEON = getsysctlflag("hw.optional.AdvSIMD") || getsysctlflag("hw.optional.neon");
    flags.FP16 = (getsysctlflag("hw.optional.arm.FEAT_FP16") || getsysctlflag("hw.optional.neon_fp16")) &&
                 (getsysctlflag("hw.optional.AdvSIMD_HPFPCvt") || getsysctlflag("hw.optional.neon_hpfp"));
    // On Apple platforms, all currently shipping ARM CPUs are aarch64 where FMA (vfmaq_f32) is mandatory in the NEON/AdvSIMD ISA.
    // Tie FMA to NEON availability rather than probing a separate sysctl key.
    flags.FMA = flags.NEON;
    return flags;
}
        #elif defined(WIN32)
            #ifndef PF_ARM_NEON_INSTRUCTIONS_AVAILABLE
                #define PF_ARM_NEON_INSTRUCTIONS_AVAILABLE 19
            #endif  // PF_ARM_NEON_INSTRUCTIONS_AVAILABLE

inline CPUFeatures getCPUFeatures() {
    CPUFeatures flags = {};
    flags.NEON = IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE) != 0;
    // Heuristic, guessing that FP16 must be available if the below conditions hold true
    flags.FP16 = (IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED) == 0) &&
                 (IsProcessorFeaturePresent(PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE) != 0);
    // PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE maps to VFPv4 fused multiply-add - same instruction used by vfmaq_f32. Mandatory on
    // aarch64; this check still works for both ARM64 and 32-bit ARM because Windows on ARM ships aarch64 today.
    flags.FMA = IsProcessorFeaturePresent(PF_ARM_FMAC_INSTRUCTIONS_AVAILABLE) != 0;
    return flags;
}
        #else
            #ifdef __arm__
                // ARM-32bit
                #ifndef HWCAP_NEON
                    #define HWCAP_NEON (1 << 12)
                #endif  // HWCAP_NEON

                #ifndef HWCAP_VFPv4
                    #define HWCAP_VFPv4 (1 << 16)
                #endif  // HWCAP_VFPv4

                #ifndef HWCAP_FPHP
                    #define HWCAP_FPHP (1 << 22)
                #endif  // HWCAP_FPHP

                #ifndef HWCAP_ASIMDHP
                    #define HWCAP_ASIMDHP (1 << 23)
                #endif  // HWCAP_ASIMDHP
            #else
                // ARM-64bit
                #ifndef HWCAP_ASIMD
                    #define HWCAP_ASIMD (1 << 1)
                #endif  // HWCAP_ASIMD

                #ifndef HWCAP_FPHP
                    #define HWCAP_FPHP (1 << 9)
                #endif  // HWCAP_FPHP

                #ifndef HWCAP_ASIMDHP
                    #define HWCAP_ASIMDHP (1 << 10)
                #endif  // HWCAP_ASIMDHP
            #endif

inline CPUFeatures getCPUFeatures() {
    CPUFeatures flags = {};
    const unsigned long hwcaps = getauxval(AT_HWCAP);
            #ifdef __arm__
    // ARM-32bit: NEON does not imply FMA; need VFPv4 separately.
    flags.NEON = (hwcaps & HWCAP_NEON) != 0;
    flags.FMA = flags.NEON && ((hwcaps & HWCAP_VFPv4) != 0);
            #else
    // ARM-64bit (aarch64): FMA (vfmaq_f32) is part of mandatory AdvSIMD; tie to NEON.
    flags.NEON = (hwcaps & HWCAP_ASIMD) != 0;
    flags.FMA = flags.NEON;
            #endif

    flags.FP16 = ((hwcaps & HWCAP_ASIMDHP) != 0) && ((hwcaps & HWCAP_FPHP) != 0);
    return flags;
}
        #endif  // __linux__
    #endif      // TRIMD_PLATFORM_ARM

}  // namespace trimd

#else

namespace trimd {

inline CPUFeatures getCPUFeatures() {
    return CPUFeatures{};
}

}  // namespace trimd

#endif  // TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION
