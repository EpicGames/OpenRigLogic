// Copyright Epic Games, Inc. All Rights Reserved.

#include "riglogic/joints/cpu/ml/MLJointsBuilder.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/controls/Controls.h"
#include "riglogic/joints/JointBehaviorFilter.h"
#include "riglogic/joints/JointsNullEvaluator.h"
#include "riglogic/joints/cpu/ml/CoordinateSystemTransformer.h"
#include "riglogic/joints/cpu/ml/MLJointsEvaluator.h"
#include "riglogic/joints/cpu/ml/MLJointsValidator.h"
#include "riglogic/joints/cpu/ml/RotationAdapters.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/SIMD.h"
#include "riglogic/utils/Extd.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

namespace ml {

MLJointsBuilder::MLJointsBuilder(const Configuration& config_, RigMetadata* meta_, MemoryResource* memRes_) :
    memRes{memRes_},
    config{config_},
    meta{meta_},
    inputIndices{memRes},
    outputIndices{memRes},
    inputRotationBaseIndices{memRes},
    outputRotationBaseIndices{memRes},
    uniqueTranslationBaseIndices{memRes},
    uniqueRotationBaseIndices{memRes},
    uniqueScaleBaseIndices{memRes},
    changeOfBasis{tdm::fmat3::identity()},
    srcSeq{meta->rotationSequence},
    srcSigns{meta->rotationSigns},
    dstSeq{meta->rotationSequence},
    dstSigns{meta->rotationSigns},
    inputJointAttrCount{},
    outputJointAttrCount{},
    rotationUnit{},
    isRestore{true},
    mlTranslationType{config.translationType},
    mlRotationType{config.rotationType},
    mlScaleType{config.scaleType} {

    outputJointAttrCount =
        static_cast<std::uint16_t>(static_cast<std::uint8_t>(config.translationType) +
                                   static_cast<std::uint8_t>(config.rotationType) + static_cast<std::uint8_t>(config.scaleType));
}

void MLJointsBuilder::computeStorageRequirements() {
}

void MLJointsBuilder::computeStorageRequirements(const JointBehaviorFilter& source) {
    RL_UNUSED(source);
}

void MLJointsBuilder::allocateStorage(const JointBehaviorFilter& source) {
    RL_UNUSED(source);
}

void MLJointsBuilder::remapIndices(std::uint16_t lod) {
    auto remap = [this](ArrayView<std::uint16_t> indices) {
        for (auto& absAttrIndex : indices) {
            const auto jointIndex = static_cast<std::uint16_t>(absAttrIndex / inputJointAttrCount);
            const auto relAttrIndex = static_cast<std::uint16_t>(absAttrIndex % inputJointAttrCount);
            const auto newAttrBase = static_cast<std::uint16_t>(jointIndex * outputJointAttrCount);
            const auto delta = static_cast<std::int32_t>(outputJointAttrCount) - static_cast<std::int32_t>(inputJointAttrCount);
            // Only the relative attribute indices for scale are offset by one when output is in quaternions
            const auto newRelAttrIndex = (relAttrIndex < 6u ? relAttrIndex : static_cast<std::uint16_t>(delta + relAttrIndex));
            absAttrIndex = static_cast<std::uint16_t>(newAttrBase + newRelAttrIndex);
        }
    };
    remap(outputIndices[lod]);
    remap(outputRotationBaseIndices[lod]);

    // Map all qx, qy, qz, qw indices to qx, qx, qx, qx
    for (auto& rotationIndex : outputRotationBaseIndices[lod]) {
        rotationIndex = static_cast<std::uint16_t>((rotationIndex / outputJointAttrCount) * outputJointAttrCount +
                                                   static_cast<std::uint16_t>(mlTranslationType));
    }

    auto& inputRotationIndices = inputRotationBaseIndices[lod];
    auto& outputRotationIndices = outputRotationBaseIndices[lod];

    UnorderedSet<std::uint16_t> deduplicator{memRes};
    deduplicator.reserve(outputRotationIndices.size());
    for (std::size_t i = {}; i < outputRotationIndices.size();) {
        if (!deduplicator.insert(outputRotationIndices[i]).second) {
            outputRotationIndices.erase(extd::advanced(outputRotationIndices.begin(), i));
            inputRotationIndices.erase(extd::advanced(inputRotationIndices.begin(), i));
        } else {
            ++i;
        }
    }
}

void MLJointsBuilder::fillStorage(const JointBehaviorFilter& source) {
    isRestore = false;
    const auto reader = source.getReader();
    const auto lodCount = reader->getLODCount();
    // The rotation unit persists into the snapshot as a discriminator RigMetadata::validate() range-checks on restore, so
    // an out-of-range value here would make create() accept a rig whose own dump() restore() rejects. Normalize it.
    rotationUnit =
        (reader->getRotationUnit() == dna::RotationUnit::radians) ? dna::RotationUnit::radians : dna::RotationUnit::degrees;

    if (reader->getMLTypeCount() == 0) {
        return;
    }

    auto findMLRotationType = [this, reader]() {
        const auto paramKeys = reader->getMLJointsParameterKeys();
        const auto paramValues = reader->getMLJointsParameterValues();
        // Keys and values are independent DNA arrays; every parameter scan walks only the paired prefix.
        const auto paramCount = std::min(paramKeys.size(), paramValues.size());
        for (std::size_t i = {}; i < paramCount; ++i) {
            if (paramKeys[i] == static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationType)) {
                const auto rotationType = static_cast<dna::RotationRepresentation>(paramValues[i]);
                if (rotationType == dna::RotationRepresentation::EulerAngles) {
                    return RotationType::EulerAngles;
                } else if (rotationType == dna::RotationRepresentation::Quaternion) {
                    return RotationType::Quaternions;
                }
            }
        }
        return config.rotationType;
    };

    auto getCoordinateSystem = [reader]() {
        const auto paramKeys = reader->getMLJointsParameterKeys();
        const auto paramValues = reader->getMLJointsParameterValues();
        tdm::coord_sys coordSys = {};
        const auto paramCount = std::min(paramKeys.size(), paramValues.size());
        for (std::size_t i = {}; i < paramCount; ++i) {
            const auto xAxis = static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointCoordinateSystemAxisX);
            const auto yAxis = static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointCoordinateSystemAxisY);
            const auto zAxis = static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointCoordinateSystemAxisZ);
            if (paramKeys[i] == xAxis) {
                coordSys.x = static_cast<tdm::axis_dir>(paramValues[i]);
            } else if (paramKeys[i] == yAxis) {
                coordSys.y = static_cast<tdm::axis_dir>(paramValues[i]);
            } else if (paramKeys[i] == zAxis) {
                coordSys.z = static_cast<tdm::axis_dir>(paramValues[i]);
            }
        }
        return coordSys;
    };

    auto getRotationSigns = [reader]() {
        const auto paramKeys = reader->getMLJointsParameterKeys();
        const auto paramValues = reader->getMLJointsParameterValues();
        tdm::rot_sign rotSigns = {};
        const auto paramCount = std::min(paramKeys.size(), paramValues.size());
        for (std::size_t i = {}; i < paramCount; ++i) {
            const auto xAxis = static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSignAxisX);
            const auto yAxis = static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSignAxisY);
            const auto zAxis = static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSignAxisZ);
            if (paramKeys[i] == xAxis) {
                rotSigns.x = (paramValues[i] == 1 ? tdm::rot_dir::positive : tdm::rot_dir::negative);
            } else if (paramKeys[i] == yAxis) {
                rotSigns.y = (paramValues[i] == 1 ? tdm::rot_dir::positive : tdm::rot_dir::negative);
            } else if (paramKeys[i] == zAxis) {
                rotSigns.z = (paramValues[i] == 1 ? tdm::rot_dir::positive : tdm::rot_dir::negative);
            }
        }
        return rotSigns;
    };

    auto getRotationSequence = [reader]() {
        const auto paramKeys = reader->getMLJointsParameterKeys();
        const auto paramValues = reader->getMLJointsParameterValues();
        const auto paramCount = std::min(paramKeys.size(), paramValues.size());
        for (std::size_t i = {}; i < paramCount; ++i) {
            if (paramKeys[i] == static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSequence)) {
                return static_cast<tdm::rot_seq>(paramValues[i]);
            }
        }
        return tdm::rot_seq{};
    };

    const bool isCoordinateSystemSpecified = [reader]() {
        const auto paramKeys = reader->getMLJointsParameterKeys();
        const auto paramValues = reader->getMLJointsParameterValues();
        // A key in the unpaired tail has no value; certifying it would run the transform on zero-initialized defaults.
        const auto pairedKeys = paramKeys.first(std::min(paramKeys.size(), paramValues.size()));
        bool isCoordSysSpecified = true;
        const std::uint16_t expectedKeys[] = {
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointCoordinateSystemAxisX),
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointCoordinateSystemAxisY),
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointCoordinateSystemAxisZ),
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSignAxisX),
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSignAxisY),
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSignAxisZ),
            static_cast<std::uint16_t>(dna::MachineLearnedBehaviorParameterKey::JointRotationSequence)};
        for (std::size_t i = {}; i < sizeof(expectedKeys) / sizeof(*expectedKeys); ++i) {
            isCoordSysSpecified = isCoordSysSpecified && extd::contains(pairedKeys, expectedKeys[i]);
        }
        return isCoordSysSpecified;
    }();

    if (isCoordinateSystemSpecified) {
        changeOfBasis = tdm::change_of_basis<float>(getCoordinateSystem(), meta->coordinateSystem);
        srcSeq = getRotationSequence();
        srcSigns = getRotationSigns();
        dstSeq = meta->rotationSequence;
        dstSigns = meta->rotationSigns;
    }

    mlRotationType = findMLRotationType();
    inputJointAttrCount =
        static_cast<std::uint16_t>(static_cast<std::uint8_t>(mlTranslationType) + static_cast<std::uint8_t>(mlRotationType) +
                                   static_cast<std::uint8_t>(mlScaleType));

    inputIndices.resize(lodCount);
    outputIndices.resize(lodCount);
    inputRotationBaseIndices.resize(lodCount);
    outputRotationBaseIndices.resize(lodCount);
    uniqueTranslationBaseIndices.resize(lodCount);
    uniqueRotationBaseIndices.resize(lodCount);
    uniqueScaleBaseIndices.resize(lodCount);

    auto isBaseTranslationAttribute = [this](std::uint16_t outputIndex) {
        const auto relTranslationStartIndex = static_cast<std::uint16_t>(0);
        const auto relAttrIndex = outputIndex % inputJointAttrCount;
        return (relAttrIndex == relTranslationStartIndex);
    };

    auto isRotationAttribute = [this](std::uint16_t outputIndex) {
        const auto relRotationStartIndex = static_cast<std::uint16_t>(mlTranslationType);
        const auto relRotationEndIndex =
            static_cast<std::uint16_t>(relRotationStartIndex + static_cast<std::uint16_t>(mlRotationType));
        const auto relAttrIndex = outputIndex % inputJointAttrCount;
        return (relAttrIndex >= relRotationStartIndex) && (relAttrIndex < relRotationEndIndex);
    };

    auto isBaseScaleAttribute = [this](std::uint16_t outputIndex) {
        const auto relScaleStartIndex = static_cast<std::uint16_t>(static_cast<std::uint16_t>(mlTranslationType) +
                                                                   static_cast<std::uint16_t>(mlRotationType));
        const auto relAttrIndex = outputIndex % inputJointAttrCount;
        return (relAttrIndex == relScaleStartIndex);
    };

    auto isJointInLOD = [](std::uint16_t outputIndex, std::uint16_t attrCount, ConstArrayView<std::uint16_t> jointIndices) {
        const auto jointIndex = outputIndex / attrCount;
        return std::find(jointIndices.begin(), jointIndices.end(), jointIndex) != jointIndices.end();
    };

    auto deduplicate = [](Vector<std::uint16_t>& indices) {
        std::sort(indices.begin(), indices.end());
        indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    };

    for (std::uint16_t lod = {}; lod < lodCount; ++lod) {
        const auto jointIndices = reader->getJointIndicesForLOD(lod);
        const auto mlJointsInputIndices = reader->getMLJointsInputIndices();
        const auto mlJointsOutputIndices = reader->getMLJointsOutputIndices();

        inputIndices[lod].reserve(mlJointsInputIndices.size());
        outputIndices[lod].reserve(mlJointsOutputIndices.size());
        inputRotationBaseIndices[lod].reserve(mlJointsInputIndices.size());
        outputRotationBaseIndices[lod].reserve(mlJointsOutputIndices.size());
        uniqueTranslationBaseIndices[lod].reserve(mlJointsInputIndices.size());
        uniqueRotationBaseIndices[lod].reserve(mlJointsInputIndices.size());
        uniqueScaleBaseIndices[lod].reserve(mlJointsInputIndices.size());

        const std::size_t mlJointsCount = std::min(mlJointsInputIndices.size(), mlJointsOutputIndices.size());
        for (std::size_t mi = {}; mi < mlJointsCount; ++mi) {
            const auto outputControlIndex = mlJointsInputIndices[mi];
            const auto jointAttrIndex = mlJointsOutputIndices[mi];
            if (isJointInLOD(jointAttrIndex, inputJointAttrCount, jointIndices)) {
                if (isRotationAttribute(jointAttrIndex)) {
                    inputRotationBaseIndices[lod].push_back(outputControlIndex);
                    outputRotationBaseIndices[lod].push_back(jointAttrIndex);
                } else {
                    inputIndices[lod].push_back(outputControlIndex);
                    outputIndices[lod].push_back(jointAttrIndex);
                    if (isCoordinateSystemSpecified) {
                        if (isBaseTranslationAttribute(jointAttrIndex)) {
                            uniqueTranslationBaseIndices[lod].push_back(outputControlIndex);
                        } else if (isBaseScaleAttribute(jointAttrIndex)) {
                            uniqueScaleBaseIndices[lod].push_back(outputControlIndex);
                        }
                    }
                }
            }
        }

        remapIndices(lod);
        if (isCoordinateSystemSpecified) {
            uniqueRotationBaseIndices[lod] = inputRotationBaseIndices[lod];
            deduplicate(uniqueRotationBaseIndices[lod]);
            deduplicate(uniqueTranslationBaseIndices[lod]);
            deduplicate(uniqueScaleBaseIndices[lod]);
        }
    }
}

void MLJointsBuilder::registerControls(Controls* controls) {
    for (std::uint16_t lod = {}; lod < static_cast<std::uint16_t>(inputIndices.size()); ++lod) {
        controls->registerControls(lod, inputIndices[lod]);
        controls->registerControls(lod, inputRotationBaseIndices[lod]);
    }
}

struct MLJointsEvaluatorFactory {

    JointsEvaluator::Pointer operator()(RotationType targetRotationType,
                                        RotationType mlRotationType,
                                        tdm::rot_seq rotationSequence,
                                        dna::RotationUnit rotationUnit,
                                        bool isCoordSysTransformed,
                                        Matrix<std::uint16_t>&& inputIndices,
                                        Matrix<std::uint16_t>&& outputIndices,
                                        Matrix<std::uint16_t>&& inputRotationBaseIndices,
                                        Matrix<std::uint16_t>&& outputRotationBaseIndices,
                                        Matrix<std::uint16_t>&& uniqueTranslationBaseIndices,
                                        Matrix<std::uint16_t>&& uniqueRotationBaseIndices,
                                        Matrix<std::uint16_t>&& uniqueScaleBaseIndices,
                                        tdm::fmat3 changeOfBasis,
                                        tdm::rot_seq srcSeq,
                                        tdm::rot_sign srcSigns,
                                        tdm::rot_seq dstSeq,
                                        tdm::rot_sign dstSigns,
                                        MemoryResource* memRes) {

        // isCoordSysTransformed selects the transformer templates and is passed in, not recomputed from changeOfBasis/
        // srcSeq/srcSigns: on restore those are still at ctor defaults here (load() streams them in later).
        if (targetRotationType == mlRotationType) {
            if (isCoordSysTransformed) {
                if (targetRotationType == RotationType::EulerAngles) {
                    if (rotationUnit == dna::RotationUnit::degrees) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer,
                                                       EulerAnglesTransformer<tdm::fdeg>,
                                                       ScaleTransformer,
                                                       NoopAdapter>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              NoopAdapter{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<TranslationTransformer,
                                                       EulerAnglesTransformer<tdm::frad>,
                                                       ScaleTransformer,
                                                       NoopAdapter>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              NoopAdapter{dstSigns},
                                              nullptr);
                    }
                } else {
                    using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, NoopAdapter>;
                    auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                    return factory.create(std::move(inputIndices),
                                          std::move(outputIndices),
                                          std::move(inputRotationBaseIndices),
                                          std::move(outputRotationBaseIndices),
                                          std::move(uniqueTranslationBaseIndices),
                                          std::move(uniqueRotationBaseIndices),
                                          std::move(uniqueScaleBaseIndices),
                                          changeOfBasis,
                                          srcSeq,
                                          srcSigns,
                                          dstSeq,
                                          dstSigns,
                                          NoopAdapter{dstSigns},
                                          nullptr);
                }
            } else {
                using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, NoopAdapter>;
                auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                return factory.create(std::move(inputIndices),
                                      std::move(outputIndices),
                                      std::move(inputRotationBaseIndices),
                                      std::move(outputRotationBaseIndices),
                                      std::move(uniqueTranslationBaseIndices),
                                      std::move(uniqueRotationBaseIndices),
                                      std::move(uniqueScaleBaseIndices),
                                      changeOfBasis,
                                      srcSeq,
                                      srcSigns,
                                      dstSeq,
                                      dstSigns,
                                      NoopAdapter{dstSigns},
                                      nullptr);
            }
        }

#ifdef RL_BUILD_WITH_XYZ_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::xyz) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::xyz>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xyz>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::fdeg>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            } else {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::xyz>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::xyz>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::frad>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            }
        }
#endif  // RL_BUILD_WITH_XYZ_ROTATION_ORDER

#ifdef RL_BUILD_WITH_XZY_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::xzy) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::xzy>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::xzy>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::fdeg>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            } else {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::xzy>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::xzy>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::frad>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            }
        }
#endif  // RL_BUILD_WITH_XZY_ROTATION_ORDER

#ifdef RL_BUILD_WITH_YXZ_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::yxz) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::yxz>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::yxz>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::fdeg>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            } else {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::yxz>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::yxz>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::frad>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            }
        }
#endif  // RL_BUILD_WITH_YXZ_ROTATION_ORDER

#ifdef RL_BUILD_WITH_YZX_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::yzx) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::yzx>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::yzx>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::fdeg>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            } else {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::yzx>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::yzx>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::frad>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            }
        }
#endif  // RL_BUILD_WITH_YZX_ROTATION_ORDER

#ifdef RL_BUILD_WITH_ZXY_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::zxy) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::zxy>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::zxy>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::fdeg>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            } else {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::zxy>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::zxy>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::frad>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            }
        }
#endif  // RL_BUILD_WITH_ZXY_ROTATION_ORDER

#ifdef RL_BUILD_WITH_ZYX_ROTATION_ORDER
        if (rotationSequence == tdm::rot_seq::zyx) {
            if (rotationUnit == dna::RotationUnit::degrees) {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::fdeg, tdm::rot_seq::zyx>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::fdeg, tdm::rot_seq::zyx>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::fdeg>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            } else {
                if (targetRotationType == RotationType::EulerAngles) {
                    using Q2E = QuaternionsToEulerAngles<tdm::frad, tdm::rot_seq::zyx>;
                    if (isCoordSysTransformed) {
                        using MLJE = MLJointsEvaluator<TranslationTransformer, QuaternionTransformer, ScaleTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, Q2E>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              Q2E{dstSigns},
                                              nullptr);
                    }
                } else {
                    using E2Q = EulerAnglesToQuaternions<tdm::frad, tdm::rot_seq::zyx>;
                    if (isCoordSysTransformed) {
                        using MLJE =
                            MLJointsEvaluator<TranslationTransformer, EulerAnglesTransformer<tdm::frad>, ScaleTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    } else {
                        using MLJE = MLJointsEvaluator<NoopTransformer, NoopTransformer, NoopTransformer, E2Q>;
                        auto factory = UniqueInstance<MLJE, JointsEvaluator>::with(memRes);
                        return factory.create(std::move(inputIndices),
                                              std::move(outputIndices),
                                              std::move(inputRotationBaseIndices),
                                              std::move(outputRotationBaseIndices),
                                              std::move(uniqueTranslationBaseIndices),
                                              std::move(uniqueRotationBaseIndices),
                                              std::move(uniqueScaleBaseIndices),
                                              changeOfBasis,
                                              srcSeq,
                                              srcSigns,
                                              dstSeq,
                                              dstSigns,
                                              E2Q{dstSigns},
                                              nullptr);
                    }
                }
            }
        }
#endif  // RL_BUILD_WITH_ZYX_ROTATION_ORDER

        return nullptr;
    }
};

JointsEvaluator::Pointer MLJointsBuilder::build() {
    const auto targetRotationType = config.rotationType;
    // Auto until this builder writes it on the create path; the deserialized kind on restore.
    const EvaluatorType type = meta->evaluators.mlJoints;

    const bool isMLDataEmpty = [this]() {
        assert(inputIndices.size() == outputIndices.size());
        bool empty = true;
        for (std::size_t lod = {}; lod < inputIndices.size(); ++lod) {
            // fillStorage() routes rotation attributes into their own arrays, so a rotation-only rig leaves inputIndices/
            // outputIndices empty at every LOD; probing those two alone would discard its ML data as a Null evaluator.
            empty = empty && inputIndices[lod].empty() && outputIndices[lod].empty() && inputRotationBaseIndices[lod].empty() &&
                    outputRotationBaseIndices[lod].empty();
        }
        return empty;
    }();

    if ((type == EvaluatorType::Null) || ((type == EvaluatorType::Auto) && isMLDataEmpty)) {
        meta->evaluators.mlJoints = EvaluatorType::Null;
        return UniqueInstance<JointsNullEvaluator, JointsEvaluator>::with(memRes).create();
    }

    // The evaluator templates are selected from mlRotationType, rotationUnit and isCoordSysTransformed; the DNA path
    // derives them in fillStorage(), which restore never runs, so restore recovers them from this subsystem's record.
    bool isCoordSysTransformed = !(changeOfBasis == tdm::fmat3::identity() && srcSeq == dstSeq && srcSigns == dstSigns);
    if (isRestore) {
        // Range-validated by RigMetadata::validate() before any factory runs.
        mlRotationType = meta->mlJointsRotationType;
        rotationUnit = meta->mlJointsRotationUnit;
        isCoordSysTransformed = (meta->mlJointsCoordSysTransformed != 0u);
    }

    // Validate only on the DNA path (type == Auto): the restore shell's matrices are streamed and validated by
    // MLJointsEvaluator::load(). A null return fails the whole create(), unlike the JointsNullEvaluator below.
    // Spans must match the adapter the factory selects: NoopAdapter does quaternion math even for Euler/Euler, hence the
    // quaternion span; a converting adapter spans each side's own type; unique-rotation is Euler only when transformed.
    const bool sameRotationType = (mlRotationType == targetRotationType);
    const auto spanOf = [](RotationType rotation) { return static_cast<std::size_t>(rotation); };
    const std::size_t inputRotationSpan = sameRotationType ? spanOf(RotationType::Quaternions) : spanOf(mlRotationType);
    const std::size_t outputRotationSpan = sameRotationType ? spanOf(RotationType::Quaternions) : spanOf(targetRotationType);
    const std::size_t uniqueRotationSpan = (isCoordSysTransformed && (mlRotationType == RotationType::EulerAngles))
                                               ? spanOf(RotationType::EulerAngles)
                                               : spanOf(RotationType::Quaternions);
    if ((type == EvaluatorType::Auto) && !MLJointsValidator::validate(inputIndices,
                                                                      outputIndices,
                                                                      inputRotationBaseIndices,
                                                                      outputRotationBaseIndices,
                                                                      uniqueTranslationBaseIndices,
                                                                      uniqueRotationBaseIndices,
                                                                      uniqueScaleBaseIndices,
                                                                      inputRotationSpan,
                                                                      outputRotationSpan,
                                                                      uniqueRotationSpan,
                                                                      *meta)) {
        return nullptr;
    }

    auto evaluator = MLJointsEvaluatorFactory()(targetRotationType,
                                                mlRotationType,
                                                meta->rotationSequence,
                                                rotationUnit,
                                                isCoordSysTransformed,
                                                std::move(inputIndices),
                                                std::move(outputIndices),
                                                std::move(inputRotationBaseIndices),
                                                std::move(outputRotationBaseIndices),
                                                std::move(uniqueTranslationBaseIndices),
                                                std::move(uniqueRotationBaseIndices),
                                                std::move(uniqueScaleBaseIndices),
                                                changeOfBasis,
                                                srcSeq,
                                                srcSigns,
                                                dstSeq,
                                                dstSigns,
                                                memRes);

    if (evaluator == nullptr) {
        // Unsupported rotation configuration: a Null record carries no discriminator words (the reader keys off word count).
        meta->evaluators.mlJoints = EvaluatorType::Null;
        return UniqueInstance<JointsNullEvaluator, JointsEvaluator>::with(memRes).create();
    }

    // Persist the template-selecting discriminators so restore rebuilds the identical evaluator.
    meta->evaluators.mlJoints = EvaluatorType::Concrete;
    meta->mlJointsRotationType = mlRotationType;
    meta->mlJointsRotationUnit = rotationUnit;
    meta->mlJointsCoordSysTransformed = (isCoordSysTransformed ? 1u : 0u);
    return evaluator;
}

}  // namespace ml

}  // namespace rl4
