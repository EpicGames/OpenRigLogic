// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/joints/cpu/bpcm/BPCMJointsEvaluator.h"

#include "riglogic/SerializationContext.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/Utils.h"

namespace rl4 {

namespace bpcm {

Evaluator::Evaluator(JointStorage&& storage_,
                     Vector<JointGroupView>&& jointGroups_,
                     CalculationStrategyPointer strategy_,
                     JointsOutputInstance::Factory instanceFactory_,
                     MemoryResource* memRes_) :
    memRes{memRes_},
    storage{std::move(storage_)},
    jointGroups{std::move(jointGroups_)},
    strategy{std::move(strategy_)},
    instanceFactory{instanceFactory_} {
}

JointsOutputInstance::Pointer Evaluator::createInstance(MemoryResource* instanceMemRes) const {
    return instanceFactory(instanceMemRes);
}

std::uint32_t Evaluator::getJointDeltaValueCountForLOD(std::uint16_t lod) const {
    std::uint32_t deltaCount = {};
    for (const auto& group : jointGroups) {
        deltaCount += (group.lods[lod].inputLODs.size * group.lods[lod].outputLODs.size);
    }
    return deltaCount;
}

void Evaluator::calculate(ControlsInputInstance* inputs, JointsOutputInstance* outputs, std::uint16_t lod) const {
    if ((lod < storage.outputRowsPerLOD.size()) && (storage.outputRowsPerLOD[lod] == 0u)) {
        return;
    }
    // Iterate the container directly: it is the authoritative bound (empty on a failed load), and a
    // uint16_t loop index would wrap on a snapshot carrying more groups than the type addresses.
    assert(strategy != nullptr);
    for (const auto& jointGroup : jointGroups) {
        if (jointGroup.rowCount != 0u) {
            strategy->calculate(jointGroup, inputs->getInputBuffer(), outputs->getOutputBuffer(), lod);
        }
    }
}

void Evaluator::calculate(ControlsInputInstance* inputs,
                          JointsOutputInstance* outputs,
                          std::uint16_t lod,
                          std::uint16_t jointGroupIndex) const {
    assert(strategy != nullptr);
    // Reachable by well-behaved callers (unclamped public-API group index), and a hostile snapshot can advertise more
    // groups than were deserialized, so this must be a guard, not an assert.
    if (jointGroupIndex >= jointGroups.size()) {
        return;
    }
    const auto& jointGroup = jointGroups[jointGroupIndex];
    if (jointGroup.rowCount != 0u) {
        strategy->calculate(jointGroup, inputs->getInputBuffer(), outputs->getOutputBuffer(), lod);
    }
}

void Evaluator::load(BoundedInputArchive& archive) {
    archive(storage);
    const SerializationContext* context = static_cast<SerializationContext*>(archive.getUserData());
    if (!RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, StorageValidator, bool>(
            *context->config,
            storage,
            *context->metadata,
            context->config->rotationType)) {
        archive.markMalformed();
        jointGroups.clear();
        return;
    }
    jointGroups = RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, StorageSnapshot, Vector<JointGroupView>>(
        *context->config,
        storage,
        memRes);
}

void Evaluator::save(terse::BinaryOutputArchive<BoundedIOStream>& archive) {
    archive(storage);
}

}  // namespace bpcm

}  // namespace rl4
