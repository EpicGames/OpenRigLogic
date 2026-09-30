// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/joints/cpu/bpcm/BPCMJointsBuilder.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/controls/Controls.h"
#include "riglogic/joints/JointBehaviorFilter.h"
#include "riglogic/joints/JointsBuilder.h"
#include "riglogic/joints/JointsNullEvaluator.h"
#include "riglogic/joints/cpu/bpcm/BPCMCalculationStrategy.h"
#include "riglogic/joints/cpu/bpcm/BPCMJointsEvaluator.h"
#include "riglogic/joints/cpu/bpcm/BPCMJointsStrategyFactory.h"
#include "riglogic/joints/cpu/bpcm/RotationAdapters.h"
#include "riglogic/joints/cpu/bpcm/Storage.h"
#include "riglogic/joints/cpu/utils/JointGroupOptimizer.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/types/Aliases.h"
#include "riglogic/types/bpcm/Optimizer.h"
#include "riglogic/utils/Extd.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

namespace bpcm {

template<typename TFVec>
static constexpr std::uint32_t BlockHeight() {
    return static_cast<std::uint32_t>(TFVec::size() * 2ul);
}

template<typename TFVec>
static constexpr std::uint32_t PadTo() {
    return static_cast<std::uint32_t>(TFVec::size());
}

template<typename TFVec>
static constexpr std::uint32_t Stride() {
    return 1u;
}

template<typename TIterator>
static void remapOutputIndicesForQuaternions(TIterator begin, TIterator end) {
    for (auto it = begin; it != end; ++it) {
        const auto absAttrIndex = *it;
        const auto jointIndex = static_cast<std::uint16_t>(absAttrIndex / 9u);
        const auto relAttrIndex = static_cast<std::uint16_t>(absAttrIndex % 9u);
        const auto newAttrBase = static_cast<std::uint16_t>(jointIndex * 10u);
        // Only scale relative attribute index is offset by one when output is in quaternions
        const auto newRelAttrIndex = (relAttrIndex < 6u ? relAttrIndex : static_cast<std::uint16_t>(relAttrIndex + 1u));
        *it = static_cast<std::uint16_t>(newAttrBase + newRelAttrIndex);
    }
}

struct VectorizationParameters {
    std::uint32_t blockHeight;
    std::uint32_t padTo;
    std::uint32_t stride;
};

// Storage rows the kernel loads for a view of viewRows out of a group padded to paddedRows; must mirror the
// full-block / masked-block / half-block loop structure of processJointGroupBlock4 and PaddedBlockView.
static std::uint32_t touchedRowCount(std::uint32_t viewRows, std::uint32_t paddedRows, std::uint32_t blockHeight) {
    if (viewRows == 0u) {
        return 0u;
    }
    const std::uint32_t halfBlockHeight = blockHeight / 2u;
    const RowLOD view{viewRows, paddedRows, blockHeight, halfBlockHeight};
    std::uint32_t touched = view.sizePaddedToLastFullBlock;
    if (viewRows > view.sizePaddedToLastFullBlock) {
        touched += extd::roundUp(viewRows - view.sizePaddedToLastFullBlock, halfBlockHeight);
    }
    return touched;
}

template<typename T, typename TF512, typename TF256, typename TF128>
struct DetectVectorizationParameters {

    VectorizationParameters operator()(ConstArrayView<LODRegion> lods, std::uint32_t unpaddedRowCount) {
        // Pick the block height minimizing estimated cost over all LODs: the kernel is bandwidth-bound, so cost is rows
        // streamed, plus an instruction penalty for sub-8-lane blocks (measured: quarter width must at least halve the
        // rows touched to win). Ties resolve to the widest block.
        static constexpr std::uint32_t eightLaneBlockHeight = 16u;
        using TFWidest = TBPCMVec<TF512, TF256, TF128>;
        std::uint32_t blockHeight = BlockHeight<TF128>();
        std::uint64_t leastCost = ~0ull;
        for (std::uint32_t candidate = BlockHeight<TFWidest>(); candidate >= BlockHeight<TF128>(); candidate /= 2u) {
            const std::uint32_t paddedRows = extd::roundUp(unpaddedRowCount, candidate / 2u);
            const std::uint64_t costPerTouchedRow = (candidate >= eightLaneBlockHeight) ? 4u : 6u;
            std::uint64_t cost = {};
            for (const auto& lodRegion : lods) {
                cost += costPerTouchedRow * touchedRowCount(lodRegion.outputLODs.size, paddedRows, candidate);
            }
            if (cost < leastCost) {
                leastCost = cost;
                blockHeight = candidate;
            }
        }
        VectorizationParameters params = {};
        params.blockHeight = blockHeight;
        params.padTo = blockHeight / 2u;
        params.stride = 1u;
        return params;
    }
};

template<typename T, typename TF512, typename TF256, typename TF128>
struct MatrixOptimizer {

    std::uint32_t operator()(ConstArrayView<float> src,
                             Extent srcDims,
                             Extent dstDims,
                             FloatArray& dst,
                             std::uint32_t offset,
                             std::uint32_t blockHeight) {
        // `offset` may exceed the current size (group starts are alignment-padded);
        // the gap elements are value-initialized and never read
        dst.resize<T>(offset + dstDims.size());
        switch (blockHeight) {
        case BlockHeight<TF512>(): {
            using BPCMOptimizer = bpcm::Optimizer<TF512, BlockHeight<TF512>(), PadTo<TF512>(), Stride<TF512>()>;
            return BPCMOptimizer::optimize(dst.data<T>() + offset, src.data(), srcDims);
        }
        case BlockHeight<TF256>(): {
            using BPCMOptimizer = bpcm::Optimizer<TF256, BlockHeight<TF256>(), PadTo<TF256>(), Stride<TF256>()>;
            return BPCMOptimizer::optimize(dst.data<T>() + offset, src.data(), srcDims);
        }
        default: {
            using BPCMOptimizer = bpcm::Optimizer<TF128, BlockHeight<TF128>(), PadTo<TF128>(), Stride<TF128>()>;
            return BPCMOptimizer::optimize(dst.data<T>() + offset, src.data(), srcDims);
        }
        }
    }
};

BPCMJointsBuilder::BPCMJointsBuilder(const Configuration& config_, RigMetadata* meta_, MemoryResource* memRes_) :
    config{config_},
    meta{meta_},
    memRes{memRes_},
    storage{memRes},
    rotationUnit{},
    lodCount{} {
}

void BPCMJointsBuilder::computeStorageRequirements() {
}

void BPCMJointsBuilder::computeStorageRequirements(const JointBehaviorFilter& /*unused*/) {
}

void BPCMJointsBuilder::allocateStorage(const JointBehaviorFilter& source) {
    lodCount = source.getLODCount();
    storage.jointGroups.resize(source.getJointGroupCount());
    storage.lodRegions.resize(storage.jointGroups.size() * static_cast<std::size_t>(lodCount));
    storage.outputRowsPerLOD.resize(lodCount);
    if (config.rotationType == RotationType::Quaternions) {
        storage.outputRotationLODs.resize(storage.lodRegions.size());
    }
}

void BPCMJointsBuilder::fillStorage(const JointBehaviorFilter& source) {
    rotationUnit = source.getRotationUnit();
    const auto features = getActiveFeatures(config);

    Vector<float> values{memRes};
    Vector<std::uint16_t> inputIndices{memRes};
    Vector<std::uint16_t> outputIndices{memRes};
    std::uint32_t valueOffset = {};
    std::uint32_t inputOffset = {};
    std::uint32_t outputOffset = {};
    std::uint32_t lodOffset = {};
    for (std::uint16_t i = {}; i < source.getJointGroupCount(); ++i) {
        const auto colCount = static_cast<std::uint32_t>(source.getColumnCount(i));
        const auto rowCount = static_cast<std::uint32_t>(source.getRowCount(i));

        values.resize(rowCount * colCount);
        source.copyValues(i, values);

        inputIndices.resize(colCount);
        source.copyInputIndices(i, inputIndices);

        outputIndices.resize(rowCount);
        source.copyOutputIndices(i, outputIndices);

        // This step might reduce both value count and input index count (by eliminating empty columns)
        ArrayView<LODRegion> lods{storage.lodRegions.data() + lodOffset, source.getLODCount()};
        JointGroupOptimizer::defragment(source,
                                        i,
                                        values,
                                        inputIndices,
                                        outputIndices,
                                        lods,
                                        config.translationPruningThreshold,
                                        config.rotationPruningThreshold,
                                        config.scalePruningThreshold);
        const auto optimizedColCount = static_cast<std::uint32_t>(inputIndices.size());
        const auto optimizedRowCount = static_cast<std::uint32_t>(outputIndices.size());

        VectorizationParameters vecParams =
            RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise,
                                                DetectVectorizationParameters,
                                                VectorizationParameters>(features,
                                                                         ConstArrayView<LODRegion>{lods},
                                                                         optimizedRowCount);

        const auto padding = extd::roundUp(optimizedRowCount, vecParams.padTo) - optimizedRowCount;
        const auto paddedRowCount = optimizedRowCount + padding;
        storage.jointGroups[i].valuesSize = optimizedColCount * paddedRowCount;
        storage.inputIndices.resize(storage.inputIndices.size() + inputIndices.size());
        storage.outputIndices.resize(storage.outputIndices.size() + paddedRowCount);

        // Source extent is the unpadded row count by the optimized column count
        const Extent srcDims{optimizedRowCount, optimizedColCount};
        const Extent dstDims{paddedRowCount, optimizedColCount};
        // With per-group block heights a preceding group may end on a boundary too small for this group's vector width;
        // rounding the offset up to the half-block element count keeps every aligned kernel load within the group aligned.
        valueOffset = extd::roundUp(valueOffset, vecParams.blockHeight / 2u);
        storage.jointGroups[i].valuesOffset = valueOffset;
        valueOffset += RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, MatrixOptimizer, std::uint32_t>(
            features,
            values,
            srcDims,
            dstDims,
            storage.values,
            valueOffset,
            vecParams.blockHeight);

        std::copy(inputIndices.begin(), inputIndices.end(), extd::advanced(storage.inputIndices.begin(), inputOffset));
        storage.jointGroups[i].inputIndicesOffset = inputOffset;
        inputOffset += optimizedColCount;

        std::copy(outputIndices.begin(), outputIndices.end(), extd::advanced(storage.outputIndices.begin(), outputOffset));
        storage.jointGroups[i].outputIndicesOffset = outputOffset;
        outputOffset += paddedRowCount;

        for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
            auto& lodRegion = lods[lod];
            lodRegion.outputLODs = RowLOD(lodRegion.outputLODs.size, paddedRowCount, vecParams.blockHeight, vecParams.padTo);
            storage.outputRowsPerLOD[lod] += lodRegion.outputLODs.size;
        }

        storage.jointGroups[i].lodsOffset = lodOffset;
        lodOffset += source.getLODCount();
        storage.jointGroups[i].colCount = optimizedColCount;
        storage.jointGroups[i].rowCount = paddedRowCount;
        storage.jointGroups[i].blockHeight = vecParams.blockHeight;
    }

    if (config.rotationType == RotationType::Quaternions) {
        // Remap output indices from 9-attribute joints to 10-attribute joints
        remapOutputIndicesForQuaternions(storage.outputIndices.begin(), storage.outputIndices.end());
        const JointBehaviorFilter filtered = source.only(dna::RotationRepresentation::EulerAngles);
        setOutputRotationIndices(filtered);
    }
}

void BPCMJointsBuilder::registerControls(Controls* controls) {
    for (const auto& group : storage.jointGroups) {
        ConstArrayView<LODRegion> lods{storage.lodRegions.data() + group.lodsOffset, lodCount};
        for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
            ConstArrayView<std::uint16_t> inputIndicesForLOD(storage.inputIndices.data() + group.inputIndicesOffset,
                                                             lods[lod].inputLODs.size);
            controls->registerControls(lod, inputIndicesForLOD);
        }
    }
}

void BPCMJointsBuilder::setOutputRotationIndices(const JointBehaviorFilter& source) {
    std::uint32_t outputOffset = {};
    for (std::uint16_t jgi = {}; jgi < source.getJointGroupCount(); ++jgi) {
        auto colCount = static_cast<std::uint32_t>(source.getColumnCount(jgi));
        auto rowCount = static_cast<std::uint32_t>(source.getRowCount(jgi));

        Vector<float> values{rowCount * colCount, {}, memRes};
        source.copyValues(jgi, values);

        Vector<std::uint16_t> inputIndices{colCount, {}, memRes};
        source.copyInputIndices(jgi, inputIndices);

        Vector<std::uint16_t> outputRotationIndices{rowCount, {}, memRes};
        source.copyOutputIndices(jgi, outputRotationIndices);

        Vector<LODRegion> lods{source.getLODCount(), {}, memRes};
        for (std::uint16_t lod = {}; lod < source.getLODCount(); ++lod) {
            lods[lod].outputLODs.size = source.getRowCountForLOD(jgi, lod);
        }
        JointGroupOptimizer::defragment(source,
                                        jgi,
                                        values,
                                        inputIndices,
                                        outputRotationIndices,
                                        lods,
                                        config.translationPruningThreshold,
                                        config.rotationPruningThreshold,
                                        config.scalePruningThreshold);

        remapOutputIndicesForQuaternions(outputRotationIndices.begin(), outputRotationIndices.end());
// Given any rotation indices (qx, qy, qz), return only qx indices for all joints in the group
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
        outputRotationBaseIndices.reserve(outputRotationIndices.size() / 3ul);
        std::transform(outputRotationIndices.begin(),
                       outputRotationIndices.end(),
                       std::back_inserter(outputRotationBaseIndices),
                       [](std::uint16_t outputIndex) { return static_cast<std::uint16_t>((outputIndex / 10) * 10 + 3); });
        deduplicate(outputRotationBaseIndices);
        std::copy(outputRotationBaseIndices.begin(),
                  outputRotationBaseIndices.end(),
                  std::back_inserter(storage.outputRotationIndices));
        storage.jointGroups[jgi].outputRotationIndicesOffset = outputOffset;
        // Must be called before outputOffset is adjusted
        setOutputRotationLODs(lods, outputRotationIndices, outputOffset, jgi);
        outputOffset += static_cast<std::uint32_t>(outputRotationBaseIndices.size());
    }
}

void BPCMJointsBuilder::setOutputRotationLODs(ConstArrayView<LODRegion> lods,
                                              ConstArrayView<std::uint16_t> outputRotationIndices,
                                              std::uint32_t outputOffset,
                                              std::uint16_t jointGroupIndex) {
    const auto offset = static_cast<std::uint32_t>(jointGroupIndex * lodCount);
    for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
        // LOD row counts are raw DNA values; clamp to the rotation rows that exist.
        const auto oldLODRowCount = std::min(static_cast<std::size_t>(lods[lod].outputLODs.size), outputRotationIndices.size());
        if (oldLODRowCount == 0) {
            storage.outputRotationLODs[offset + lod] = 0;
            continue;
        }

        // The rotation LOD is a prefix of the per-joint qx list, so it must reach the deepest-stored joint among ALL
        // rows the LOD drives; joint rows may interleave, so the last row's joint alone is not enough.
        const auto start = extd::advanced(storage.outputRotationIndices.begin(), outputOffset);
        const auto end = storage.outputRotationIndices.end();
        std::size_t newLODRowCount = {};
        for (std::size_t row = {}; row < oldLODRowCount; ++row) {
            const auto qxRotationIndex = static_cast<std::uint16_t>((outputRotationIndices[row] / 10) * 10 + 3);
            const auto it = std::find(start, end, qxRotationIndex);
            if (it != end) {
                newLODRowCount = std::max(newLODRowCount, static_cast<std::size_t>(std::distance(start, it) + 1));
            }
        }
        storage.outputRotationLODs[offset + lod] = static_cast<std::uint16_t>(newLODRowCount);
    }
    storage.jointGroups[jointGroupIndex].outputRotationLODsOffset = offset;
}

JointsEvaluator::Pointer BPCMJointsBuilder::build() {
    // Auto until this builder writes it on the create path; the deserialized kind on restore.
    const EvaluatorType type = meta->evaluators.bpcmJoints;

    auto jointGroupsEmpty = [this]() {
        for (std::size_t i = {}; i < storage.jointGroups.size(); ++i) {
            if (storage.jointGroups[i].rowCount != 0u) {
                return false;
            }
        }
        return true;
    };

    if ((type == EvaluatorType::Null) || ((type == EvaluatorType::Auto) && jointGroupsEmpty())) {
        meta->evaluators.bpcmJoints = EvaluatorType::Null;
        return UniqueInstance<JointsNullEvaluator, JointsEvaluator>::with(memRes).create();
    }

    using StrategyPointer = UniqueInstance<JointGroupLinearCalculationStrategy>::PointerType;
    StrategyPointer strategy;
#ifdef RL_BUILD_WITH_FAST
    if (config.floatingPointModel == FloatingPointModel::Fast) {
        strategy = createFastLinearStrategy(config, meta->rotationSequence, meta->rotationSigns, rotationUnit, memRes);
    }
#endif  // RL_BUILD_WITH_FAST
    if (strategy == nullptr) {
        strategy = createPreciseLinearStrategy(config, meta->rotationSequence, meta->rotationSigns, rotationUnit, memRes);
    }

    if (strategy == nullptr) {
        meta->evaluators.bpcmJoints = EvaluatorType::Null;
        return UniqueInstance<JointsNullEvaluator, JointsEvaluator>::with(memRes).create();
    }

    // Same StorageValidator as the restore path. Precise tag: validation does no FP math and the block strides it
    // checks are tag-invariant.
    if (!RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, StorageValidator, bool>(config,
                                                                                                  storage,
                                                                                                  *meta,
                                                                                                  config.rotationType)) {
        return nullptr;
    }

    meta->evaluators.bpcmJoints = EvaluatorType::Concrete;
    auto jointGroups =
        RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, StorageSnapshot, Vector<JointGroupView>>(config,
                                                                                                                  storage,
                                                                                                                  memRes);
    auto factory = UniqueInstance<Evaluator, JointsEvaluator>::with(memRes);
    return factory.create(std::move(storage), std::move(jointGroups), std::move(strategy), nullptr, memRes);
}

}  // namespace bpcm

}  // namespace rl4
