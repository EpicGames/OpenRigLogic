// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#ifdef TRIMD_ENABLE_SSE
    #include "trimd/Fallback.h"
    #include "trimd/Macros.h"
    #include "trimd/Platform.h"
    #include "trimd/Shim.h"

    #include <immintrin.h>

namespace trimd {

namespace sse {

// TF128 is parameterized on the FPModel tag for symbol-mangling purposes - see
// the note at the top of trimd/AVX.h. `using F128 = TF128<FPModel::Precise>` is
// the backwards-compatible alias, `using F128Fast = TF128<FPModel::Fast>` is the Fast-arm
// tag. Body is identical between the two instantiations; codegen difference
// comes from the TU's compile flags.
template<FPModel Mode = FPModel::Precise>
struct TF128 {
    using value_type = float;

    __m128 data;

    FORCE_INLINE TF128() :
        data{_mm_setzero_ps()} {
    }

    FORCE_INLINE explicit TF128(__m128 value) :
        data{value} {
    }

    FORCE_INLINE explicit TF128(float value) :
        TF128{_mm_set1_ps(value)} {
    }

    FORCE_INLINE TF128(float v1, float v2, float v3, float v4) :
        data{_mm_set_ps(v4, v3, v2, v1)} {
    }

    static FORCE_INLINE TF128 fromAlignedSource(const float* source) {
        return TF128{_mm_load_ps(source)};
    }

    static FORCE_INLINE TF128 fromUnalignedSource(const float* source) {
        return TF128{_mm_loadu_ps(source)};
    }

    static FORCE_INLINE TF128 loadSingleValue(const float* source) {
        return TF128{_mm_load_ss(source)};
    }

    #ifdef TRIMD_ENABLE_F16C
    static TF128 fromAlignedSource(const std::uint16_t* source);
    static TF128 fromUnalignedSource(const std::uint16_t* source);
    static TF128 loadSingleValue(const std::uint16_t* source);
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
        data = _mm_load_ps(source);
    }

    FORCE_INLINE void unalignedLoad(const float* source) {
        data = _mm_loadu_ps(source);
    }

    FORCE_INLINE void alignedStore(float* dest) const {
        _mm_store_ps(dest, data);
    }

    FORCE_INLINE void unalignedStore(float* dest) const {
        _mm_storeu_ps(dest, data);
    }

    #ifdef TRIMD_ENABLE_F16C
    void alignedLoad(const std::uint16_t* source);
    void unalignedLoad(const std::uint16_t* source);
    void alignedStore(std::uint16_t* dest) const;
    void unalignedStore(std::uint16_t* dest) const;
    #endif  // TRIMD_ENABLE_F16C

    FORCE_INLINE float sum() const {
        __m128 temp = _mm_movehdup_ps(data);
        __m128 result = _mm_add_ps(data, temp);
        temp = _mm_movehl_ps(temp, result);
        result = _mm_add_ss(result, temp);
        return _mm_cvtss_f32(result);
    }

    FORCE_INLINE TF128& operator+=(const TF128& rhs) {
        data = _mm_add_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator-=(const TF128& rhs) {
        data = _mm_sub_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator*=(const TF128& rhs) {
        data = _mm_mul_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator/=(const TF128& rhs) {
        data = _mm_div_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator&=(const TF128& rhs) {
        data = _mm_and_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator|=(const TF128& rhs) {
        data = _mm_or_ps(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator^=(const TF128& rhs) {
        data = _mm_xor_ps(data, rhs.data);
        return *this;
    }

    static constexpr std::size_t size() {
        return sizeof(__m128) / sizeof(float);
    }

    static constexpr std::size_t alignment() {
        return alignof(__m128);
    }
};

// Backwards-compatible alias and explicit Fast-arm alias.
using F128 = TF128<FPModel::Precise>;
using F128Fast = TF128<FPModel::Fast>;

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
TRIMD_TARGET_F16C inline TF128<Mode> TF128<Mode>::fromAlignedSource(const std::uint16_t* source) {
    return TF128<Mode>{_mm_cvtph_ps(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(source)))};
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline TF128<Mode> TF128<Mode>::fromUnalignedSource(const std::uint16_t* source) {
    return TF128<Mode>{_mm_cvtph_ps(_mm_loadu_si64(reinterpret_cast<const __m128i*>(source)))};
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline TF128<Mode> TF128<Mode>::loadSingleValue(const std::uint16_t* source) {
    return TF128<Mode>{_mm_cvtph_ps(_mm_loadu_si16(source))};
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF128<Mode>::alignedLoad(const std::uint16_t* source) {
    data = _mm_cvtph_ps(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(source)));
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF128<Mode>::unalignedLoad(const std::uint16_t* source) {
    data = _mm_cvtph_ps(_mm_loadu_si64(reinterpret_cast<const __m128i*>(source)));
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF128<Mode>::alignedStore(std::uint16_t* dest) const {
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wold-style-cast"
            #if !defined(__clang__)
                #pragma GCC diagnostic ignored "-Wuseless-cast"
            #endif
        #endif
    _mm_storel_epi64(reinterpret_cast<__m128i*>(dest), _mm_cvtps_ph(data, _MM_FROUND_CUR_DIRECTION));
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic pop
        #endif
}

template<FPModel Mode>
TRIMD_TARGET_F16C inline void TF128<Mode>::unalignedStore(std::uint16_t* dest) const {
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wold-style-cast"
            #if !defined(__clang__)
                #pragma GCC diagnostic ignored "-Wuseless-cast"
            #endif
        #endif
    _mm_storeu_si64(reinterpret_cast<__m128i*>(dest), _mm_cvtps_ph(data, _MM_FROUND_CUR_DIRECTION));
        #if defined(__clang__) || defined(__GNUC__)
            #pragma GCC diagnostic pop
        #endif
}

    #endif  // TRIMD_ENABLE_F16C

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator==(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_cmpeq_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator!=(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_cmpneq_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator<(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_cmplt_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator<=(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_cmple_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator>(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_cmpgt_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator>=(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_cmpge_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator+(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) += rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator-(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) -= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator*(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) *= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator/(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) /= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator&(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) &= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator|(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) |= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator^(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>(lhs) ^= rhs;
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator~(const TF128<Mode>& rhs) {
    static const auto ones = static_cast<int>(0xFFFFFFFF);
    TF128<Mode> result{_mm_castsi128_ps(_mm_set1_epi32(ones))};
    return (result ^= rhs);
}

template<FPModel Mode>
FORCE_INLINE void transpose(TF128<Mode>& row0, TF128<Mode>& row1, TF128<Mode>& row2, TF128<Mode>& row3) {
    _MM_TRANSPOSE4_PS(row0.data, row1.data, row2.data, row3.data);
}

    // See avx::fma for the FMA-availability contract. Plain `inline` (not
    // FORCE_INLINE) for the same reason as the F16C block above: TRIMD_TARGET_FMA
    // would clash with always_inline when the consumer TU lacks `-mfma`.
    #ifdef TRIMD_ENABLE_FMA
template<FPModel Mode>
TRIMD_TARGET_FMA inline TF128<Mode> fma(const TF128<Mode>& a, const TF128<Mode>& b, const TF128<Mode>& c) {
    return TF128<Mode>{_mm_fmadd_ps(a.data, b.data, c.data)};
}
    #else
template<FPModel Mode>
FORCE_INLINE TF128<Mode> fma(const TF128<Mode>& a, const TF128<Mode>& b, const TF128<Mode>& c) {
    return TF128<Mode>{_mm_add_ps(_mm_mul_ps(a.data, b.data), c.data)};
}
    #endif  // TRIMD_ENABLE_FMA

template<FPModel Mode>
FORCE_INLINE TF128<Mode> abs(const TF128<Mode>& rhs) {
    const __m128 ABSMASK = _mm_castsi128_ps(_mm_set1_epi32(~(1 << 31)));
    return TF128<Mode>{_mm_and_ps(ABSMASK, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> andnot(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{_mm_andnot_ps(lhs.data, rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> rsqrt(const TF128<Mode>& rhs) {
    #ifndef TRIMD_ENABLE_FAST_INVERSE_SQRT
    return TF128<Mode>{_mm_rsqrt_ps(rhs.data)};
    #else
    const __m128i shifted = _mm_srli_epi32(_mm_castps_si128(rhs.data), 1);
    const __m128i subtracted = _mm_sub_epi32(_mm_set1_epi32(0x5f1ffff9), shifted);
    TF128<Mode> result{_mm_castsi128_ps(subtracted)};
    result *= TF128<Mode>{0.703952253f} * (TF128<Mode>{2.38924456f} - rhs * result * result);
    return result;
    #endif  // TRIMD_ENABLE_FAST_INVERSE_SQRT
}

using F256 = fallback::T256<F128>;
using F256Fast = fallback::T256<F128Fast>;
using fallback::abs;
using fallback::andnot;
using fallback::fma;
using fallback::rsqrt;
using fallback::transpose;

}  // namespace sse

}  // namespace trimd

#endif  // TRIMD_ENABLE_SSE
