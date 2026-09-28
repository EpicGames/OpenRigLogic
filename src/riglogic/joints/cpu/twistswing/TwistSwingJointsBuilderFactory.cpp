// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/joints/cpu/twistswing/TwistSwingJointsBuilderFactory.h"

#include "riglogic/joints/cpu/twistswing/TwistSwingJointsBuilder.h"

namespace rl4 {

UniqueInstance<JointsBuilder>::PointerType TwistSwingJointsBuilderFactory::create(const Configuration& config,
                                                                                  RigMetadata* meta,
                                                                                  MemoryResource* memRes) {
    // Only a scalar implementation exists for twist and swing evaluation.
    using ScalarTwistSwingJointsBuilder = TwistSwingJointsBuilder<float, trimd::scalar::F256, trimd::scalar::F128>;
    return UniqueInstance<ScalarTwistSwingJointsBuilder, JointsBuilder>::with(memRes).create(config, meta, memRes);
}

}  // namespace rl4
