// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef RL_BUILD_WITH_FAST

    #ifndef TRIMD_ENABLE_FAST_FP
        #define TRIMD_ENABLE_FAST_FP
    #endif

    #include "riglogic/system/simd/Utils.h"

    #if !defined(TRIMD_FAST_FP_AVAILABLE)

        #pragma message(                                                                                                         \
            "CPUMachineLearnedBehaviorOperationSetFast.cpp is not compiled with /fp:fast - the Fast floating point model will fall back to Precise")

        #include "riglogic/ml/cpu/Operation.h"

namespace rl4 {

namespace ml {

namespace cpu {

UniqueInstance<OperationSet>::PointerType createFastOperationSet(const Configuration& config,
                                                                 OperationSetData&& data,
                                                                 MemoryResource* memRes) {
    return createPreciseOperationSet(config, std::move(data), memRes);
}

}  // namespace cpu

}  // namespace ml

}  // namespace rl4

    #else

TRIMD_FAST_FP_BEGIN

        #include "riglogic/ml/cpu/Operation.h"

namespace rl4 {

namespace ml {

namespace cpu {

UniqueInstance<OperationSet>::PointerType createFastOperationSet(const Configuration& config,
                                                                 OperationSetData&& data,
                                                                 MemoryResource* memRes) {
    using BasePointer = UniqueInstance<OperationSet>::PointerType;
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Fast, OperationSetFactory, BasePointer>(config,
                                                                                                           std::move(data),
                                                                                                           memRes);
}

}  // namespace cpu

}  // namespace ml

}  // namespace rl4

TRIMD_FAST_FP_END

    #endif  // !defined(TRIMD_FAST_FP_AVAILABLE)

#endif  // RL_BUILD_WITH_FAST
