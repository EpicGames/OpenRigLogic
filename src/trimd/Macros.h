// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if !defined(FORCE_INLINE)
    #if defined(_MSC_VER)
        #define FORCE_INLINE __forceinline
    #else
        #define FORCE_INLINE inline __attribute__((always_inline))
    #endif
#endif

// Apple Clang's __clang_major__ runs ahead of the LLVM release it ships (12.0.0 = LLVM 10,
// 12.0.5 = 11, 13.0.0 = 12, 17.0.0 = 19), so its checks go through __apple_build_version__.
#if defined(__apple_build_version__)
    #define TRIMD_CLANG_AT_LEAST(llvmMajor, appleBuild) (__apple_build_version__ >= (appleBuild))
#elif defined(__clang__)
    #define TRIMD_CLANG_AT_LEAST(llvmMajor, appleBuild) (__clang_major__ >= (llvmMajor))
#endif

// Clang's float_control push/pop stack exists only where the target implements strict FP:
// x86 since clang 12, AArch64 since clang 18, never on 32-bit ARM. Elsewhere the pragma is
// rejected under -Wignored-pragmas and END can only restore defaults, not the ambient state.
// AArch64 is tested first because ARM64EC also defines the x86-64 compatibility macros.
#if defined(__clang__)
    #if defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
        #define TRIMD_CLANG_FLOAT_CONTROL TRIMD_CLANG_AT_LEAST(18, 17000000)
    #elif defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
        #define TRIMD_CLANG_FLOAT_CONTROL TRIMD_CLANG_AT_LEAST(12, 13000000)
    #else
        #define TRIMD_CLANG_FLOAT_CONTROL 0
    #endif
#endif

// TRIMD_FAST_FP_BEGIN/END narrow `/fp:fast`-equivalent FP-model semantics to a
// specific lexical region (typically a kernel function template). The region
// gains permission to fuse `a*b + c` into FMA, reassociate FP expressions, and
// drop denormal/NaN guarantees. Surrounding code in the same TU keeps the
// compiler's default strict FP model - useful when only the SIMD hot loop wants
// the relaxation. Opt-in via TRIMD_ENABLE_FAST_FP (no-op otherwise).
//
// Note: the ISA enable (`/arch:AVX2` for FMA intrinsics on MSVC) is orthogonal
// and must still be set per-TU. These macros only affect FP model, not codegen
// target capability.
#if !defined(TRIMD_FAST_FP_BEGIN)
    #if defined(TRIMD_ENABLE_FAST_FP)
        #if defined(__clang__)
            // Clang's `fp contract(fast)` is the narrow option - just enables FMA
            // contraction without the rest of fast-math.
            #if TRIMD_CLANG_FLOAT_CONTROL
                #define TRIMD_FAST_FP_BEGIN _Pragma("float_control(push)") _Pragma("clang fp contract(fast)")
                #define TRIMD_FAST_FP_END _Pragma("float_control(pop)")
            #else
                #define TRIMD_FAST_FP_BEGIN _Pragma("clang fp contract(fast)")
                #define TRIMD_FAST_FP_END _Pragma("clang fp contract(on)")
            #endif
        #elif defined(_MSC_VER)
            // MSVC: float_control(precise, off) enables fast-FP semantics (reassoc,
            // denormal flush, no NaN guarantee) BUT it does not by itself enable
            // mul-add contraction into FMA. The separate fp_contract pragma controls
            // that. We need both for `sum += a*b` to fuse into vfmadd231ps.
            #define TRIMD_FAST_FP_BEGIN __pragma(float_control(precise, off, push)) __pragma(fp_contract(on))
            #define TRIMD_FAST_FP_END __pragma(fp_contract(off)) __pragma(float_control(pop))
        #elif defined(__GNUC__)
            #define TRIMD_FAST_FP_BEGIN _Pragma("GCC push_options") _Pragma("GCC optimize(\"fast-math\")")
            #define TRIMD_FAST_FP_END _Pragma("GCC pop_options")
        #else
            #define TRIMD_FAST_FP_BEGIN
            #define TRIMD_FAST_FP_END
        #endif
    #else
        #define TRIMD_FAST_FP_BEGIN
        #define TRIMD_FAST_FP_END
    #endif  // TRIMD_ENABLE_FAST_FP
#endif

// TRIMD_FAST_FP_AVAILABLE is defined when TRIMD_FAST_FP_BEGIN actually delivers relaxed
// FP semantics in this TU. MSVC pragmas can only restrict, never relax, so there the TU
// itself must be compiled with /fp:fast (detected via _M_FP_FAST); the Clang/GCC scoped
// pragmas relax on their own.
#if defined(TRIMD_ENABLE_FAST_FP) && !defined(TRIMD_FAST_FP_AVAILABLE)
    #if defined(__clang__) || defined(__GNUC__) || (defined(_MSC_VER) && defined(_M_FP_FAST))
        #define TRIMD_FAST_FP_AVAILABLE
    #endif
#endif

// TRIMD_PRECISE_FP_BEGIN/END are the strict counterpart: they pin precise FP semantics
// (no contraction, no reassociation) for a lexical region regardless of the TU's or the
// build system's ambient FP flags. This is what keeps a Precise kernel arm bit-reproducible
// when the surrounding build uses fast-math defaults (e.g. GCC's -ffp-contract=fast at
// optimization levels, or host build systems that compile everything with /fp:fast).
// Opt-in via TRIMD_ENABLE_FAST_FP, like the macros above (no-op otherwise).
#if !defined(TRIMD_PRECISE_FP_BEGIN)
    #if defined(TRIMD_ENABLE_FAST_FP)
        #if defined(__clang__)
            // `precise, on` clears every ambient fast-math flag (reassociation, reciprocal,
            // approximate functions, no-NaN/Inf/signed-zero) but leaves within-statement
            // contraction on, so `contract(off)` is still needed; the fallback without the
            // float_control stack can only address contraction and reassociation.
            #if TRIMD_CLANG_FLOAT_CONTROL
                #define TRIMD_PRECISE_FP_BEGIN _Pragma("float_control(precise, on, push)") _Pragma("clang fp contract(off)")
                #define TRIMD_PRECISE_FP_END _Pragma("float_control(pop)")
            #elif TRIMD_CLANG_AT_LEAST(11, 12050000)
                #define TRIMD_PRECISE_FP_BEGIN _Pragma("clang fp contract(off) reassociate(off)")
                #define TRIMD_PRECISE_FP_END _Pragma("clang fp contract(on)")
            #else
                #define TRIMD_PRECISE_FP_BEGIN _Pragma("clang fp contract(off)")
                #define TRIMD_PRECISE_FP_END _Pragma("clang fp contract(on)")
            #endif
        #elif defined(_MSC_VER)
            #define TRIMD_PRECISE_FP_BEGIN __pragma(float_control(precise, on, push)) __pragma(fp_contract(off))
            #define TRIMD_PRECISE_FP_END __pragma(float_control(pop))
        #elif defined(__GNUC__)
            #define TRIMD_PRECISE_FP_BEGIN                                                                                       \
                _Pragma("GCC push_options") _Pragma("GCC optimize(\"no-fast-math\",\"fp-contract=off\")")
            #define TRIMD_PRECISE_FP_END _Pragma("GCC pop_options")
        #else
            #define TRIMD_PRECISE_FP_BEGIN
            #define TRIMD_PRECISE_FP_END
        #endif
    #else
        #define TRIMD_PRECISE_FP_BEGIN
        #define TRIMD_PRECISE_FP_END
    #endif  // TRIMD_ENABLE_FAST_FP
#endif
