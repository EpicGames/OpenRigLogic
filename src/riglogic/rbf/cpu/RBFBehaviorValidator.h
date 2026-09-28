// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/rbf/cpu/RBFSolver.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/types/LODSpec.h"

#include <cstddef>
#include <cstdint>

namespace rl4 {

namespace rbf {

namespace cpu {

// Shared by the DNA factory path and the snapshot load() path; bounds every deserialized index calculate() dereferences.
// Bounds are metadata-authoritative for the control buffer (getControlInputCount()) and structural for everything else.
struct RBFBehaviorValidator {
    static bool validate(const LODSpec<std::uint16_t>& lods,
                         const Vector<RBFSolver::Pointer>& solvers,
                         const Matrix<std::uint16_t>& solverRawControlInputIndices,
                         const Matrix<std::uint16_t>& solverRawControlOutputIndices,
                         const Matrix<std::uint16_t>& solverPoseIndices,
                         const Matrix<std::uint16_t>& poseInputControlIndices,
                         const Matrix<std::uint16_t>& poseOutputControlIndices,
                         const Matrix<float>& poseOutputControlWeights,
                         const Vector<std::uint16_t>& inputCountPerSolver,
                         const Vector<std::uint16_t>& targetCountPerSolver,
                         const RigMetadata& metadata) {
        const std::size_t controlInputCount = metadata.getControlInputCount();
        const std::size_t solverCount = solvers.size();

        // Every per-solver array is indexed by solverIndex, so they must all be at least as long as solvers[].
        if ((solverRawControlInputIndices.size() < solverCount) || (solverRawControlOutputIndices.size() < solverCount) ||
            (solverPoseIndices.size() < solverCount) || (inputCountPerSolver.size() < solverCount) ||
            (targetCountPerSolver.size() < solverCount)) {
            return false;
        }
        // The output-instance ctor walks targetCountPerSolver.size() while reading inputCountPerSolver[solverIndex] in
        // lockstep, so the two must be equal, not merely floored.
        if (inputCountPerSolver.size() != targetCountPerSolver.size()) {
            return false;
        }

        // The three pose matrices are indexed by the same pose index and must line up.
        const std::size_t poseCount = poseInputControlIndices.size();
        if ((poseOutputControlIndices.size() != poseCount) || (poseOutputControlWeights.size() != poseCount)) {
            return false;
        }

        // lodCount is deserialized independently of indicesPerLOD, so a ceiling alone would leave indicesPerLOD[lod] out
        // of bounds for the higher LODs; require exactly lodCount rows.
        if (lods.indicesPerLOD.size() != static_cast<std::size_t>(metadata.lodCount)) {
            return false;
        }
        for (const auto& solverIndicesForLOD : lods.indicesPerLOD) {
            for (const auto solverIndex : solverIndicesForLOD) {
                if (static_cast<std::size_t>(solverIndex) >= solverCount) {
                    return false;
                }
            }
        }

        // Each pose's output control indices run parallel to its output control weights.
        for (std::size_t p = {}; p < poseCount; ++p) {
            if (poseOutputControlIndices[p].size() != poseOutputControlWeights[p].size()) {
                return false;
            }
            for (const auto inputControlIndex : poseInputControlIndices[p]) {
                if (static_cast<std::size_t>(inputControlIndex) >= controlInputCount) {
                    return false;
                }
            }
            for (const auto outputControlIndex : poseOutputControlIndices[p]) {
                if (static_cast<std::size_t>(outputControlIndex) >= controlInputCount) {
                    return false;
                }
            }
        }

        for (std::size_t s = {}; s < solverCount; ++s) {
            const RBFSolver& solver = *solvers[s];
            // Enums select function pointers; an out-of-range weight function yields a null getDistanceWeight with no
            // safe switch default.
            if (!isValidWeightFunction(solver.getWeightFunction())) {
                return false;
            }
            if (!isValidDistanceMethod(solver.getDistanceMethod())) {
                return false;
            }

            const std::size_t inputCount = static_cast<std::size_t>(inputCountPerSolver[s]);
            const std::size_t targetCount = static_cast<std::size_t>(targetCountPerSolver[s]);

            // The gather loop copies rawControlInputCount values into the per-solver input buffer.
            const auto& rawControlInputIndices = solverRawControlInputIndices[s];
            if (rawControlInputIndices.size() > inputCount) {
                return false;
            }
            for (const auto inputIndex : rawControlInputIndices) {
                if (static_cast<std::size_t>(inputIndex) >= controlInputCount) {
                    return false;
                }
            }
            for (const auto outputIndex : solverRawControlOutputIndices[s]) {
                if (static_cast<std::size_t>(outputIndex) >= controlInputCount) {
                    return false;
                }
            }

            // The target set is deserialized independently of targetCountPerSolver[s]: solve() writes ti < targets.size()
            // into buffers sized targetCount, and normalizeAndCutOff reads targetScale up to that width.
            const auto solverTargets = solver.getTargets();
            if (solverTargets.size() > targetCount) {
                return false;
            }
            if (solver.getTargetScales().size() < targetCount) {
                return false;
            }
            // getDistance reads target[i]/input[i] up to the input width; each target must be at least that wide.
            for (const auto& target : solverTargets) {
                if (target.size() < inputCount) {
                    return false;
                }
            }
            // Quaternion-family paths stride the input in blocks of four; any other width over-reads the final subview.
            if (usesQuaternionInput(solver.getDistanceMethod()) && ((inputCount % 4u) != 0u)) {
                return false;
            }

            // Pose indices subscript the pose matrices; their count drives reads of outputWeightsBuffer, sized targetCount.
            const auto& poseIndices = solverPoseIndices[s];
            if (poseIndices.size() > targetCount) {
                return false;
            }
            for (const auto poseIndex : poseIndices) {
                if (static_cast<std::size_t>(poseIndex) >= poseCount) {
                    return false;
                }
            }
        }

        return true;
    }

private:
    static bool isValidWeightFunction(RBFFunctionType weightFunction) {
        switch (weightFunction) {
        case RBFFunctionType::Gaussian:
        case RBFFunctionType::Exponential:
        case RBFFunctionType::Linear:
        case RBFFunctionType::Cubic:
        case RBFFunctionType::Quintic:
            return true;
        default:
            return false;
        }
    }

    static bool isValidDistanceMethod(RBFDistanceMethod distanceMethod) {
        switch (distanceMethod) {
        case RBFDistanceMethod::Euclidean:
        case RBFDistanceMethod::Quaternion:
        case RBFDistanceMethod::SwingAngle:
        case RBFDistanceMethod::TwistAngle:
            return true;
        default:
            return false;
        }
    }

    // Distance/convert paths that read the input in blocks of four floats (one quaternion per stride).
    static bool usesQuaternionInput(RBFDistanceMethod distanceMethod) {
        return (distanceMethod == RBFDistanceMethod::Quaternion) || (distanceMethod == RBFDistanceMethod::SwingAngle) ||
               (distanceMethod == RBFDistanceMethod::TwistAngle);
    }
};

}  // namespace cpu

}  // namespace rbf

}  // namespace rl4
