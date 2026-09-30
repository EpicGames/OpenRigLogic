// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/animatedmaps/AnimatedMapsImpl.h"

#include "riglogic/SerializationContext.h"
#include "riglogic/TypeDefs.h"
#include "riglogic/animatedmaps/AnimatedMapsOutputInstance.h"
#include "riglogic/animatedmaps/AnimatedMapsValidator.h"
#include "riglogic/conditionaltable/ConditionalTable.h"
#include "riglogic/controls/ControlsInputInstance.h"
#include "riglogic/riglogic/RigMetadata.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cassert>
#include <cstddef>
#include <utility>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

AnimatedMapsImpl::AnimatedMapsImpl(Vector<std::uint16_t>&& lods_,
                                   ConditionalTable&& conditionals_,
                                   AnimatedMapsOutputInstance::Factory instanceFactory_) :
    lods{std::move(lods_)},
    conditionals{std::move(conditionals_)},
    instanceFactory{instanceFactory_} {
}

AnimatedMapsOutputInstance::Pointer AnimatedMapsImpl::createInstance(MemoryResource* instanceMemRes) const {
    return instanceFactory(instanceMemRes);
}

ConstArrayView<std::uint16_t> AnimatedMapsImpl::getAnimatedMapIndicesForLOD(std::uint16_t lod) const {
    assert(lod < lods.size());
    if (lod >= lods.size()) {
        return {};
    }
    // lods[lod] bounded against the output index count at load().
    return conditionals.getOutputIndices().first(lods[lod]);
}

void AnimatedMapsImpl::calculate(const ControlsInputInstance* inputs,
                                 AnimatedMapsOutputInstance* outputs,
                                 std::uint16_t lod) const {
    assert(lod < lods.size());
    conditionals.calculateForward(inputs->getInputBuffer().data(), outputs->getOutputBuffer().data(), lods[lod]);
}

void AnimatedMapsImpl::load(BoundedInputArchive& archive) {
    archive(lods, conditionals);
    const SerializationContext* context = static_cast<SerializationContext*>(archive.getUserData());
    const RigMetadata& metadata = *context->metadata;
    if (!AnimatedMapsValidator::validate(lods, conditionals, metadata)) {
        archive.markMalformed();
    }
}

void AnimatedMapsImpl::save(terse::BinaryOutputArchive<BoundedIOStream>& archive) {
    archive(lods, conditionals);
}

}  // namespace rl4
