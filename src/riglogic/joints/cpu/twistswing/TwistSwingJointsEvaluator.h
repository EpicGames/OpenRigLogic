// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/SerializationContext.h"
#include "riglogic/TypeDefs.h"
#include "riglogic/controls/ControlsInputInstance.h"
#include "riglogic/joints/JointsEvaluator.h"
#include "riglogic/joints/JointsOutputInstance.h"
#include "riglogic/joints/cpu/twistswing/TwistSwingSetup.h"
#include "riglogic/joints/cpu/twistswing/TwistSwingValidator.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <tdm/Computations.h>
#include <tdm/Quat.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstddef>
#include <cstdint>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

static inline tdm::fquat separateTwistComponentByAxisX(const tdm::fquat& input) {
    const tdm::fquat twist{input.x, 0.0f, 0.0f, input.w};
    return (twist.length2() == 0.0f) ? tdm::fquat{} : tdm::normalize(twist);
}

static inline tdm::fquat separateTwistComponentByAxisY(const tdm::fquat& input) {
    const tdm::fquat twist{0.0f, input.y, 0.0f, input.w};
    return (twist.length2() == 0.0f) ? tdm::fquat{} : tdm::normalize(twist);
}

static inline tdm::fquat separateTwistComponentByAxisZ(const tdm::fquat& input) {
    const tdm::fquat twist{0.0f, 0.0f, input.z, input.w};
    return (twist.length2() == 0.0f) ? tdm::fquat{} : tdm::normalize(twist);
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
class TwistSwingJointsEvaluator : public JointsEvaluator {
public:
    struct Accessor;
    friend Accessor;

public:
    explicit TwistSwingJointsEvaluator(Vector<TwistSwingSetup>&& setups_,
                                       TRotationAdapter&& rotationAdapter_,
                                       JointsOutputInstance::Factory instanceFactory_,
                                       MemoryResource* memRes);

    JointsOutputInstance::Pointer createInstance(MemoryResource* instanceMemRes) const override;
    std::uint32_t getJointDeltaValueCountForLOD(std::uint16_t lod) const override;
    void calculate(ControlsInputInstance* inputs, JointsOutputInstance* outputs, std::uint16_t lod) const override;
    void calculate(ControlsInputInstance* inputs,
                   JointsOutputInstance* outputs,
                   std::uint16_t lod,
                   std::uint16_t jointGroupIndex) const override;
    void load(BoundedInputArchive& archive) override;
    void save(terse::BinaryOutputArchive<BoundedIOStream>& archive) override;

private:
    Vector<TwistSwingSetup> setups;
    TRotationAdapter rotationAdapter;
    JointsOutputInstance::Factory instanceFactory;
};

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::TwistSwingJointsEvaluator(
    Vector<TwistSwingSetup>&& setups_,
    TRotationAdapter&& rotationAdapter_,
    JointsOutputInstance::Factory instanceFactory_,
    MemoryResource* /*unused*/) :
    setups{std::move(setups_)},
    rotationAdapter{std::move(rotationAdapter_)},
    instanceFactory{std::move(instanceFactory_)} {
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
JointsOutputInstance::Pointer
TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::createInstance(MemoryResource* instanceMemRes) const {

    return instanceFactory(instanceMemRes);
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
std::uint32_t TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::getJointDeltaValueCountForLOD(
    std::uint16_t /*unused*/) const {
    return {};
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
void TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::calculate(ControlsInputInstance* inputs,
                                                                                        JointsOutputInstance* outputs,
                                                                                        std::uint16_t /*unused*/) const {
    const auto inputBuffer = inputs->getInputBuffer();
    auto outputBuffer = outputs->getOutputBuffer();

    const tdm::fquat identity;

    static constexpr decltype(separateTwistComponentByAxisX)* separateTwistComponent[] = {separateTwistComponentByAxisX,
                                                                                          separateTwistComponentByAxisY,
                                                                                          separateTwistComponentByAxisZ};

    for (const auto& setup : setups) {
        tdm::fquat invTwist;

        // Both portions are populated only when twist and swing share input indices; otherwise just one.
        // A populated portion carries a full input quad; empty means not populated.
        if (!setup.swingInputIndices.empty()) {
            const tdm::fquat swingInput{inputBuffer[setup.swingInputIndices[0]],
                                        inputBuffer[setup.swingInputIndices[1]],
                                        inputBuffer[setup.swingInputIndices[2]],
                                        inputBuffer[setup.swingInputIndices[3]]};
            const tdm::fquat twist = separateTwistComponent[static_cast<std::size_t>(setup.swingTwistAxis)](swingInput);
            invTwist = tdm::inverse(twist);
            const tdm::fquat swing = swingInput * invTwist;
            const tdm::fquat invSwing = tdm::inverse(swing);

            const std::size_t swingCount = setup.swingBlendWeights.size();
            for (std::size_t si = {}; si < swingCount; ++si) {
                const float swingBlendWeight = setup.swingBlendWeights[si];
                const tdm::fquat invSwingFraction = tdm::slerp(invSwing, identity, swingBlendWeight);
                const tdm::fquat swingOutput = invTwist * invSwingFraction;
                const std::uint16_t* outputIndices = &setup.swingOutputIndices[si * 4ul];
                const float outbuf[] = {swingOutput.x, swingOutput.y, swingOutput.z, swingOutput.w};
                rotationAdapter.forward(outbuf, 1ul, 1ul, outputIndices, outputBuffer);
            }
        }

        if (!setup.twistInputIndices.empty() && setup.swingInputIndices.empty()) {
            // A merged setup (shared inputs) reuses the swing-derived twist; a twist-only setup computes it
            // from its own input (the "fromEnd" case).
            const tdm::fquat twistInput{inputBuffer[setup.twistInputIndices[0]],
                                        inputBuffer[setup.twistInputIndices[1]],
                                        inputBuffer[setup.twistInputIndices[2]],
                                        inputBuffer[setup.twistInputIndices[3]]};
            const tdm::fquat twist = separateTwistComponent[static_cast<std::size_t>(setup.twistTwistAxis)](twistInput);
            invTwist = twist;  // No actual inversion is needed here
        }

        const std::size_t twistCount = setup.twistBlendWeights.size();
        for (std::size_t ti = {}; ti < twistCount; ++ti) {
            const float twistBlendWeight = setup.twistBlendWeights[ti];
            const tdm::fquat twistOutput = tdm::slerp(invTwist, identity, twistBlendWeight);
            const float outbuf[] = {twistOutput.x, twistOutput.y, twistOutput.z, twistOutput.w};
            const std::uint16_t* outputIndices = &setup.twistOutputIndices[ti * 4ul];
            rotationAdapter.forward(outbuf, 1ul, 1ul, outputIndices, outputBuffer);
        }
    }
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
void TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::calculate(ControlsInputInstance* /*unused*/,
                                                                                        JointsOutputInstance* /*unused*/,
                                                                                        std::uint16_t /*unused*/,
                                                                                        std::uint16_t /*unused*/) const {
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
void TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::load(BoundedInputArchive& archive) {
    archive(setups);
    const SerializationContext* context = static_cast<SerializationContext*>(archive.getUserData());
    if (!TwistSwingValidator::validate(setups, *context->metadata)) {
        archive.markMalformed();
    }
}

template<typename TValue, typename TFVec256, typename TFVec128, class TRotationAdapter>
void TwistSwingJointsEvaluator<TValue, TFVec256, TFVec128, TRotationAdapter>::save(
    terse::BinaryOutputArchive<BoundedIOStream>& archive) {
    archive(setups);
}

}  // namespace rl4
