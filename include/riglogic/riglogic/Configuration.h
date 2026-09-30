// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <cstdint>

namespace rl4 {

/**
    @brief Implementation type for RigLogic calculations.
*/
enum class CalculationType : std::uint8_t {
    Scalar = 0,    ///< scalar CPU algorithm
    SSE = 1,       ///< vectorized (SSE) CPU algorithm
    AVX = 2,       ///< vectorized (AVX) CPU algorithm (RigLogic must be built with AVX support,
                   ///< otherwise it falls back to using the Scalar version)
    AVX512F = 5,   ///< vectorized (AVX-512F) CPU algorithm (RigLogic must be built with AVX-512F
                   ///< support, otherwise it falls back to AVX / SSE / Scalar)
    NEON = 3,      ///< vectorized (NEON) CPU algorithm (RigLogic must be built with NEON support,
                   ///< otherwise it falls back to using the Scalar version)
    AnyVector = 4  ///< Pick any available vectorization
};

/**
    @brief Floating point type used in vectorized calculations.
*/
enum class FloatingPointType : std::uint8_t {
    Float,
    HalfFloat
};

/**
    @brief Floating point evaluation model.

    `Precise` (the default) is bit-reproducible across machines. `Fast` opts into relaxed
    FP contraction (FMA fusion where the build's codegen flags allow it); it requires a
    binary built with RL_BUILD_WITH_FAST=ON and otherwise falls back to `Precise`. The
    model is orthogonal to the calculation type - Fast uses the same instruction set and
    needs no additional CPU capability. Results may differ at rounding-error scale.
*/
enum class FloatingPointModel : std::uint8_t {
    Precise,
    Fast
};

/**
    @brief Translation type to be used by RigLogic.
*/
enum class TranslationType : std::uint8_t {
    Vector = 3,
};

/**
    @brief Rotation type to be used by RigLogic.
*/
enum class RotationType : std::uint8_t {
    EulerAngles = 3,
    Quaternions = 4
};

/**
    @brief Scale type to be used by RigLogic.
*/
enum class ScaleType : std::uint8_t {
    Vector = 3,
};

struct Configuration {
    CalculationType calculationType = CalculationType::AnyVector;
    FloatingPointType floatingPointType = FloatingPointType::HalfFloat;
    FloatingPointModel floatingPointModel = FloatingPointModel::Precise;
    bool loadJoints = true;
    bool loadBlendShapes = true;
    bool loadAnimatedMaps = true;
    bool loadMachineLearnedBehavior = true;
    bool loadRBFBehavior = true;
    bool loadTwistSwingBehavior = true;
    TranslationType translationType = TranslationType::Vector;
    RotationType rotationType = RotationType::EulerAngles;
    ScaleType scaleType = ScaleType::Vector;
    float translationPruningThreshold = 0.0f;  // Reasonably safe to try 0.0001f;
    float rotationPruningThreshold = 0.0f;     // Reasonably safe to try 0.1f
    float scalePruningThreshold = 0.0f;        // Reasonably safe to try 0.001f;

    /**
        @brief Check that every enum field holds a valid enumerator of its type.
        @note Deserialization or out-of-range casts can produce values outside the defined enumerators,
            which downstream select calculation strategies and must therefore be rejected up front.
    */
    bool validate() const {
        switch (calculationType) {
        case CalculationType::Scalar:
        case CalculationType::SSE:
        case CalculationType::AVX:
        case CalculationType::AVX512F:
        case CalculationType::NEON:
        case CalculationType::AnyVector:
            break;
        default:
            return false;
        }
        if ((floatingPointType != FloatingPointType::Float) && (floatingPointType != FloatingPointType::HalfFloat)) {
            return false;
        }
        if ((floatingPointModel != FloatingPointModel::Precise) && (floatingPointModel != FloatingPointModel::Fast)) {
            return false;
        }
        if (translationType != TranslationType::Vector) {
            return false;
        }
        if ((rotationType != RotationType::EulerAngles) && (rotationType != RotationType::Quaternions)) {
            return false;
        }
        return scaleType == ScaleType::Vector;
    }
};

}  // namespace rl4
