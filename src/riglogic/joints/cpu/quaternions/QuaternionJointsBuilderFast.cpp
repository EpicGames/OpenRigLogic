// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef RL_BUILD_WITH_FAST

    #ifndef TRIMD_ENABLE_FAST_FP
        #define TRIMD_ENABLE_FAST_FP
    #endif

    #include "riglogic/system/simd/Utils.h"

    #if !defined(TRIMD_FAST_FP_AVAILABLE)

        #pragma message(                                                                                                         \
            "QuaternionJointsBuilderFast.cpp is not compiled with /fp:fast - the Fast floating point model will fall back to Precise")

        #include "riglogic/joints/cpu/quaternions/QuaternionCalculationStrategy.h"

namespace rl4 {

UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType createFastQuaternionStrategy(const Configuration& /*unused*/,
                                                                                                  tdm::rot_seq /*unused*/,
                                                                                                  const tdm::rot_sign& /*unused*/,
                                                                                                  dna::RotationUnit /*unused*/,
                                                                                                  MemoryResource* /*unused*/) {
    return {};
}

}  // namespace rl4

    #else

TRIMD_FAST_FP_BEGIN

        #include "riglogic/joints/cpu/quaternions/QuaternionJointsStrategyFactory.h"

namespace rl4 {

UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType createFastQuaternionStrategy(
    const Configuration& config,
    tdm::rot_seq rotationSequence,
    const tdm::rot_sign& rotationSigns,
    dna::RotationUnit rotationUnit,
    MemoryResource* memRes) {

    using StrategyPointer = UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType;
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Fast, JointGroupQuaternionStrategyFactory, StrategyPointer>(
        config,
        config.rotationType,
        rotationSequence,
        rotationSigns,
        rotationUnit,
        memRes);
}

}  // namespace rl4

TRIMD_FAST_FP_END

    #endif  // !defined(TRIMD_FAST_FP_AVAILABLE)

#endif  // RL_BUILD_WITH_FAST
