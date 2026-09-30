// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/rbf/cpu/AdditiveRBFSolver.h"

namespace rl4 {

AdditiveRBFSolver::AdditiveRBFSolver(MemoryResource* memRes) :
    RBFSolver(memRes) {
}

AdditiveRBFSolver::AdditiveRBFSolver(const RBFSolverRecipe& recipe, MemoryResource* memRes) :
    RBFSolver(recipe, memRes) {
}

RBFSolverType AdditiveRBFSolver::getSolverType() const {
    return RBFSolverType::Additive;
}

void AdditiveRBFSolver::solve(ArrayView<float> input, ArrayView<float> /*unused*/, ArrayView<float> outputWeights) const {
    convertInput(input);
    getDistanceWeight(getDistanceTargets(), input, outputWeights, radius);

    normalizeAndCutOff(outputWeights);
}

}  // namespace rl4
