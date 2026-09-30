// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/quaternions/QuaternionCalculationStrategy.h"
#include "riglogic/joints/cpu/quaternions/RotationAdapters.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/system/simd/Utils.h"

namespace rl4 {

template<typename T, typename TF512, typename TF256, typename TF128>
struct JointGroupQuaternionStrategyFactory {
    using BasePointer = UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType;

    BasePointer operator()(RotationType rotationType,
                           tdm::rot_seq rotationSequence,
                           tdm::rot_sign rotationSigns,
                           dna::RotationUnit rotationUnit,
                           MemoryResource* memRes) {

        if (rotationType == RotationType::Quaternions) {
            using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, PassthroughAdapter>;
            return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                PassthroughAdapter{rotationSigns});
        }

#ifdef RL_BUILD_WITH_XYZ_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::xyz) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::xyz>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            } else {
                using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::xyz>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER

#ifdef RL_BUILD_WITH_XZY_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::xzy) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::xzy>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            } else {
                using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::xzy>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_XZY_ROTATION_ORDER

#ifdef RL_BUILD_WITH_YXZ_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::yxz) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::yxz>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            } else {
                using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::yxz>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_YXZ_ROTATION_ORDER

#ifdef RL_BUILD_WITH_YZX_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::yzx) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::yzx>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            } else {
                using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::yzx>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_YZX_ROTATION_ORDER

#ifdef RL_BUILD_WITH_ZXY_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::zxy) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::zxy>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            } else {
                using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::zxy>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_ZXY_ROTATION_ORDER

#ifdef RL_BUILD_WITH_ZYX_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::zyx) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::zyx>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            } else {
                using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::zyx>;
                using CalculationStrategy = VectorizedJointGroupQuaternionCalculationStrategy<T, TF256, TF128, Q2E>;
                return UniqueInstance<CalculationStrategy, JointGroupQuaternionCalculationStrategy>::with(memRes).create(
                    Q2E{rotationSigns});
            }
        }
#endif  // RL_BUILD_WITH_ZYX_ROTATION_ORDER

        return nullptr;
    }
};

UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType createPreciseQuaternionStrategy(
    const Configuration& config,
    tdm::rot_seq rotationSequence,
    const tdm::rot_sign& rotationSigns,
    dna::RotationUnit rotationUnit,
    MemoryResource* memRes);

#ifdef RL_BUILD_WITH_FAST
UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType createFastQuaternionStrategy(
    const Configuration& config,
    tdm::rot_seq rotationSequence,
    const tdm::rot_sign& rotationSigns,
    dna::RotationUnit rotationUnit,
    MemoryResource* memRes);
#endif  // RL_BUILD_WITH_FAST

}  // namespace rl4
