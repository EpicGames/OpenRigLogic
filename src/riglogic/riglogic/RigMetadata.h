// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/Configuration.h"

#include <cstdint>
#include <limits>

namespace rl4 {

enum class InitializationMethod : std::uint16_t {
    Create,
    Restore
};

enum class EvaluatorType : std::uint16_t {
    Auto,
    Null,
    Concrete
};

// A field per subsystem, no self-describing records: a snapshot is restorable only by the exact library version
// that wrote it, and fixed fields make stream desyncs structurally impossible.
struct EvaluatorTypes {
    EvaluatorType mlBehavior = EvaluatorType::Auto;
    EvaluatorType rbfBehavior = EvaluatorType::Auto;
    EvaluatorType bpcmJoints = EvaluatorType::Auto;
    EvaluatorType quaternionJoints = EvaluatorType::Auto;
    EvaluatorType twistSwingJoints = EvaluatorType::Auto;
    EvaluatorType mlJoints = EvaluatorType::Auto;
    EvaluatorType blendShapes = EvaluatorType::Auto;
    EvaluatorType animatedMaps = EvaluatorType::Auto;
    EvaluatorType psdNet = EvaluatorType::Auto;

    template<class Archive>
    void serialize(Archive& archive) {
        archive(mlBehavior,
                rbfBehavior,
                bpcmJoints,
                quaternionJoints,
                twistSwingJoints,
                mlJoints,
                blendShapes,
                animatedMaps,
                psdNet);
    }
};

struct RigMetadata {
    using Pointer = UniqueInstance<RigMetadata>::PointerType;

    tdm::coord_sys coordinateSystem;
    tdm::rot_seq rotationSequence;
    tdm::rot_sign rotationSigns;
    std::uint16_t lodCount;
    std::uint16_t guiControlCount;
    std::uint16_t rawControlCount;
    std::uint16_t psdControlCount;
    std::uint16_t mlControlCount;
    std::uint16_t rbfControlCount;
    std::uint16_t jointGroupCount;
    std::uint16_t jointAttributeCount;
    std::uint16_t blendShapeCount;
    std::uint16_t animatedMapCount;
    std::uint16_t mlTypeCount;
    std::uint16_t rbfSolverCount;
    std::uint16_t twistCount;
    std::uint16_t swingCount;
    // Which evaluator each subsystem built (create) or must rebuild (restore).
    EvaluatorTypes evaluators;
    // ML-joints template discriminators, derived from the DNA in fillStorage(), which restore never runs; restore()
    // runs the factories before any evaluator blob streams, and MLJointsBuilder::build() selects the template from
    // these. The defaults are valid values so validate() can range-check them unconditionally.
    RotationType mlJointsRotationType;
    dna::RotationUnit mlJointsRotationUnit;
    std::uint16_t mlJointsCoordSysTransformed;

    // Returns nullptr when jointCount x attrsPerJoint exceeds the uint16 joint output index space.
    static Pointer create(const Configuration& config, const dna::Reader* reader, MemoryResource* memRes) {
        const auto numAttrsPerJoint =
            (static_cast<std::uint8_t>(config.translationType) + static_cast<std::uint8_t>(config.rotationType) +
             static_cast<std::uint8_t>(config.scaleType));
        const std::size_t attributeCount =
            static_cast<std::size_t>(reader->getJointCount()) * static_cast<std::size_t>(numAttrsPerJoint);
        if (attributeCount > static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max())) {
            return nullptr;
        }

        Pointer meta = UniqueInstance<RigMetadata>::with(memRes).create(memRes);
        meta->coordinateSystem = reader->getCoordinateSystem();
        meta->rotationSequence = reader->getRotationSequence();
        meta->rotationSigns = reader->getRotationSign();
        meta->lodCount = reader->getLODCount();
        meta->guiControlCount = reader->getGUIControlCount();
        meta->rawControlCount = reader->getRawControlCount();
        meta->psdControlCount = reader->getPSDCount();
        meta->jointGroupCount = reader->getJointGroupCount();
        meta->jointAttributeCount = static_cast<std::uint16_t>(attributeCount);
        meta->blendShapeCount = reader->getBlendShapeChannelCount();
        meta->animatedMapCount = reader->getAnimatedMapCount();
        meta->mlControlCount = reader->getMLControlCount();
        meta->mlTypeCount = reader->getMLTypeCount();
        meta->rbfControlCount = reader->getRBFPoseControlCount();
        meta->rbfSolverCount = reader->getRBFSolverCount();
        meta->twistCount = reader->getTwistCount();
        meta->swingCount = reader->getSwingCount();
        return meta;
    }

    explicit RigMetadata(MemoryResource* memRes) :
        coordinateSystem{},
        rotationSequence{},
        rotationSigns{},
        lodCount{},
        guiControlCount{},
        rawControlCount{},
        psdControlCount{},
        mlControlCount{},
        rbfControlCount{},
        jointGroupCount{},
        jointAttributeCount{},
        blendShapeCount{},
        animatedMapCount{},
        mlTypeCount{},
        rbfSolverCount{},
        twistCount{},
        swingCount{},
        evaluators{},
        mlJointsRotationType{RotationType::EulerAngles},
        mlJointsRotationUnit{dna::RotationUnit::degrees},
        mlJointsCoordSysTransformed{} {
        static_cast<void>(memRes);
    }

    template<class Archive>
    void serialize(Archive& archive) {
        archive(coordinateSystem,
                rotationSequence,
                rotationSigns,
                lodCount,
                guiControlCount,
                rawControlCount,
                psdControlCount,
                mlControlCount,
                rbfControlCount,
                jointGroupCount,
                jointAttributeCount,
                blendShapeCount,
                animatedMapCount,
                mlTypeCount,
                rbfSolverCount,
                twistCount,
                swingCount,
                evaluators,
                mlJointsRotationType,
                mlJointsRotationUnit,
                mlJointsCoordSysTransformed);
    }

    // Total length of the per-instance control input buffer (raw, PSD, ML and RBF sections back to back); the
    // authoritative bound for any DNA-derived index into it.
    std::size_t getControlInputCount() const {
        return static_cast<std::size_t>(rawControlCount) + static_cast<std::size_t>(psdControlCount) +
               static_cast<std::size_t>(mlControlCount) + static_cast<std::size_t>(rbfControlCount);
    }

    // Called by restore() before any factory reads the metadata: the rotation enums select calculation-strategy
    // templates, so an out-of-range value picks a template that does not match the stored data.
    bool validate(InitializationMethod method) const {
        // tdm::rot_seq has exactly six orderings; anything else falls through the strategy factory's #ifdef chain.
        switch (rotationSequence) {
        case tdm::rot_seq::xyz:
        case tdm::rot_seq::xzy:
        case tdm::rot_seq::yxz:
        case tdm::rot_seq::yzx:
        case tdm::rot_seq::zxy:
        case tdm::rot_seq::zyx:
            break;
        default:
            return false;
        }
        // Each component of rot_sign must be exactly +1 or -1: the adapters multiply Euler components by it.
        const auto isValidDirection = [](tdm::rot_dir direction) {
            return (direction == tdm::rot_dir::positive) || (direction == tdm::rot_dir::negative);
        };
        if (!isValidDirection(rotationSigns.x) || !isValidDirection(rotationSigns.y) || !isValidDirection(rotationSigns.z)) {
            return false;
        }
        if ((mlJointsRotationType != RotationType::EulerAngles) && (mlJointsRotationType != RotationType::Quaternions)) {
            return false;
        }
        if ((mlJointsRotationUnit != dna::RotationUnit::degrees) && (mlJointsRotationUnit != dna::RotationUnit::radians)) {
            return false;
        }
        if (mlJointsCoordSysTransformed > 1u) {
            return false;
        }
        // On restore every evaluator kind is deserialized and must be one a factory can rebuild; Auto exists only
        // pre-factory on the create path.
        if (method == InitializationMethod::Create) {
            return true;
        }
        const EvaluatorType kinds[] = {evaluators.mlBehavior,
                                       evaluators.rbfBehavior,
                                       evaluators.bpcmJoints,
                                       evaluators.quaternionJoints,
                                       evaluators.twistSwingJoints,
                                       evaluators.mlJoints,
                                       evaluators.blendShapes,
                                       evaluators.animatedMaps,
                                       evaluators.psdNet};
        for (const auto kind : kinds) {
            if ((kind != EvaluatorType::Null) && (kind != EvaluatorType::Concrete)) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace rl4
