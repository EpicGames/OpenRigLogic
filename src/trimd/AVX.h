// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#ifdef TRIMD_ENABLE_AVX
    #include "trimd/Macros.h"
    #include "trimd/Platform.h"
    #include "trimd/Shim.h"

    #include <immintrin.h>

namespace trimd {

namespace avx {

// TF256 is parameterized on a compile-time FPModel tag. The two instantiations
// (`Precise` and `Fast`) share the source body - the tag serves purely as a
// symbol-mangling discriminator so the same kernel template instantiated on
// `TF256<FPModel::Precise>` and `TF256<FPModel::Fast>` produces distinct,
// coexisting symbols in the binary. That lets a single binary host both an
// AVX-only kernel and an AVX2+FMA kernel; the runtime dispatcher picks based
// on detected CPU features.
//
// Codegen difference between the two arms comes from the TU's compile flags:
// the `Fast` instantiation should live in a TU compiled with `/arch:AVX2 +
// /fp:fast` (MSVC) or `-mavx2 -mfma -ffp-contract=fast` (GCC/Clang) so the
// compiler can fuse `a*b + c` into `vfmadd` and emit AVX2-class instructions.
// The `Precise` instantiation should live in a TU compiled with `/arch:AVX`
// so it stays AVX-only and runs on Sandy/Ivy Bridge CPUs.
//
// `using F256 = TF256<FPModel::Precise>;` provides the backwards-compatible alias so
// existing uses of `trimd::avx::F256` continue to work unchanged.
// `using F256Fast = TF256<FPModel::Fast>;` is the explicit Fast-arm alias used
// by the Fast kernel TUs.
template<FPModel Mode = FPModel::Precise>
struct TF256 {
    using value_type = float;

    __m256 data;

    FORCE_INLINE TF256() :
        data{_mm256_setzero_ps()} {
    }

    FORCE_INLINE explicit TF256(__m256 value) :
        data{value} {
    }

    FORCE_INLINE explicit TF256(float value) :
        TF256{_mm256_set1_ps(value)} {
    }

    FORCE_INLINE TF256(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8) :
        data{_mm256_set_ps(v8, v7, v6, v5, v4, v3, v2, v1)} {
    }

    static FORCE_INLINE TF256 fromAlignedSource(const float* source) {
        return TF256{_mm256_load_ps(source)};
    }

    static FORCE_INLINE TF256 fromUnalignedSource(const float* source) {
        return TF256{_mm256_loadu_ps(source)};
    }

    static FORCE_INLINE TF256 loadSingleValue(const float* source) {
        const __m256i mask = _mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, -1);
        return TF256{_mm256_maskload_ps(source, mask)};
    }

    #ifdef TRIMD_ENABLE_F16C
    static TF256 fromAlignedSource(const std::uint16_t* source);
    static TF256 fromUnalignedSource(const std::uint16_t* source);
    static TF256 loadSingleValue(const std::uint16_t* source);
    #endif  // TRIMD_ENABLE_F16C

    template<typename T>
    static void prefetchT0(const T* source) {
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
        _mm_prefetch(reinterpret_cast<const char*>(source), _MM_HINT_T0);
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
    }

    template<typename T>
    static void prefetchT1(const T* source) {
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
        _mm_prefetch(reinterpret_cast<const char*>(source), _MM_HINT_T1);
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
    }

    template<typename T>
    static void prefetchT2(const T* source) {
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
        _mm_prefetch(reinterpret_cast<const char*>(source), _MM_HINT_T2);
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
    }

    template<typename T>
    static void prefetchNTA(const T* source) {
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
        _mm_prefetch(reinterpret_cast<const char*>(source), _MM_HINT_NTA);
    #if defined(__clang__) || defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
    }

    FORCE_INLINE void alignedLoad(const float* source) {
        data = _mm256_load_ps(source);
    }

    FORCE_INLINE void unalignedLoad(const float* source) {
        data = _mm256_loadu_ps(source);
    }

    FORCE_INLINE void alignedStore(float* dest) const {
        _mm256_store_ps(dest, data);
    }

    FORCE_INLINE void unalignedStore(float* dest) const {
        _mm256_storeu_ps(dest, data);
    }

    #ifdef TRIMD_ENABLE_F16C
    void alignedLoad(const std::uint16_t* source);
    void unalignedLoad(const std::uint16_t* source);
    void alignedStore(std::uint16_t* dest) const;
    void unalignedStore(std::uint16_t* dest) const;
    #endif  // TRIMD_ENABLE_F16C

    FORCE_INLINE float sum() const {
        // (data[3] + data[7], data[2] + data[6], data[1] + data[5], data[0] + data[4])
        const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(data, 1), _mm256_castps256_ps128(data));
        // (-, -, data[1] + data[3] + data[5] + data[7], data[0] + data[2] + data[4] + data[6])
        const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
        // (-, -, -, data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7])
        const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
        return _mm_cvtss_f32(x32);
    }

    FORCE_INLINE TF256& operator+=(const TF256& rhs) {
        data = _mm256_add_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF256& operator-=(const TF256& rhs) {
        data = _mm256_sub_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF256& operator*=(const TF256& rhs) {
        data = _mm256_mul_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF256& operator/=(const TF256& rhs) {
        data = _mm256_div_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF256& operator&=(const TF256& rhs) {
        data = _mm256_and_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF256& operator|=(const TF256& rhs) {
        data = _mm256_or_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF256& operator^=(const TF256& rhs) {
        data = _mm256_xor_ps(data, rhs.data);
        return *this;
    }

    static constexpr std::size_t size() {
        return sizeof(__m256) / sizeof(float);
    }

    static constexpr std::size_t alignment() {
        return alignof(__m256);
    }
};

// Backwards-compatible alias for existing code paths that use `trimd::avx::F256` - the
// strict-FP arm. Inside-namespace alias keeps name lookup at all existing call
// sites unchanged.
using F256 = TF256<FPModel::Precise>;

// Explicit Fast-arm alias. Used by the per-arm Fast kernel TUs and the
// dispatcher in Utils.h to produce distinct template instantiations from the
// Precise arm. Same body / methods; distinct mangled symbols.
using F256Fast = TF256<FPModel::Fast>;

    #ifdef TRIMD_ENABLE_F16C

// Plain `inline` (not FORCE_INLINE) is required: TRIMD_TARGET_F16C elevates
// codegen to `f16c` only for these functions, so when the consumer TU has no
// `-mf16c` baseline (the runtime-dispatch case, or any build that defines
// TRIMD_ENABLE_F16C without the matching flag) GCC refuses to inline a
// target-elevated callee into a caller that lacks the ISA and errors on
// always_inline. Plain `inline` lets the compiler emit an out-of-line wrapper
// with the elevated codegen, which is also what runtime dispatch wants: an
// addressable function symbol the dispatcher can invoke conditionally after a
// CPU feature check.
template<FPModel Mode>
TRIMD_TARGET_F16C inline TF256<Mode> TF256<Mode>::fromAlignedSource(const std::uint16_t* source) {
    return TF256<Mode>{_mm256_cvtph_ps(_mm_load_si128(reinterpret_cast<const __m128i*>(source)))};
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline TF256<Mode> TF256<Mode>::fromUnalignedSource(const std::uint16_t* source) {
    return TF256<Mode>{_mm256_cvtph_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(source)))};
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline TF256<Mode> TF256<Mode>::loadSingleValue(const std::uint16_t* source) {
    return TF256<Mode>{_mm256_cvtph_ps(_mm_loadu_si16(source))};
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF256<Mode>::alignedLoad(const std::uint16_t* source) {
    data = _mm256_cvtph_ps(_mm_load_si128(reinterpret_cast<const __m128i*>(source)));
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF256<Mode>::unalignedLoad(const std::uint16_t* source) {
    data = _mm256_cvtph_ps(_mm_loadu_si128(reinterpret_cast<const __m128i*>(source)));
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF256<Mode>::alignedStore(std::uint16_t* dest) const {
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wold-style-cast"
            #if !defined(__clang__)
                #pragma GCC diagnostic ignored "-Wuseless-cast"
            #endif
        #endif
    _mm_store_si128(reinterpret_cast<__m128i*>(dest), _mm256_cvtps_ph(data, _MM_FROUND_CUR_DIRECTION));
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic pop
        #endif
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF256<Mode>::unalignedStore(std::uint16_t* dest) const {
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wold-style-cast"
            #if !defined(__clang__)
                #pragma GCC diagnostic ignored "-Wuseless-cast"
            #endif
        #endif
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dest), _mm256_cvtps_ph(data, _MM_FROUND_CUR_DIRECTION));
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic pop
        #endif
}

    #endif  // TRIMD_ENABLE_F16C

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator==(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_cmp_ps(lhs.data, rhs.data, _CMP_EQ_OQ)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator!=(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_cmp_ps(lhs.data, rhs.data, _CMP_NEQ_OQ)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator<(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_cmp_ps(lhs.data, rhs.data, _CMP_LT_OQ)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator<=(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_cmp_ps(lhs.data, rhs.data, _CMP_LE_OQ)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator>(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_cmp_ps(lhs.data, rhs.data, _CMP_GT_OQ)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator>=(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_cmp_ps(lhs.data, rhs.data, _CMP_GE_OQ)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator+(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) += rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator-(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) -= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator*(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) *= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator/(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) /= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator&(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) &= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator|(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) |= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator^(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>(lhs) ^= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> operator~(const TF256<Mode>& rhs) {
    TF256<Mode> result{};
    result = (result == result);
    return (result ^= rhs);
}

template<FPModel Mode>
FORCE_INLINE void transpose(TF256<Mode>& row0,
                            TF256<Mode>& row1,
                            TF256<Mode>& row2,
                            TF256<Mode>& row3,
                            TF256<Mode>& row4,
                            TF256<Mode>& row5,
                            TF256<Mode>& row6,
                            TF256<Mode>& row7) {
    __m256 t0 = _mm256_unpacklo_ps(row0.data, row1.data);
    __m256 t1 = _mm256_unpackhi_ps(row0.data, row1.data);
    __m256 t2 = _mm256_unpacklo_ps(row2.data, row3.data);
    __m256 t3 = _mm256_unpackhi_ps(row2.data, row3.data);
    __m256 t4 = _mm256_unpacklo_ps(row4.data, row5.data);
    __m256 t5 = _mm256_unpackhi_ps(row4.data, row5.data);
    __m256 t6 = _mm256_unpacklo_ps(row6.data, row7.data);
    __m256 t7 = _mm256_unpackhi_ps(row6.data, row7.data);
    __m256 tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
    __m256 tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
    __m256 tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
    __m256 tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
    __m256 tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
    __m256 tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
    __m256 tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
    __m256 tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));
    row0.data = _mm256_permute2f128_ps(tt0, tt4, 0x20);
    row1.data = _mm256_permute2f128_ps(tt1, tt5, 0x20);
    row2.data = _mm256_permute2f128_ps(tt2, tt6, 0x20);
    row3.data = _mm256_permute2f128_ps(tt3, tt7, 0x20);
    row4.data = _mm256_permute2f128_ps(tt0, tt4, 0x31);
    row5.data = _mm256_permute2f128_ps(tt1, tt5, 0x31);
    row6.data = _mm256_permute2f128_ps(tt2, tt6, 0x31);
    row7.data = _mm256_permute2f128_ps(tt3, tt7, 0x31);
}

    // Fused multiply-add: returns a*b + c. Uses _mm256_fmadd_ps when the TU has
    // opted into FMA codegen via TRIMD_ENABLE_FMA; otherwise falls back to
    // `(a*b) + c`. The FPModel tag on TF256 is orthogonal - it only mangles
    // symbols. The TU's compile flags (and TRIMD_ENABLE_FMA define) decide
    // whether vfmadd actually emits. Plain `inline` (not FORCE_INLINE) for the
    // same reason as the F16C block above: TRIMD_TARGET_FMA would clash with
    // always_inline when the consumer TU lacks `-mfma`.
    #ifdef TRIMD_ENABLE_FMA
template<FPModel Mode>
TRIMD_TARGET_FMA inline TF256<Mode> fma(const TF256<Mode>& a, const TF256<Mode>& b, const TF256<Mode>& c) {
    return TF256<Mode>{_mm256_fmadd_ps(a.data, b.data, c.data)};
}
    #else
template<FPModel Mode>
FORCE_INLINE TF256<Mode> fma(const TF256<Mode>& a, const TF256<Mode>& b, const TF256<Mode>& c) {
    return TF256<Mode>{_mm256_add_ps(_mm256_mul_ps(a.data, b.data), c.data)};
}
    #endif  // TRIMD_ENABLE_FMA

template<FPModel Mode>
FORCE_INLINE TF256<Mode> abs(const TF256<Mode>& rhs) {
    const __m256 ABSMASK = _mm256_castsi256_ps(_mm256_set1_epi32(~(1 << 31)));
    return TF256<Mode>{_mm256_and_ps(ABSMASK, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> andnot(const TF256<Mode>& lhs, const TF256<Mode>& rhs) {
    return TF256<Mode>{_mm256_andnot_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF256<Mode> rsqrt(const TF256<Mode>& rhs) {
    #ifndef TRIMD_ENABLE_FAST_INVERSE_SQRT
    return TF256<Mode>{_mm256_rsqrt_ps(rhs.data)};
    #else
    // Pure AVX2-only implementation:
    //
    // const __m256i shifted = _mm256_srli_epi32(_mm256_castps_si256(rhs.data), 1);
    // const __m256i subtracted = _mm256_sub_epi32(_mm256_set1_epi32(0x5f1ffff9), shifted);
    // F256 result{_mm256_castsi256_ps(subtracted)};
    // result *= F256{0.703952253f} * (F256{2.38924456f} - rhs * result * result);
    // return result;
    //
    // Combination of SSE and AVX:
    const __m128 lower = _mm256_castps256_ps128(rhs.data);
    const __m128 upper = _mm256_extractf128_ps(rhs.data, 1);
    const __m128i shiftedLower = _mm_srli_epi32(_mm_castps_si128(lower), 1);
    const __m128i shiftedUpper = _mm_srli_epi32(_mm_castps_si128(upper), 1);
    const __m128i subtractedLower = _mm_sub_epi32(_mm_set1_epi32(0x5f1ffff9), shiftedLower);
    const __m128i subtractedUpper = _mm_sub_epi32(_mm_set1_epi32(0x5f1ffff9), shiftedUpper);
    TF256<Mode> result{
        _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_castsi128_ps(subtractedLower)), _mm_castsi128_ps(subtractedUpper), 1)};
    result *= TF256<Mode>{0.703952253f} * (TF256<Mode>{2.38924456f} - rhs * result * result);
    return result;
    #endif  // TRIMD_ENABLE_FAST_INVERSE_SQRT
}

}  // namespace avx

}  // namespace trimd

#endif  // TRIMD_ENABLE_AVX
