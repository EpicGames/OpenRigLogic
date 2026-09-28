// Copyright Epic Games, Inc. All Rights Reserved.

// TRIMD_ENABLE_FAST_FP only unlocks TRiMD's scoped FP pragma machinery (strict and
// relaxed); it does not itself relax FP semantics
#ifndef TRIMD_ENABLE_FAST_FP
    #define TRIMD_ENABLE_FAST_FP
#endif

#include "riglogic/system/simd/Macros.h"

TRIMD_PRECISE_FP_BEGIN

#include "riglogic/ml/cpu/Operation.h"
#include "riglogic/system/simd/Utils.h"

namespace rl4 {

namespace ml {

namespace cpu {

UniqueInstance<OperationSet>::PointerType createPreciseOperationSet(const Configuration& config,
                                                                    OperationSetData&& data,
                                                                    MemoryResource* memRes) {
    using BasePointer = UniqueInstance<OperationSet>::PointerType;
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, OperationSetFactory, BasePointer>(config,
                                                                                                              std::move(data),
                                                                                                              memRes);
}

}  // namespace cpu

}  // namespace ml

}  // namespace rl4

TRIMD_PRECISE_FP_END
