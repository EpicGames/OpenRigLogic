// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#ifdef TRIMD_ENABLE_NEON
    #include "trimd/Fallback.h"
    #include "trimd/Macros.h"
    #include "trimd/Platform.h"
    #include "trimd/Shim.h"

    #include <arm_neon.h>

    #include <cstring>

namespace trimd {

namespace neon {

// See note at the top of trimd/AVX.h. TF128 carries the FPModel tag purely for
// symbol mangling so a single binary can host a strict-FP NEON arm and a
// `-ffp-contract=fast` NEON arm and runtime-dispatch between them. NEON's
// `vfmaq_f32` is part of mandatory AdvSIMD on aarch64; the Fast arm's additional
// gain is reassociation, not FMA itself.
template<FPModel Mode = FPModel::Precise>
struct TF128 {
    using value_type = float;

    float32x4_t data;

    FORCE_INLINE TF128() :
        data{vdupq_n_f32(0)} {
    }

    explicit FORCE_INLINE TF128(float32x4_t value) :
        data{value} {
    }

    explicit FORCE_INLINE TF128(float value) :
        TF128{vdupq_n_f32(value)} {
    }

    FORCE_INLINE TF128(float v1, float v2, float v3, float v4) {
        const float source[] = {v1, v2, v3, v4};
        data = vld1q_f32(source);
    }

    static FORCE_INLINE TF128 fromAlignedSource(const float* source) {
        return TF128{vld1q_f32(source)};
    }

    static FORCE_INLINE TF128 fromUnalignedSource(const float* source) {
        return TF128{vld1q_f32(source)};
    }

    static FORCE_INLINE TF128 loadSingleValue(const float* source) {
        return TF128{vsetq_lane_f32(*source, vdupq_n_f32(0), 0)};
    }

    #ifdef TRIMD_ENABLE_NEON_FP16
    // Plain `inline` (no always_inline) - see note on TRIMD_TARGET_NEON_FP16 below.
    static TF128 fromAlignedSource(const std::uint16_t* source);
    static TF128 fromUnalignedSource(const std::uint16_t* source);
    static TF128 loadSingleValue(const std::uint16_t* source);
    #endif  // TRIMD_ENABLE_NEON_FP16

    template<typename T>
    static FORCE_INLINE void prefetchT0(const T* source) {
        prefetch_t0(source);
    }

    template<typename T>
    static FORCE_INLINE void prefetchT1(const T* source) {
        prefetch_t1(source);
    }

    template<typename T>
    static FORCE_INLINE void prefetchT2(const T* source) {
        prefetch_t2(source);
    }

    template<typename T>
    static FORCE_INLINE void prefetchNTA(const T* source) {
        prefetch_nta(source);
    }

    FORCE_INLINE void alignedLoad(const float* source) {
        data = vld1q_f32(source);
    }

    FORCE_INLINE void unalignedLoad(const float* source) {
        data = vld1q_f32(source);
    }

    FORCE_INLINE void alignedStore(float* dest) const {
        vst1q_f32(dest, data);
    }

    FORCE_INLINE void unalignedStore(float* dest) const {
        vst1q_f32(dest, data);
    }

    #ifdef TRIMD_ENABLE_NEON_FP16
    void alignedLoad(const std::uint16_t* source);
    void unalignedLoad(const std::uint16_t* source);
    void alignedStore(std::uint16_t* dest) const;
    void unalignedStore(std::uint16_t* dest) const;
    #endif  // TRIMD_ENABLE_NEON_FP16

    FORCE_INLINE float sum() const {
        const float32x2_t tmp = vadd_f32(vget_high_f32(data), vget_low_f32(data));
        return vget_lane_f32(vpadd_f32(tmp, tmp), 0);
    }

    FORCE_INLINE TF128& operator+=(const TF128& rhs) {
        data = vaddq_f32(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator-=(const TF128& rhs) {
        data = vsubq_f32(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator*=(const TF128& rhs) {
        data = vmulq_f32(data, rhs.data);
        return *this;
    }

    FORCE_INLINE TF128& operator/=(const TF128& rhs) {
        // reciprocal0 = 1 / rhs (initial estimate)
        const float32x4_t reciprocal0 = vrecpeq_f32(rhs.data);
        // Newton-Raphson step to refine the initial reciprocal estimate.
        // If accuracy is not enough, additional refinement steps may be added (as many as needed)
        // until desired accuracy is reached (just duplicate the below line).
        // reciprocal1 = reciprocal0 * (2.0 - (reciprocal0 * rhs))
        const float32x4_t reciprocal1 = vmulq_f32(reciprocal0, vrecpsq_f32(reciprocal0, rhs.data));
        // data = data * reciprocal1
        data = vmulq_f32(data, reciprocal1);
        return *this;
    }

    FORCE_INLINE TF128& operator&=(const TF128& rhs) {
        data = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(data), vreinterpretq_u32_f32(rhs.data)));
        return *this;
    }

    FORCE_INLINE TF128& operator|=(const TF128& rhs) {
        data = vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(data), vreinterpretq_u32_f32(rhs.data)));
        return *this;
    }

    FORCE_INLINE TF128& operator^=(const TF128& rhs) {
        data = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(data), vreinterpretq_u32_f32(rhs.data)));
        return *this;
    }

    static constexpr std::size_t size() {
        return sizeof(float32x4_t) / sizeof(float);
    }

    static constexpr std::size_t alignment() {
        return alignof(float32x4_t);
    }
};

// Backwards-compatible alias and explicit Fast-arm alias.
using F128 = TF128<FPModel::Precise>;
using F128Fast = TF128<FPModel::Fast>;

    #ifdef TRIMD_ENABLE_NEON_FP16

// Plain `inline` (not FORCE_INLINE) is required: TRIMD_TARGET_NEON_FP16 elevates
// codegen to `+fp16` only for these functions, so when the consumer TU has no
// `+fp16` baseline (the runtime-dispatch case, and the linux-arm CI case where
// TRIMD_ENABLE_NEON_FP16 is set without TRIMD_ENABLE_NEON_COMPILER_FLAGS), GCC
// refuses to inline a target-elevated callee into a caller that lacks the ISA
// and errors on always_inline. Plain `inline` lets the compiler emit an
// out-of-line wrapper with the elevated codegen, which is also what runtime
// dispatch wants: an addressable function symbol the dispatcher can invoke
// conditionally after a CPU feature check.
template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline TF128<Mode> TF128<Mode>::fromAlignedSource(const std::uint16_t* source) {
    return TF128<Mode>{vcvt_f32_f16(vld1_f16(reinterpret_cast<const float16_t*>(source)))};
}

template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline TF128<Mode> TF128<Mode>::fromUnalignedSource(const std::uint16_t* source) {
    return TF128<Mode>{vcvt_f32_f16(vld1_f16(reinterpret_cast<const float16_t*>(source)))};
}

template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline TF128<Mode> TF128<Mode>::loadSingleValue(const std::uint16_t* source) {
    float16_t value;
    std::memcpy(&value, source, sizeof(float16_t));
    return TF128<Mode>{vcvt_f32_f16(vset_lane_f16(value, vdup_n_f16(float16_t{}), 0))};
}

template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline void TF128<Mode>::alignedLoad(const std::uint16_t* source) {
    data = vcvt_f32_f16(vld1_f16(reinterpret_cast<const float16_t*>(source)));
}

template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline void TF128<Mode>::unalignedLoad(const std::uint16_t* source) {
    data = vcvt_f32_f16(vld1_f16(reinterpret_cast<const float16_t*>(source)));
}

template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline void TF128<Mode>::alignedStore(std::uint16_t* dest) const {
    vst1_f16(reinterpret_cast<float16_t*>(dest), vcvt_f16_f32(data));
}

template<FPModel Mode>
TRIMD_TARGET_NEON_FP16 inline void TF128<Mode>::unalignedStore(std::uint16_t* dest) const {
    vst1_f16(reinterpret_cast<float16_t*>(dest), vcvt_f16_f32(data));
}

    #endif  // TRIMD_ENABLE_NEON_FP16

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator==(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vceqq_f32(lhs.data, rhs.data))};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator!=(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(lhs.data, rhs.data)))};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator<(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vcltq_f32(lhs.data, rhs.data))};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator<=(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vcleq_f32(lhs.data, rhs.data))};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator>(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vcgtq_f32(lhs.data, rhs.data))};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> operator>=(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vcgeq_f32(lhs.data, rhs.data))};
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
    return TF128<Mode>{vreinterpretq_f32_u32(vmvnq_u32(vreinterpretq_u32_f32(rhs.data)))};
}

template<FPModel Mode>
FORCE_INLINE void transpose(TF128<Mode>& row0, TF128<Mode>& row1, TF128<Mode>& row2, TF128<Mode>& row3) {
    // row01 = [row0.x, row1.x, row0.z, row1.z], [row0.y, row1.y, row0.w, row1.w]
    float32x4x2_t row01 = vtrnq_f32(row0.data, row1.data);
    // row23 = [row2.x, row3.x, row2.z, row3.z], [row2.y, row3.y, row2.w, row3.w]
    float32x4x2_t row23 = vtrnq_f32(row2.data, row3.data);

    // row0 = row0.x, row1.x, row2.x, row3.x
    row0 = TF128<Mode>{vcombine_f32(vget_low_f32(row01.val[0]), vget_low_f32(row23.val[0]))};
    // row1 = row0.y, row1.y, row2.y, row3.y
    row1 = TF128<Mode>{vcombine_f32(vget_low_f32(row01.val[1]), vget_low_f32(row23.val[1]))};
    // row2 = row0.z, row1.z, row2.z, row3.z
    row2 = TF128<Mode>{vcombine_f32(vget_high_f32(row01.val[0]), vget_high_f32(row23.val[0]))};
    // row3 = row0.w, row1.w, row2.w, row3.w
    row3 = TF128<Mode>{vcombine_f32(vget_high_f32(row01.val[1]), vget_high_f32(row23.val[1]))};
}

    // Fused multiply-add via vfmaq_f32. Gated on TRIMD_ENABLE_FMA so the TU's
    // build wiring decides; on aarch64 the consumer should always enable it
    // (vfmaq_f32 is mandatory there), on ARMv7 only when `-mfpu=neon-vfpv4` /
    // `-mfpu=neon-fp-armv8` is in effect.
    #ifdef TRIMD_ENABLE_FMA
template<FPModel Mode>
FORCE_INLINE TF128<Mode> fma(const TF128<Mode>& a, const TF128<Mode>& b, const TF128<Mode>& c) {
    return TF128<Mode>{vfmaq_f32(c.data, a.data, b.data)};
}
    #else
template<FPModel Mode>
FORCE_INLINE TF128<Mode> fma(const TF128<Mode>& a, const TF128<Mode>& b, const TF128<Mode>& c) {
    return TF128<Mode>{vaddq_f32(vmulq_f32(a.data, b.data), c.data)};
}
    #endif  // TRIMD_ENABLE_FMA

template<FPModel Mode>
FORCE_INLINE TF128<Mode> abs(const TF128<Mode>& rhs) {
    return TF128<Mode>{vabsq_f32(rhs.data)};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> andnot(const TF128<Mode>& lhs, const TF128<Mode>& rhs) {
    return TF128<Mode>{vreinterpretq_f32_u32(vbicq_u32(vreinterpretq_u32_f32(rhs.data), vreinterpretq_u32_f32(lhs.data)))};
}

template<FPModel Mode>
FORCE_INLINE TF128<Mode> rsqrt(const TF128<Mode>& rhs) {
    #ifndef TRIMD_ENABLE_FAST_INVERSE_SQRT
    const float32x4_t reciprocal0 = vrsqrteq_f32(rhs.data);
    const float32x4_t reciprocal1 = vmulq_f32(vrsqrtsq_f32(vmulq_f32(reciprocal0, reciprocal0), rhs.data), reciprocal0);
    return TF128<Mode>{reciprocal1};
    #else
    const uint32x4_t shifted = vshrq_n_u32(vreinterpretq_u32_f32(rhs.data), 1);
    const uint32x4_t subtracted = vsubq_u32(vdupq_n_u32(0x5f1ffff9), shifted);
    TF128<Mode> result{vreinterpretq_f32_u32(subtracted)};
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

}  // namespace neon

}  // namespace trimd

#endif  // TRIMD_ENABLE_NEON
