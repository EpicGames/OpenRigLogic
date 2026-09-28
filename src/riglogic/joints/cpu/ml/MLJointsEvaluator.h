// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/SerializationContext.h"
#include "riglogic/TypeDefs.h"
#include "riglogic/controls/ControlsInputInstance.h"
#include "riglogic/joints/JointsEvaluator.h"
#include "riglogic/joints/JointsOutputInstance.h"
#include "riglogic/joints/cpu/ml/CoordinateSystemTransformer.h"
#include "riglogic/joints/cpu/ml/MLJointsValidator.h"
#include "riglogic/joints/cpu/ml/RotationAdapters.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstdint>

namespace rl4 {

namespace ml {

// Rotation component span each adapter/transformer reads/writes (a RotationType's enumerator value is its component count).
template<class TRotationAdapter>
struct MLRotationAdapterSpans {
    // Default (NoopAdapter): quaternion in, quaternion out.
    static constexpr std::size_t input() {
        return static_cast<std::size_t>(RotationType::Quaternions);
    }
    static constexpr std::size_t output() {
        return static_cast<std::size_t>(RotationType::Quaternions);
    }
};

template<typename TAngle, tdm::rot_seq Order>
struct MLRotationAdapterSpans<EulerAnglesToQuaternions<TAngle, Order>> {
    static constexpr std::size_t input() {
        return static_cast<std::size_t>(RotationType::EulerAngles);
    }
    static constexpr std::size_t output() {
        return static_cast<std::size_t>(RotationType::Quaternions);
    }
};

template<typename TAngle, tdm::rot_seq Order>
struct MLRotationAdapterSpans<QuaternionsToEulerAngles<TAngle, Order>> {
    static constexpr std::size_t input() {
        return static_cast<std::size_t>(RotationType::Quaternions);
    }
    static constexpr std::size_t output() {
        return static_cast<std::size_t>(RotationType::EulerAngles);
    }
};

template<class TRotationTransformer>
struct MLRotationTransformerSpan {
    // Noop/Quaternion transformer spans a quaternion.
    static constexpr std::size_t value() {
        return static_cast<std::size_t>(RotationType::Quaternions);
    }
};

template<typename TAngle>
struct MLRotationTransformerSpan<EulerAnglesTransformer<TAngle>> {
    static constexpr std::size_t value() {
        return static_cast<std::size_t>(RotationType::EulerAngles);
    }
};

template<class TTranslationTransformer, class TRotationTransformer, class TScaleTransformer, class TRotationAdapter>
class MLJointsEvaluator : public JointsEvaluator {
public:
    struct Accessor;
    friend Accessor;

public:
    MLJointsEvaluator(Matrix<std::uint16_t>&& inputIndices_,
                      Matrix<std::uint16_t>&& outputIndices_,
                      Matrix<std::uint16_t>&& inputRotationBaseIndices_,
                      Matrix<std::uint16_t>&& outputRotationBaseIndices_,
                      Matrix<std::uint16_t>&& uniqueTranslationBaseIndices_,
                      Matrix<std::uint16_t>&& uniqueRotationBaseIndices_,
                      Matrix<std::uint16_t>&& uniqueScaleBaseIndices_,
                      const tdm::fmat3& changeOfBasis_,
                      tdm::rot_seq srcSeq_,
                      tdm::rot_sign srcSigns_,
                      tdm::rot_seq dstSeq_,
                      tdm::rot_sign dstSigns_,
                      TRotationAdapter&& rotationAdapter_,
                      JointsOutputInstance::Factory instanceFactory_) :
        inputIndices{std::move(inputIndices_)},
        outputIndices{std::move(outputIndices_)},
        inputRotationBaseIndices{std::move(inputRotationBaseIndices_)},
        outputRotationBaseIndices{std::move(outputRotationBaseIndices_)},
        uniqueTranslationBaseIndices{std::move(uniqueTranslationBaseIndices_)},
        uniqueRotationBaseIndices{std::move(uniqueRotationBaseIndices_)},
        uniqueScaleBaseIndices{std::move(uniqueScaleBaseIndices_)},
        changeOfBasis{std::move(changeOfBasis_)},
        srcSeq{std::move(srcSeq_)},
        srcSigns{std::move(srcSigns_)},
        dstSeq{std::move(dstSeq_)},
        dstSigns{std::move(dstSigns_)},
        rotationAdapter{std::move(rotationAdapter_)},
        instanceFactory{instanceFactory_} {
    }

    JointsOutputInstance::Pointer createInstance(MemoryResource* instanceMemRes) const override {
        return instanceFactory(instanceMemRes);
    }

    std::uint32_t getJointDeltaValueCountForLOD(std::uint16_t lod) const override {
        RL_UNUSED(lod);
        return {};
    }

    void calculate(ControlsInputInstance* inputs, JointsOutputInstance* outputs, std::uint16_t lod) const override {
        assert(lod < inputIndices.size());
        assert(lod < outputIndices.size());
        assert(lod < inputRotationBaseIndices.size());
        assert(lod < outputRotationBaseIndices.size());
        assert(lod < uniqueTranslationBaseIndices.size());
        assert(lod < uniqueRotationBaseIndices.size());
        assert(lod < uniqueScaleBaseIndices.size());
        const auto& inputIndicesForLOD = inputIndices[lod];
        const auto& outputIndicesForLOD = outputIndices[lod];
        const auto& inputRotationBaseIndicesForLOD = inputRotationBaseIndices[lod];
        const auto& outputRotationBaseIndicesForLOD = outputRotationBaseIndices[lod];
        const auto& uniqueTranslationBaseIndicesForLOD = uniqueTranslationBaseIndices[lod];
        const auto& uniqueRotationBaseIndicesForLOD = uniqueRotationBaseIndices[lod];
        const auto& uniqueScaleBaseIndicesForLOD = uniqueScaleBaseIndices[lod];
        auto inputBuffer = inputs->getInputBuffer();
        auto outputBuffer = outputs->getOutputBuffer();

        TTranslationTransformer::transform(inputBuffer,
                                           uniqueTranslationBaseIndicesForLOD,
                                           changeOfBasis,
                                           srcSeq,
                                           srcSigns,
                                           dstSeq,
                                           dstSigns);
        TRotationTransformer::transform(inputBuffer,
                                        uniqueRotationBaseIndicesForLOD,
                                        changeOfBasis,
                                        srcSeq,
                                        srcSigns,
                                        dstSeq,
                                        dstSigns);
        TScaleTransformer::transform(inputBuffer,
                                     uniqueScaleBaseIndicesForLOD,
                                     changeOfBasis,
                                     srcSeq,
                                     srcSigns,
                                     dstSeq,
                                     dstSigns);

        for (std::size_t i = {}; i < inputIndicesForLOD.size(); ++i) {
            outputBuffer[outputIndicesForLOD[i]] += inputBuffer[inputIndicesForLOD[i]];
        }

        rotationAdapter.adapt(inputBuffer, outputBuffer, inputRotationBaseIndicesForLOD, outputRotationBaseIndicesForLOD);
    }

    void calculate(ControlsInputInstance* inputs,
                   JointsOutputInstance* outputs,
                   std::uint16_t lod,
                   std::uint16_t jointGroupIndex) const override {
        // Assume all joints are in a single joint group in this case
        RL_UNUSED(jointGroupIndex);
        calculate(inputs, outputs, lod);
    }

    void load(BoundedInputArchive& archive) override {
        archive(inputIndices,
                outputIndices,
                inputRotationBaseIndices,
                outputRotationBaseIndices,
                uniqueTranslationBaseIndices,
                uniqueRotationBaseIndices,
                uniqueScaleBaseIndices,
                changeOfBasis,
                srcSeq,
                srcSigns,
                dstSeq,
                dstSigns);

        const SerializationContext* context = static_cast<SerializationContext*>(archive.getUserData());
        const RigMetadata& metadata = *context->metadata;
        if (!MLJointsValidator::validate(inputIndices,
                                         outputIndices,
                                         inputRotationBaseIndices,
                                         outputRotationBaseIndices,
                                         uniqueTranslationBaseIndices,
                                         uniqueRotationBaseIndices,
                                         uniqueScaleBaseIndices,
                                         MLRotationAdapterSpans<TRotationAdapter>::input(),
                                         MLRotationAdapterSpans<TRotationAdapter>::output(),
                                         MLRotationTransformerSpan<TRotationTransformer>::value(),
                                         metadata)) {
            archive.markMalformed();
        }
    }

    void save(terse::BinaryOutputArchive<BoundedIOStream>& archive) override {
        archive(inputIndices,
                outputIndices,
                inputRotationBaseIndices,
                outputRotationBaseIndices,
                uniqueTranslationBaseIndices,
                uniqueRotationBaseIndices,
                uniqueScaleBaseIndices,
                changeOfBasis,
                srcSeq,
                srcSigns,
                dstSeq,
                dstSigns);
    }

private:
    Matrix<std::uint16_t> inputIndices;
    Matrix<std::uint16_t> outputIndices;
    Matrix<std::uint16_t> inputRotationBaseIndices;
    Matrix<std::uint16_t> outputRotationBaseIndices;
    Matrix<std::uint16_t> uniqueTranslationBaseIndices;
    Matrix<std::uint16_t> uniqueRotationBaseIndices;
    Matrix<std::uint16_t> uniqueScaleBaseIndices;
    tdm::fmat3 changeOfBasis;
    tdm::rot_seq srcSeq;
    tdm::rot_sign srcSigns;
    tdm::rot_seq dstSeq;
    tdm::rot_sign dstSigns;
    TRotationAdapter rotationAdapter;
    JointsOutputInstance::Factory instanceFactory;
};

}  // namespace ml

}  // namespace rl4
