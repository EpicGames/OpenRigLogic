// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/system/simd/SIMD.h"
#include "riglogic/utils/Macros.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstdint>
#include <utility>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

// rl4::FloatingPointModel (public) and trimd::FPModel are independent enums that map 1:1
// numerically; these asserts pin the static_cast between them.
static_assert(static_cast<std::uint8_t>(FloatingPointModel::Precise) == static_cast<std::uint8_t>(trimd::FPModel::Precise),
              "rl4::FloatingPointModel::Precise must map 1:1 to trimd::FPModel::Precise");
static_assert(static_cast<std::uint8_t>(FloatingPointModel::Fast) == static_cast<std::uint8_t>(trimd::FPModel::Fast),
              "rl4::FloatingPointModel::Fast must map 1:1 to trimd::FPModel::Fast");

struct ActiveFeatures {
    CalculationType calculationType;
    FloatingPointType floatingPointType;
    FloatingPointModel floatingPointModel;
};

inline bool operator==(const ActiveFeatures& lhs, const ActiveFeatures& rhs) {
    return (lhs.calculationType == rhs.calculationType) && (lhs.floatingPointType == rhs.floatingPointType) &&
           (lhs.floatingPointModel == rhs.floatingPointModel);
}

inline bool operator!=(const ActiveFeatures& lhs, const ActiveFeatures& rhs) {
    return !(lhs == rhs);
}

inline ActiveFeatures getActiveFeatures(const Configuration& config) {
    ActiveFeatures result = {};

    auto features = trimd::getCPUFeatures();
    RL_UNUSED(features);
    RL_UNUSED(config);

#ifdef RL_BUILD_WITH_AVX512F
    #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
    features.AVX512F = true;
    #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
    if (features.AVX512F &&
        ((config.calculationType == CalculationType::AVX512F) || (config.calculationType == CalculationType::AnyVector))) {
        result.calculationType = CalculationType::AVX512F;
        result.floatingPointType = FloatingPointType::Float;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
        #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
        features.F16C = true;
        #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
        if (features.F16C && (config.floatingPointType == FloatingPointType::HalfFloat)) {
            result.floatingPointType = FloatingPointType::HalfFloat;
        }
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #ifdef RL_BUILD_WITH_FAST
        if (config.floatingPointModel == FloatingPointModel::Fast) {
            result.floatingPointModel = FloatingPointModel::Fast;
        }
    #endif  // RL_BUILD_WITH_FAST
        return result;
    }
#endif  // RL_BUILD_WITH_AVX512F

#ifdef RL_BUILD_WITH_AVX
    #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
    features.AVX = true;
    #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
    if (features.AVX &&
        ((config.calculationType == CalculationType::AVX) || (config.calculationType == CalculationType::AnyVector))) {
        result.calculationType = CalculationType::AVX;
        result.floatingPointType = FloatingPointType::Float;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
        #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
        features.F16C = true;
        #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
        if (features.F16C && (config.floatingPointType == FloatingPointType::HalfFloat)) {
            result.floatingPointType = FloatingPointType::HalfFloat;
        }
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #ifdef RL_BUILD_WITH_FAST
        if (config.floatingPointModel == FloatingPointModel::Fast) {
            result.floatingPointModel = FloatingPointModel::Fast;
        }
    #endif  // RL_BUILD_WITH_FAST
        return result;
    }
#endif  // RL_BUILD_WITH_AVX

#ifdef RL_BUILD_WITH_SSE
    #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
    features.SSE2 = true;
    #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
    if (features.SSE2 &&
        ((config.calculationType == CalculationType::SSE) || (config.calculationType == CalculationType::AnyVector))) {
        result.calculationType = CalculationType::SSE;
        result.floatingPointType = FloatingPointType::Float;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
        #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
        features.F16C = true;
        #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
        if (features.F16C && (config.floatingPointType == FloatingPointType::HalfFloat)) {
            result.floatingPointType = FloatingPointType::HalfFloat;
        }
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #ifdef RL_BUILD_WITH_FAST
        if (config.floatingPointModel == FloatingPointModel::Fast) {
            result.floatingPointModel = FloatingPointModel::Fast;
        }
    #endif  // RL_BUILD_WITH_FAST
        return result;
    }
#endif  // RL_BUILD_WITH_SSE

#ifdef RL_BUILD_WITH_NEON
    #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
    features.NEON = true;
    #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
    if (features.NEON &&
        ((config.calculationType == CalculationType::NEON) || (config.calculationType == CalculationType::AnyVector))) {
        result.calculationType = CalculationType::NEON;
        result.floatingPointType = FloatingPointType::Float;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
        #ifdef RL_DISABLE_RUNTIME_FEATURE_DETECTION
        features.FP16 = true;
        #endif  // RL_DISABLE_RUNTIME_FEATURE_DETECTION
        if (features.FP16 && (config.floatingPointType == FloatingPointType::HalfFloat)) {
            result.floatingPointType = FloatingPointType::HalfFloat;
        }
    #endif  // RL_BUILD_WITH_HALF_FLOATS
    #ifdef RL_BUILD_WITH_FAST
        if (config.floatingPointModel == FloatingPointModel::Fast) {
            result.floatingPointModel = FloatingPointModel::Fast;
        }
    #endif  // RL_BUILD_WITH_FAST
        return result;
    }
#endif  // RL_BUILD_WITH_NEON

    result.calculationType = CalculationType::Scalar;
    result.floatingPointType = FloatingPointType::Float;
#ifdef RL_BUILD_WITH_FAST
    if (config.floatingPointModel == FloatingPointModel::Fast) {
        result.floatingPointModel = FloatingPointModel::Fast;
    }
#endif  // RL_BUILD_WITH_FAST
    return result;
}

struct RuntimeTemplateInstantiator {

    // M only mangles symbols so both FP arms coexist in one binary; for that to hold, invoke<Fast> must
    // only be instantiated from TUs compiled with relaxed-FP flags.
    template<FloatingPointModel M, template<class...> class TClass, class TReturnType, typename... Args>
    static TReturnType invoke(const ActiveFeatures& features, Args&&... args) {
        static constexpr trimd::FPModel TM = static_cast<trimd::FPModel>(M);
        RL_UNUSED(features);

#ifdef RL_BUILD_WITH_SSE
        if (features.calculationType == CalculationType::SSE) {
            using TF128 = trimd::sse::TF128<TM>;
            using TF256 = trimd::fallback::T256<TF128>;
            using TF512 = trimd::fallback::T512<TF256>;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
            if (features.floatingPointType == FloatingPointType::HalfFloat) {
                return TClass<std::uint16_t, TF512, TF256, TF128>()(std::forward<Args>(args)...);
            }
    #endif  // RL_BUILD_WITH_HALF_FLOATS

            return TClass<float, TF512, TF256, TF128>()(std::forward<Args>(args)...);
        }
#endif  // RL_BUILD_WITH_SSE

#ifdef RL_BUILD_WITH_AVX512F
        if (features.calculationType == CalculationType::AVX512F) {
            using TF128 = trimd::sse::TF128<TM>;
            using TF256 = trimd::avx::TF256<TM>;
            using TF512 = trimd::avx512::TF512<TM>;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
            if (features.floatingPointType == FloatingPointType::HalfFloat) {
                return TClass<std::uint16_t, TF512, TF256, TF128>()(std::forward<Args>(args)...);
            }
    #endif  // RL_BUILD_WITH_HALF_FLOATS

            return TClass<float, TF512, TF256, TF128>()(std::forward<Args>(args)...);
        }
#endif  // RL_BUILD_WITH_AVX512F

#ifdef RL_BUILD_WITH_AVX
        if (features.calculationType == CalculationType::AVX) {
            using TF128 = trimd::sse::TF128<TM>;
            using TF256 = trimd::avx::TF256<TM>;
            using TF512 = trimd::fallback::T512<TF256>;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
            if (features.floatingPointType == FloatingPointType::HalfFloat) {
                return TClass<std::uint16_t, TF512, TF256, TF128>()(std::forward<Args>(args)...);
            }
    #endif  // RL_BUILD_WITH_HALF_FLOATS

            return TClass<float, TF512, TF256, TF128>()(std::forward<Args>(args)...);
        }
#endif  // RL_BUILD_WITH_AVX

#ifdef RL_BUILD_WITH_NEON
        if (features.calculationType == CalculationType::NEON) {
            using TF128 = trimd::neon::TF128<TM>;
            using TF256 = trimd::fallback::T256<TF128>;
            using TF512 = trimd::fallback::T512<TF256>;
    #ifdef RL_BUILD_WITH_HALF_FLOATS
            if (features.floatingPointType == FloatingPointType::HalfFloat) {
                return TClass<std::uint16_t, TF512, TF256, TF128>()(std::forward<Args>(args)...);
            }
    #endif  // RL_BUILD_WITH_HALF_FLOATS

            return TClass<float, TF512, TF256, TF128>()(std::forward<Args>(args)...);
        }
#endif  // RL_BUILD_WITH_NEON

        {
            using TF128 = trimd::scalar::T128<float, TM>;
            using TF256 = trimd::fallback::T256<TF128>;
            using TF512 = trimd::fallback::T512<TF256>;
            return TClass<float, TF512, TF256, TF128>()(std::forward<Args>(args)...);
        }
    }

    // Sites dispatching more than once must resolve getActiveFeatures once and use the overload above,
    // so every decision is made from the same snapshot.
    template<FloatingPointModel M, template<class...> class TClass, class TReturnType, typename... Args>
    static TReturnType invoke(const Configuration& config, Args&&... args) {
        return invoke<M, TClass, TReturnType>(getActiveFeatures(config), std::forward<Args>(args)...);
    }
};

}  // namespace rl4
