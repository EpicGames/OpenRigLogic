// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#ifdef TRIMD_ENABLE_AVX512F
    #include "trimd/Macros.h"
    #include "trimd/Platform.h"
    #include "trimd/Shim.h"

    #include <immintrin.h>

namespace trimd {

namespace avx512 {

// See note at the top of trimd/AVX.h. TF512 mirrors avx::TF256 but holds a 16-lane
// `__m512` register. Templated on FPModel for symbol-mangling; both instantiations
// share the source body. Codegen difference between Precise and Fast arms comes
// from the TU's compile flags (typically `/arch:AVX512 /fp:fast` on MSVC or
// `-mavx512f -ffp-contract=fast` on Clang/GCC for the Fast arm).
template<FPModel Mode = FPModel::Precise>
struct TF512 {
    using value_type = float;

    __m512 data;

    FORCE_INLINE TF512() :
        data{_mm512_setzero_ps()} {
    }

    FORCE_INLINE explicit TF512(__m512 value) :
        data{value} {
    }

    FORCE_INLINE explicit TF512(float value) :
        TF512{_mm512_set1_ps(value)} {
    }

    FORCE_INLINE TF512(float v1,
                       float v2,
                       float v3,
                       float v4,
                       float v5,
                       float v6,
                       float v7,
                       float v8,
                       float v9,
                       float v10,
                       float v11,
                       float v12,
                       float v13,
                       float v14,
                       float v15,
                       float v16) :
        data{_mm512_set_ps(v16, v15, v14, v13, v12, v11, v10, v9, v8, v7, v6, v5, v4, v3, v2, v1)} {
    }

    static FORCE_INLINE TF512 fromAlignedSource(const float* source) {
        return TF512{_mm512_load_ps(source)};
    }

    static FORCE_INLINE TF512 fromUnalignedSource(const float* source) {
        return TF512{_mm512_loadu_ps(source)};
    }

    static FORCE_INLINE TF512 loadSingleValue(const float* source) {
        // Mask all-zero except lane 0; AVX-512 mask-load fills others with 0.
        return TF512{_mm512_maskz_loadu_ps(__mmask16{1u}, source)};
    }

    #ifdef TRIMD_ENABLE_F16C
    static TF512 fromAlignedSource(const std::uint16_t* source);
    static TF512 fromUnalignedSource(const std::uint16_t* source);
    static TF512 loadSingleValue(const std::uint16_t* source);
    #endif  // TRIMD_ENABLE_F16C

    template<typename T>
    static FORCE_INLINE void prefetchT0(const T* source) {
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
    static FORCE_INLINE void prefetchT1(const T* source) {
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
    static FORCE_INLINE void prefetchT2(const T* source) {
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
    static FORCE_INLINE void prefetchNTA(const T* source) {
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
        data = _mm512_load_ps(source);
    }

    FORCE_INLINE void unalignedLoad(const float* source) {
        data = _mm512_loadu_ps(source);
    }

    FORCE_INLINE void alignedStore(float* dest) const {
        _mm512_store_ps(dest, data);
    }

    FORCE_INLINE void unalignedStore(float* dest) const {
        _mm512_storeu_ps(dest, data);
    }

    #ifdef TRIMD_ENABLE_F16C
    void alignedLoad(const std::uint16_t* source);
    void unalignedLoad(const std::uint16_t* source);
    void alignedStore(std::uint16_t* dest) const;
    void unalignedStore(std::uint16_t* dest) const;
    #endif  // TRIMD_ENABLE_F16C

    FORCE_INLINE float sum() const {
        return _mm512_reduce_add_ps(data);
    }

    FORCE_INLINE TF512& operator+=(const TF512& rhs) {
        data = _mm512_add_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF512& operator-=(const TF512& rhs) {
        data = _mm512_sub_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF512& operator*=(const TF512& rhs) {
        data = _mm512_mul_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF512& operator/=(const TF512& rhs) {
        data = _mm512_div_ps(data, rhs.data);
        return *this;
    }

    // Bitwise ops go through the integer-typed AVX-512F intrinsics (`*_si512`).
    // The `*_ps` variants (vandps/vorps/vxorps/vandnps on zmm) were added in
    // AVX-512DQ, not F. Casts are free at codegen time - same vpand/vpor/vpxor
    // /vpandn zmm instruction emits either way.
    FORCE_INLINE TF512& operator&=(const TF512& rhs) {
        data = _mm512_castsi512_ps(_mm512_and_si512(_mm512_castps_si512(data), _mm512_castps_si512(rhs.data)));
        return *this;
    }

    FORCE_INLINE TF512& operator|=(const TF512& rhs) {
        data = _mm512_castsi512_ps(_mm512_or_si512(_mm512_castps_si512(data), _mm512_castps_si512(rhs.data)));
        return *this;
    }

    FORCE_INLINE TF512& operator^=(const TF512& rhs) {
        data = _mm512_castsi512_ps(_mm512_xor_si512(_mm512_castps_si512(data), _mm512_castps_si512(rhs.data)));
        return *this;
    }

    static constexpr std::size_t size() {
        return sizeof(__m512) / sizeof(float);
    }

    static constexpr std::size_t alignment() {
        return alignof(__m512);
    }
};

// Backwards-compatible alias for code paths using `trimd::avx512::F512` (strict-FP arm).
using F512 = TF512<FPModel::Precise>;

// Explicit Fast-arm alias used by the Fast kernel TUs and the runtime dispatcher.
using F512Fast = TF512<FPModel::Fast>;

    #ifdef TRIMD_ENABLE_F16C

template<FPModel Mode>
FORCE_INLINE TF512<Mode> TF512<Mode>::fromAlignedSource(const std::uint16_t* source) {
    return TF512<Mode>{_mm512_cvtph_ps(_mm256_load_si256(reinterpret_cast<const __m256i*>(source)))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> TF512<Mode>::fromUnalignedSource(const std::uint16_t* source) {
    return TF512<Mode>{_mm512_cvtph_ps(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source)))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> TF512<Mode>::loadSingleValue(const std::uint16_t* source) {
    return TF512<Mode>{_mm512_cvtph_ps(_mm256_castsi128_si256(_mm_loadu_si16(source)))};
}

template<FPModel Mode>
FORCE_INLINE void TF512<Mode>::alignedLoad(const std::uint16_t* source) {
    data = _mm512_cvtph_ps(_mm256_load_si256(reinterpret_cast<const __m256i*>(source)));
}

template<FPModel Mode>
FORCE_INLINE void TF512<Mode>::unalignedLoad(const std::uint16_t* source) {
    data = _mm512_cvtph_ps(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source)));
}

template<FPModel Mode>
FORCE_INLINE void TF512<Mode>::alignedStore(std::uint16_t* dest) const {
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wold-style-cast"
            #if !defined(__clang__)
                #pragma GCC diagnostic ignored "-Wuseless-cast"
            #endif
        #endif
    _mm256_store_si256(reinterpret_cast<__m256i*>(dest), _mm512_cvtps_ph(data, _MM_FROUND_CUR_DIRECTION));
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic pop
        #endif
}

template<FPModel Mode>
FORCE_INLINE void TF512<Mode>::unalignedStore(std::uint16_t* dest) const {
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wold-style-cast"
            #if !defined(__clang__)
                #pragma GCC diagnostic ignored "-Wuseless-cast"
            #endif
        #endif
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dest), _mm512_cvtps_ph(data, _MM_FROUND_CUR_DIRECTION));
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic pop
        #endif
}

    #endif  // TRIMD_ENABLE_F16C

// AVX-512 compare instructions return `__mmask16` (one bit per lane). Convert
// to a vector of all-1s / all-0s per lane to preserve the AVX/SSE API contract
// that operator== returns the same vector type.
//
// `_mm512_maskz_set1_epi32(mask, -1)` is the AVX-512F-only way to do this:
// lanes selected by the mask get 0xFFFFFFFF, the rest are zeroed. The shorter
// `_mm512_movm_epi32` does the same thing in one op but requires AVX-512DQ,
// so we avoid it to keep this header strictly AVX-512F.
namespace detail {

FORCE_INLINE __m512 maskToVector(__mmask16 mask) {
    return _mm512_castsi512_ps(_mm512_maskz_set1_epi32(mask, -1));
}

}  // namespace detail

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator==(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>{detail::maskToVector(_mm512_cmp_ps_mask(lhs.data, rhs.data, _CMP_EQ_OQ))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator!=(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>{detail::maskToVector(_mm512_cmp_ps_mask(lhs.data, rhs.data, _CMP_NEQ_OQ))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator<(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>{detail::maskToVector(_mm512_cmp_ps_mask(lhs.data, rhs.data, _CMP_LT_OQ))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator<=(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>{detail::maskToVector(_mm512_cmp_ps_mask(lhs.data, rhs.data, _CMP_LE_OQ))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator>(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>{detail::maskToVector(_mm512_cmp_ps_mask(lhs.data, rhs.data, _CMP_GT_OQ))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator>=(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>{detail::maskToVector(_mm512_cmp_ps_mask(lhs.data, rhs.data, _CMP_GE_OQ))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator+(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) += rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator-(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) -= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator*(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) *= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator/(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) /= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator&(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) &= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator|(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) |= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator^(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    return TF512<Mode>(lhs) ^= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> operator~(const TF512<Mode>& rhs) {
    TF512<Mode> result{};
    result = (result == result);
    return (result ^= rhs);
}

    // AVX-512F has fmadd in the base ISA, so this branch is always available when the
    // TU is compiled with /arch:AVX512 (MSVC) / -mavx512f (Clang/GCC). We still gate
    // it on TRIMD_ENABLE_FMA to keep the cross-ISA contract consistent.
    #ifdef TRIMD_ENABLE_FMA
template<FPModel Mode>
FORCE_INLINE TF512<Mode> fma(const TF512<Mode>& a, const TF512<Mode>& b, const TF512<Mode>& c) {
    return TF512<Mode>{_mm512_fmadd_ps(a.data, b.data, c.data)};
}
    #else
template<FPModel Mode>
FORCE_INLINE TF512<Mode> fma(const TF512<Mode>& a, const TF512<Mode>& b, const TF512<Mode>& c) {
    return TF512<Mode>{_mm512_add_ps(_mm512_mul_ps(a.data, b.data), c.data)};
}
    #endif

template<FPModel Mode>
FORCE_INLINE TF512<Mode> abs(const TF512<Mode>& rhs) {
    // AND with sign-bit-cleared mask. Go through the integer AVX-512F variant
    // (_mm512_and_si512) - _mm512_and_ps would require AVX-512DQ.
    const __m512i ABSMASK = _mm512_set1_epi32(~(1 << 31));
    return TF512<Mode>{_mm512_castsi512_ps(_mm512_and_si512(ABSMASK, _mm512_castps_si512(rhs.data)))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> andnot(const TF512<Mode>& lhs, const TF512<Mode>& rhs) {
    // _mm512_andnot_ps requires AVX-512DQ; the integer variant is AVX-512F.
    return TF512<Mode>{_mm512_castsi512_ps(_mm512_andnot_si512(_mm512_castps_si512(lhs.data), _mm512_castps_si512(rhs.data)))};
}

template<FPModel Mode>
FORCE_INLINE TF512<Mode> rsqrt(const TF512<Mode>& rhs) {
    #ifndef TRIMD_ENABLE_FAST_INVERSE_SQRT
    // AVX-512F's _mm512_rsqrt14_ps gives ~14-bit relative precision, the closest
    // analog to AVX's _mm256_rsqrt_ps.
    return TF512<Mode>{_mm512_rsqrt14_ps(rhs.data)};
    #else
    // Quake-style fast inverse square root: bit-cast, magic-constant subtract,
    // one Newton-Raphson refinement. Pattern mirrors avx::rsqrt's else branch.
    const __m512i shifted = _mm512_srli_epi32(_mm512_castps_si512(rhs.data), 1);
    const __m512i subtracted = _mm512_sub_epi32(_mm512_set1_epi32(0x5f1ffff9), shifted);
    TF512<Mode> result{_mm512_castsi512_ps(subtracted)};
    result *= TF512<Mode>{0.703952253f} * (TF512<Mode>{2.38924456f} - rhs * result * result);
    return result;
    #endif  // TRIMD_ENABLE_FAST_INVERSE_SQRT
}

}  // namespace avx512

}  // namespace trimd

#endif  // TRIMD_ENABLE_AVX512F
