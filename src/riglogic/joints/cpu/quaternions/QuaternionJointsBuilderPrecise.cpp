// Copyright Epic Games, Inc. All Rights Reserved.

// TRIMD_ENABLE_FAST_FP only unlocks TRiMD's scoped FP pragma machinery; it does not itself relax FP semantics
#ifndef TRIMD_ENABLE_FAST_FP
    #define TRIMD_ENABLE_FAST_FP
#endif

#include "riglogic/system/simd/Macros.h"

TRIMD_PRECISE_FP_BEGIN

#include "riglogic/joints/cpu/quaternions/QuaternionJointsStrategyFactory.h"
#include "riglogic/system/simd/Utils.h"

namespace rl4 {

UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType createPreciseQuaternionStrategy(
    const Configuration& config,
    tdm::rot_seq rotationSequence,
    const tdm::rot_sign& rotationSigns,
    dna::RotationUnit rotationUnit,
    MemoryResource* memRes) {

    using StrategyPointer = UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType;
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, JointGroupQuaternionStrategyFactory, StrategyPointer>(
        config,
        config.rotationType,
        rotationSequence,
        rotationSigns,
        rotationUnit,
        memRes);
}

}  // namespace rl4

TRIMD_PRECISE_FP_END
