// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/bpcm/BPCMCalculationStrategy.h"
#include "riglogic/joints/cpu/bpcm/RotationAdapters.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/system/simd/Utils.h"

namespace rl4 {

namespace bpcm {

template<typename T, typename TF512, typename TF256, typename TF128>
struct JointGroupLinearStrategyFactory {
    using BasePointer = UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType;

    BasePointer operator()(RotationType rotationType,
                           tdm::rot_seq rotationSequence,
                           const tdm::rot_sign& rotationSigns,
                           dna::RotationUnit rotationUnit,
                           MemoryResource* memRes) {

        if (rotationType == RotationType::EulerAngles) {
            using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, NoopAdapter>;
            return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                NoopAdapter{rotationSigns});
        }

#ifdef RL_BUILD_WITH_XYZ_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::xyz) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            } else {
                using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::xyz>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER

#ifdef RL_BUILD_WITH_XZY_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::xzy) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xzy>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            } else {
                using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::xzy>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_XZY_ROTATION_ORDER

#ifdef RL_BUILD_WITH_YXZ_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::yxz) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::yxz>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            } else {
                using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::yxz>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_YXZ_ROTATION_ORDER

#ifdef RL_BUILD_WITH_YZX_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::yzx) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::yzx>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            } else {
                using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::yzx>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_YZX_ROTATION_ORDER

#ifdef RL_BUILD_WITH_ZXY_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::zxy) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::zxy>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            } else {
                using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::zxy>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_ZXY_ROTATION_ORDER

#ifdef RL_BUILD_WITH_ZYX_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::zyx) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::zyx>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            } else {
                using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::zyx>;
                using CalculationStrategy = MultiWidthJointGroupLinearCalculationStrategy<T, TF512, TF256, TF128, E2Q>;
                return UniqueInstance<CalculationStrategy, JointGroupLinearCalculationStrategy>::with(memRes).create(
                    E2Q{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_ZYX_ROTATION_ORDER

        return nullptr;
    }
};

// Each arm lives in its own TU pinning its FP contraction mode; BPCMJointsBuilderFast.cpp is the only TU allowed to
// instantiate Fast-tagged kernels.
UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType createPreciseLinearStrategy(const Configuration& config,
                                                                                             tdm::rot_seq rotationSequence,
                                                                                             const tdm::rot_sign& rotationSigns,
                                                                                             dna::RotationUnit rotationUnit,
                                                                                             MemoryResource* memRes);

#ifdef RL_BUILD_WITH_FAST
UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType createFastLinearStrategy(const Configuration& config,
                                                                                          tdm::rot_seq rotationSequence,
                                                                                          const tdm::rot_sign& rotationSigns,
                                                                                          dna::RotationUnit rotationUnit,
                                                                                          MemoryResource* memRes);
#endif  // RL_BUILD_WITH_FAST

}  // namespace bpcm

}  // namespace rl4
