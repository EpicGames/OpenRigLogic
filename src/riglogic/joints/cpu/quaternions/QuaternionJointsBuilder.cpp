// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/joints/cpu/quaternions/QuaternionJointsBuilder.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/controls/Controls.h"
#include "riglogic/joints/JointBehaviorFilter.h"
#include "riglogic/joints/JointsBuilder.h"
#include "riglogic/joints/JointsNullEvaluator.h"
#include "riglogic/joints/cpu/quaternions/QuaternionCalculationStrategy.h"
#include "riglogic/joints/cpu/quaternions/QuaternionJointsEvaluator.h"
#include "riglogic/joints/cpu/quaternions/QuaternionJointsStrategyFactory.h"
#include "riglogic/joints/cpu/quaternions/RotationAdapters.h"
#include "riglogic/joints/cpu/utils/JointGroupOptimizer.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/Utils.h"
#include "riglogic/types/bpcm/Optimizer.h"
#include "riglogic/utils/Extd.h"

#include <tdm/Quat.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

namespace qjc {

static constexpr std::uint32_t BlockHeight = 32u;
static constexpr std::uint32_t PadTo = 16u;
static constexpr std::uint32_t Stride = 4u;

}  // namespace qjc

template<typename T, typename TF512, typename TF256, typename TF128>
struct MatrixOptimizer {

    void operator()(ConstArrayView<float> src, Extent srcDims, Extent dstDims, FloatArray& dst) {
        dst.resize<T>(dstDims.size());
        using BPCMOptimizer = bpcm::Optimizer<TF256, qjc::BlockHeight, qjc::PadTo, qjc::Stride>;
        BPCMOptimizer::optimize(dst.data<T>(), src.data(), srcDims);
    }
};

QuaternionJointsBuilder::QuaternionJointsBuilder(const Configuration& config_, RigMetadata* meta_, MemoryResource* memRes_) :
    config{config_},
    meta{meta_},
    memRes{memRes_},
    jointGroups{memRes_},
    rotationUnit{} {
}

void QuaternionJointsBuilder::computeStorageRequirements() {
}

void QuaternionJointsBuilder::computeStorageRequirements(const JointBehaviorFilter& /*unused*/) {
}

void QuaternionJointsBuilder::allocateStorage(const JointBehaviorFilter& source) {
    jointGroups.resize(source.getJointGroupCount(), JointGroup{memRes});
    for (auto& group : jointGroups) {
        group.lods.resize(source.getLODCount());
    }
}

void QuaternionJointsBuilder::setInputIndices(JointGroup& group, ConstArrayView<std::uint16_t> inputIndices) {
    group.inputIndices.assign(inputIndices.begin(), inputIndices.end());
}

void QuaternionJointsBuilder::setOutputIndices(JointGroup& group, ConstArrayView<std::uint16_t> outputIndices) {
// Given any rotation indices (rx, ry, rz), return only rx indices for all joints in the group
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wattributes"
#endif
    auto deduplicate = [this](Vector<std::uint16_t>& v) {
        UnorderedSet<std::uint16_t> deduplicator{memRes};
        v.erase(v.rend().base(), std::remove_if(v.rbegin(), v.rend(), [&deduplicator](const std::uint16_t value) {
                                     return !deduplicator.insert(value).second;
                                 }).base());
    };
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
    Vector<std::uint16_t> outputRotationBaseIndices{memRes};
    outputRotationBaseIndices.reserve(outputIndices.size() / 3ul);
    std::transform(outputIndices.begin(),
                   outputIndices.end(),
                   std::back_inserter(outputRotationBaseIndices),
                   [](std::uint16_t outputIndex) { return static_cast<std::uint16_t>((outputIndex / 9) * 9); });
    deduplicate(outputRotationBaseIndices);
    group.outputIndices.reserve(outputRotationBaseIndices.size() * static_cast<std::uint8_t>(RotationType::Quaternions));
    for (const auto baseIndex : outputRotationBaseIndices) {
        // 9-attribute joint rx -> 10-attribute joint qx
        const auto jointIndex = static_cast<std::uint16_t>(baseIndex / 9u);
        const auto remappedBaseIndex = static_cast<std::uint16_t>(jointIndex * 10u);
        group.outputIndices.push_back(static_cast<std::uint16_t>(remappedBaseIndex + 3));
        group.outputIndices.push_back(static_cast<std::uint16_t>(remappedBaseIndex + 4));
        group.outputIndices.push_back(static_cast<std::uint16_t>(remappedBaseIndex + 5));
        group.outputIndices.push_back(static_cast<std::uint16_t>(remappedBaseIndex + 6));
    }
}

void QuaternionJointsBuilder::setValues(JointGroup& group,
                                        ConstArrayView<float> eulers,
                                        ConstArrayView<std::uint16_t> inputIndices,
                                        ConstArrayView<std::uint16_t> outputIndices) {
    std::function<tdm::frad(float)> angConv;
    if (rotationUnit == dna::RotationUnit::degrees) {
        angConv = [](float angle) { return tdm::frad{tdm::fdeg{angle}}; };
    } else {
        angConv = [](float angle) { return tdm::frad{angle}; };
    }

    const std::size_t colCount = inputIndices.size();
    const std::size_t rowCount = std::min(outputIndices.size(), (colCount == 0u) ? std::size_t{} : eulers.size() / colCount);

    // One quaternion slot per unique joint; slotOfJoint maps joint index -> slot so sizing and emission agree.
    const std::size_t quatCount = group.outputIndices.size() / 4u;
    std::size_t maxJointIndex = {};
    for (std::size_t slot = {}; slot < quatCount; ++slot) {
        maxJointIndex = std::max(maxJointIndex, static_cast<std::size_t>(group.outputIndices[slot * 4u] / 10u));
    }
    Vector<std::size_t> slotOfJoint{(quatCount == 0u) ? std::size_t{} : maxJointIndex + 1u, quatCount, memRes};
    for (std::size_t slot = {}; slot < quatCount; ++slot) {
        slotOfJoint[group.outputIndices[slot * 4u] / 10u] = slot;
    }

    Vector<float> quaternions(quatCount * 4u * colCount, {}, memRes);
    Vector<tdm::frad3> angles{quatCount, tdm::frad3{}, memRes};
    for (std::size_t col = {}; col < colCount; ++col) {
        std::fill(angles.begin(), angles.end(), tdm::frad3{});
        for (std::size_t row = {}; row < rowCount; ++row) {
            const std::size_t jointIndex = outputIndices[row] / 9u;
            const std::size_t slot = (jointIndex < slotOfJoint.size()) ? slotOfJoint[jointIndex] : quatCount;
            if (slot == quatCount) {
                continue;  // every row's joint has a slot by construction; defensive only
            }
            const auto relAttrIndex = static_cast<std::uint16_t>(outputIndices[row] % 9u);
            // 0 = rx, 1 = ry, 2 = rz
            const auto relRotAttrIndex = static_cast<std::uint16_t>(relAttrIndex % 3u);
            angles[slot][relRotAttrIndex] = angConv(eulers[row * colCount + col]);
        }
        for (std::size_t slot = {}; slot < quatCount; ++slot) {
            tdm::fquat q{angles[slot], meta->rotationSequence, meta->rotationSigns};
            quaternions[((slot * 4u) + 0ul) * colCount + col] = q.x;
            quaternions[((slot * 4u) + 1ul) * colCount + col] = q.y;
            quaternions[((slot * 4u) + 2ul) * colCount + col] = q.z;
            quaternions[((slot * 4u) + 3ul) * colCount + col] = q.w;
        }
    }

    const auto newRowCount = static_cast<std::uint32_t>(group.outputIndices.size());
    const auto paddedRowCount = extd::roundUp(newRowCount, qjc::PadTo);
    const auto storageColCount = static_cast<std::uint32_t>(colCount);

    group.colCount = storageColCount;
    group.rowCount = paddedRowCount;
    Extent srcDims{newRowCount, storageColCount};
    Extent dstDims{paddedRowCount, storageColCount};
    RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, MatrixOptimizer, void>(config,
                                                                                            quaternions,
                                                                                            srcDims,
                                                                                            dstDims,
                                                                                            group.values);
}

void QuaternionJointsBuilder::setLODs(JointGroup& group, ConstArrayView<std::uint16_t> outputIndices) {
    const auto maxRemappedRotationIndex = [](std::uint16_t absRotAttrIndex) {
        const auto jointIndex = static_cast<std::uint16_t>(absRotAttrIndex / 9u);
        const auto newAttrBase = static_cast<std::uint16_t>(jointIndex * 10u);
        // Only rotation indices are inputs; the maximum is the last quaternion attribute, qw = 6 in
        // [tx, ty, tz, qx, qy, qz, qw, sx, sy, sz]
        return static_cast<std::uint16_t>(newAttrBase + 6);
    };

    const auto newRowCount = static_cast<std::uint32_t>(group.outputIndices.size());
    const auto paddedRowCount = extd::roundUp(newRowCount, qjc::PadTo);
    const auto lodCount = static_cast<std::uint16_t>(group.lods.size());
    for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
        // DNA per-LOD row counts are not bounded by the LOD-0 row count and the storage validator runs only after
        // this builder; unclamped, an increasing LOD row count indexes past the filtered output indices.
        const std::uint32_t oldLODRowCount =
            std::min(group.lods[lod].outputLODs.size, static_cast<std::uint32_t>(outputIndices.size()));
        std::uint32_t newLODRowCount = {};
        if (oldLODRowCount != 0) {
            const auto qwRotationIndexAtOldLODRowCount = maxRemappedRotationIndex(outputIndices[oldLODRowCount - 1ul]);
            auto it = std::find(group.outputIndices.begin(), group.outputIndices.end(), qwRotationIndexAtOldLODRowCount);
            assert(it != group.outputIndices.end());
            newLODRowCount = static_cast<std::uint16_t>(std::distance(group.outputIndices.begin(), it) + 1);
        }
        RowLOD outputLOD{newLODRowCount, paddedRowCount, qjc::BlockHeight, qjc::PadTo};
        group.lods[lod].outputLODs = outputLOD;
    }
}

void QuaternionJointsBuilder::remapOutputIndices(JointGroup& group) {
    for (auto& outputIndex : group.outputIndices) {
        const auto jointIndex = static_cast<std::uint16_t>(outputIndex / 10u);
        const auto relAttrIndex = static_cast<std::uint16_t>(outputIndex % 10u);
        const auto newAttrBase = static_cast<std::uint16_t>(jointIndex * 9u);
        // Only rotations are output indices; qx, qy, qz are kept, qw is dropped
        outputIndex = (relAttrIndex == 6) ? std::uint16_t{} : static_cast<std::uint16_t>(newAttrBase + relAttrIndex);
    }
}

void QuaternionJointsBuilder::fillStorage(const JointBehaviorFilter& source) {
    rotationUnit = source.getRotationUnit();

    for (std::uint16_t jgi = {}; jgi < static_cast<std::uint16_t>(jointGroups.size()); ++jgi) {
        auto rowCount = source.getRowCount(jgi);
        auto colCount = source.getColumnCount(jgi);
        if ((rowCount == 0u) || (colCount == 0u)) {
            continue;
        }

        JointGroup& group = jointGroups[jgi];

        Vector<float> eulers(static_cast<std::size_t>(rowCount) * static_cast<std::size_t>(colCount), {}, memRes);
        source.copyValues(jgi, eulers);

        Vector<std::uint16_t> inputIndices{colCount, {}, memRes};
        source.copyInputIndices(jgi, inputIndices);

        Vector<std::uint16_t> outputIndices{rowCount, {}, memRes};
        source.copyOutputIndices(jgi, outputIndices);

        JointGroupOptimizer::defragment(source,
                                        jgi,
                                        eulers,
                                        inputIndices,
                                        outputIndices,
                                        group.lods,
                                        config.translationPruningThreshold,
                                        config.rotationPruningThreshold,
                                        config.scalePruningThreshold);

        setInputIndices(group, inputIndices);
        setOutputIndices(group, outputIndices);
        setValues(group, eulers, inputIndices, outputIndices);
        setLODs(group, outputIndices);

        // Euler output needs the indices mapped back to 9-attribute joints; quaternion output is already in place.
        if (config.rotationType == RotationType::EulerAngles) {
            remapOutputIndices(group);
        }
    }
}

void QuaternionJointsBuilder::registerControls(Controls* controls) {
    for (const auto& group : jointGroups) {
        for (std::uint16_t lod = {}; lod < static_cast<std::uint16_t>(group.lods.size()); ++lod) {
            const auto inputIndicesForLOD =
                ConstArrayView<std::uint16_t>{group.inputIndices}.first(group.lods[lod].inputLODs.size);
            controls->registerControls(lod, inputIndicesForLOD);
        }
    }
}

JointsEvaluator::Pointer QuaternionJointsBuilder::build() {
    // Auto until this builder writes it on the create path; the deserialized kind on restore.
    const EvaluatorType type = meta->evaluators.quaternionJoints;

    auto jointGroupsEmpty = [this]() {
        for (std::size_t i = {}; i < jointGroups.size(); ++i) {
            if (jointGroups[i].rowCount != 0u) {
                return false;
            }
        }
        return true;
    };

    if ((type == EvaluatorType::Null) || ((type == EvaluatorType::Auto) && jointGroupsEmpty())) {
        meta->evaluators.quaternionJoints = EvaluatorType::Null;
        return UniqueInstance<JointsNullEvaluator, JointsEvaluator>::with(memRes).create();
    }

    using StrategyPointer = UniqueInstance<JointGroupQuaternionCalculationStrategy>::PointerType;
    StrategyPointer strategy;
#ifdef RL_BUILD_WITH_FAST
    if (config.floatingPointModel == FloatingPointModel::Fast) {
        strategy = createFastQuaternionStrategy(config, meta->rotationSequence, meta->rotationSigns, rotationUnit, memRes);
    }
#endif  // RL_BUILD_WITH_FAST
    if (strategy == nullptr) {
        strategy = createPreciseQuaternionStrategy(config, meta->rotationSequence, meta->rotationSigns, rotationUnit, memRes);
    }

    if (strategy == nullptr) {
        meta->evaluators.quaternionJoints = EvaluatorType::Null;
        return UniqueInstance<JointsNullEvaluator, JointsEvaluator>::with(memRes).create();
    }

    if (!RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, StorageValidator, bool>(config, jointGroups, *meta)) {
        return nullptr;
    }

    meta->evaluators.quaternionJoints = EvaluatorType::Concrete;
    auto factory = UniqueInstance<QuaternionJointsEvaluator, JointsEvaluator>::with(memRes);
    return factory.create(std::move(strategy), std::move(jointGroups), nullptr, memRes);
}

}  // namespace rl4
