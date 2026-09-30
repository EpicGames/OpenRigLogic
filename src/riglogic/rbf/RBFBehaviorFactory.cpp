// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/rbf/RBFBehaviorFactory.h"

#include "riglogic/rbf/RBFBehaviorEvaluator.h"
#include "riglogic/rbf/RBFBehaviorNullEvaluator.h"
#include "riglogic/rbf/cpu/CPURBFBehaviorFactory.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/Utils.h"

#include <cstdint>

namespace rl4 {

namespace {

// RBF storage is always float (T, the runtime storage type, is unused) and the solvers run at up to 256-bit width.
template<typename T, typename TF512, typename TF256, typename TF128>
struct RBFEvaluatorFactory {
    RBFBehaviorEvaluator::Pointer operator()(RigMetadata* meta, const dna::Reader* reader, MemoryResource* memRes) {
        return rbf::cpu::Factory<float, TF256, TF128>::create(meta, reader, memRes);
    }
};

}  // namespace

RBFBehaviorEvaluator::Pointer createRBFEvaluator(const Configuration& config,
                                                 RigMetadata* meta,
                                                 const dna::Reader* reader,
                                                 MemoryResource* memRes) {
    return RuntimeTemplateInstantiator::invoke<FloatingPointModel::Precise, RBFEvaluatorFactory, RBFBehaviorEvaluator::Pointer>(
        config,
        meta,
        reader,
        memRes);
}

RBFBehavior::Pointer RBFBehaviorFactory::create(const Configuration& config,
                                                RigMetadata* meta,
                                                const dna::Reader* reader,
                                                MemoryResource* memRes) {
    auto moduleFactory = UniqueInstance<RBFBehavior>::with(memRes);
    if (!config.loadRBFBehavior || (meta->lodCount == 0u) || (meta->rbfSolverCount == 0u) || (meta->rbfControlCount == 0u)) {
        meta->evaluators.rbfBehavior = EvaluatorType::Null;
        auto evaluator = UniqueInstance<RBFBehaviorNullEvaluator, RBFBehaviorEvaluator>::with(memRes).create();
        return moduleFactory.create(std::move(evaluator));
    }

    meta->evaluators.rbfBehavior = EvaluatorType::Concrete;
    auto evaluator = createRBFEvaluator(config, meta, reader, memRes);
    if (!evaluator) {
        return nullptr;
    }
    return moduleFactory.create(std::move(evaluator));
}

RBFBehavior::Pointer RBFBehaviorFactory::create(const Configuration& config, RigMetadata* meta, MemoryResource* memRes) {
    auto moduleFactory = UniqueInstance<RBFBehavior>::with(memRes);
    const EvaluatorType type = meta->evaluators.rbfBehavior;

    if (type == EvaluatorType::Null) {
        auto evaluator = UniqueInstance<RBFBehaviorNullEvaluator, RBFBehaviorEvaluator>::with(memRes).create();
        return moduleFactory.create(std::move(evaluator));
    }

    return moduleFactory.create(createRBFEvaluator(config, meta, nullptr, memRes));
}

}  // namespace rl4
