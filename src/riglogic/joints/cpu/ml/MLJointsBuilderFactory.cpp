// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/joints/cpu/ml/MLJointsBuilderFactory.h"

#include "riglogic/joints/cpu/ml/MLJointsBuilder.h"
#include "riglogic/system/simd/SIMD.h"

namespace rl4 {

UniqueInstance<JointsBuilder>::PointerType MLJointsBuilderFactory::create(const Configuration& config,
                                                                          RigMetadata* meta,
                                                                          MemoryResource* memRes) {
    return UniqueInstance<ml::MLJointsBuilder, JointsBuilder>::with(memRes).create(config, meta, memRes);
}

}  // namespace rl4
