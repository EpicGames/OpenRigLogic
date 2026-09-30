// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/animatedmaps/AnimatedMapsFactory.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/animatedmaps/AnimatedMapsImpl.h"
#include "riglogic/animatedmaps/AnimatedMapsImplOutputInstance.h"
#include "riglogic/animatedmaps/AnimatedMapsNull.h"
#include "riglogic/animatedmaps/AnimatedMapsOutputInstance.h"
#include "riglogic/animatedmaps/AnimatedMapsValidator.h"
#include "riglogic/conditionaltable/ConditionalTable.h"
#include "riglogic/controls/Controls.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/utils/Extd.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

static AnimatedMapsOutputInstance::Factory createAnimatedMapsOutputInstanceFactory(const Configuration& /*unused*/,
                                                                                   std::uint16_t animatedMapCount) {
    return [=](MemoryResource* memRes) {
        return UniqueInstance<AnimatedMapsImplOutputInstance, AnimatedMapsOutputInstance>::with(memRes).create(animatedMapCount,
                                                                                                               memRes);
    };
}

AnimatedMaps::Pointer AnimatedMapsFactory::create(const Configuration& config, RigMetadata* meta, MemoryResource* memRes) {
    const EvaluatorType type = meta->evaluators.animatedMaps;

    if (type == EvaluatorType::Null) {
        return UniqueInstance<AnimatedMapsNull, AnimatedMaps>::with(memRes).create();
    }

    auto instanceFactory = createAnimatedMapsOutputInstanceFactory(config, meta->animatedMapCount);
    auto moduleFactory = UniqueInstance<AnimatedMapsImpl, AnimatedMaps>::with(memRes);
    return moduleFactory.create(Vector<std::uint16_t>{memRes}, ConditionalTable{memRes}, instanceFactory);
}

AnimatedMaps::Pointer AnimatedMapsFactory::create(const Configuration& config,
                                                  RigMetadata* meta,
                                                  const dna::Reader* reader,
                                                  Controls* controls,
                                                  MemoryResource* memRes) {
    if (!config.loadAnimatedMaps || (reader->getLODCount() == 0u) || (reader->getAnimatedMapLODs().size() == 0u)) {
        meta->evaluators.animatedMaps = EvaluatorType::Null;
        return UniqueInstance<AnimatedMapsNull, AnimatedMaps>::with(memRes).create();
    }

    Vector<std::uint16_t> lods{memRes};
    Vector<std::uint16_t> inputIndices{memRes};
    Vector<std::uint16_t> outputIndices{memRes};
    Vector<float> fromValues{memRes};
    Vector<float> toValues{memRes};
    Vector<float> slopeValues{memRes};
    Vector<float> cutValues{memRes};

    extd::copy(reader->getAnimatedMapLODs(), lods);
    extd::copy(reader->getAnimatedMapInputIndices(), inputIndices);
    extd::copy(reader->getAnimatedMapOutputIndices(), outputIndices);
    extd::copy(reader->getAnimatedMapFromValues(), fromValues);
    extd::copy(reader->getAnimatedMapToValues(), toValues);
    extd::copy(reader->getAnimatedMapSlopeValues(), slopeValues);
    extd::copy(reader->getAnimatedMapCutValues(), cutValues);

    // The sum must fit the uint16 input width the ConditionalTable stores; reject rather than wrap.
    const std::size_t rawAndPSDCount =
        static_cast<std::size_t>(reader->getRawControlCount()) + static_cast<std::size_t>(reader->getPSDCount());
    if (rawAndPSDCount > static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())) {
        return nullptr;
    }
    const auto inputCount = static_cast<std::uint16_t>(rawAndPSDCount);
    const auto outputCount = reader->getAnimatedMapCount();

    if (!AnimatedMapsValidator::validate(lods, inputIndices, outputIndices, outputCount, *meta)) {
        return nullptr;
    }

    meta->evaluators.animatedMaps = EvaluatorType::Concrete;

    // DNAs may contain these parameters in reverse order
    // i.e. the `from` value is actually larger than the `to` value
    const std::size_t conditionalCount = std::min(fromValues.size(), toValues.size());
    for (std::size_t i = 0ul; i < conditionalCount; ++i) {
        if (fromValues[i] > toValues[i]) {
            std::swap(fromValues[i], toValues[i]);
        }
    }

    for (std::uint16_t lod = {}; lod < static_cast<std::uint16_t>(lods.size()); ++lod) {
        // No clamp needed: the validator above rejected any lods[lod] exceeding inputIndices.size().
        const auto inputIndicesForLOD = ConstArrayView<std::uint16_t>{inputIndices}.first(lods[lod]);
        controls->registerControls(lod, inputIndicesForLOD);
    }

    ConditionalTable conditionals{std::move(inputIndices),
                                  std::move(outputIndices),
                                  std::move(fromValues),
                                  std::move(toValues),
                                  std::move(slopeValues),
                                  std::move(cutValues),
                                  inputCount,
                                  outputCount,
                                  memRes};

    auto instanceFactory = createAnimatedMapsOutputInstanceFactory(config, outputCount);
    auto moduleFactory = UniqueInstance<AnimatedMapsImpl, AnimatedMaps>::with(memRes);
    return moduleFactory.create(std::move(lods), std::move(conditionals), instanceFactory);
}

}  // namespace rl4
