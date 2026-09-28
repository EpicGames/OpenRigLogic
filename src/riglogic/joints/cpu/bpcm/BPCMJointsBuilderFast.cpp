// Copyright Epic Games, Inc. All Rights Reserved.

// FPModel::Fast instantiations of the BPCM strategies; the FPModel tags keep these symbols distinct from the
// Precise arm's. On MSVC this TU must be compiled with /fp:fast (pragmas can restrict but not relax contraction);
// without it the Fast arm reports unavailable and callers fall back to Precise.

#ifdef RL_BUILD_WITH_FAST

    #ifndef TRIMD_ENABLE_FAST_FP
        #define TRIMD_ENABLE_FAST_FP
    #endif

    #include "riglogic/system/simd/Utils.h"

    #if !defined(TRIMD_FAST_FP_AVAILABLE)

        #pragma message(                                                                                                         \
            "BPCMJointsBuilderFast.cpp is not compiled with /fp:fast - the Fast floating point model will fall back to Precise")

namespace rl4 {

namespace bpcm {

UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType createFastLinearStrategy(const Configuration& /*unused*/,
                                                                                          tdm::rot_seq /*unused*/,
                                                                                          const tdm::rot_sign& /*unused*/,
                                                                                          dna::RotationUnit /*unused*/,
                                                                                          MemoryResource* /*unused*/) {
    return {};
}

}  // namespace bpcm

}  // namespace rl4

    #else

TRIMD_FAST_FP_BEGIN

        #include "riglogic/joints/cpu/bpcm/BPCMJointsStrategyFactory.h"

namespace rl4 {

namespace bpcm {

UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType createFastLinearStrategy(const Configuration& config,
                                                                                          tdm::rot_seq rotationSequence,
                                                                                          const tdm::rot_sign& rotationSigns,
                                                                                          dna::RotationUnit rotationUnit,
                                                                                          MemoryResource* memRes) {

    using StrategyPointer = UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType;
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Fast, JointGroupLinearStrategyFactory, StrategyPointer>(
        config,
        config.rotationType,
        rotationSequence,
        rotationSigns,
        rotationUnit,
        memRes);
}

}  // namespace bpcm

}  // namespace rl4

TRIMD_FAST_FP_END

    #endif  // !defined(TRIMD_FAST_FP_AVAILABLE)

#endif  // RL_BUILD_WITH_FAST
