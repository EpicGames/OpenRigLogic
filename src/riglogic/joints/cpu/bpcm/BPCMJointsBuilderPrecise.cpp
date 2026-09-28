// Copyright Epic Games, Inc. All Rights Reserved.

// FPModel::Precise instantiations of the BPCM strategies. Strict FP is pinned with TRiMD's scoped pragmas so this
// arm stays bit-reproducible under any host build (GCC defaults to -ffp-contract=fast); the pragmas must be in
// effect while the kernel templates are parsed, hence the include split.

// TRIMD_ENABLE_FAST_FP only unlocks TRiMD's scoped FP pragma machinery (strict and
// relaxed); it does not itself relax FP semantics
#ifndef TRIMD_ENABLE_FAST_FP
    #define TRIMD_ENABLE_FAST_FP
#endif

#include "riglogic/system/simd/Macros.h"

TRIMD_PRECISE_FP_BEGIN

#include "riglogic/joints/cpu/bpcm/BPCMJointsStrategyFactory.h"
#include "riglogic/system/simd/Utils.h"

namespace rl4 {

namespace bpcm {

UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType createPreciseLinearStrategy(const Configuration& config,
                                                                                             tdm::rot_seq rotationSequence,
                                                                                             const tdm::rot_sign& rotationSigns,
                                                                                             dna::RotationUnit rotationUnit,
                                                                                             MemoryResource* memRes) {

    using StrategyPointer = UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType;
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, JointGroupLinearStrategyFactory, StrategyPointer>(
        config,
        config.rotationType,
        rotationSequence,
        rotationSigns,
        rotationUnit,
        memRes);
}

}  // namespace bpcm

}  // namespace rl4

TRIMD_PRECISE_FP_END
