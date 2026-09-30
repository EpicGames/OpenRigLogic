// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/twistswing/TwistSwingSetup.h"
#include "riglogic/joints/cpu/twistswing/TwistSwingValidator.h"
#include "riglogic/riglogic/RigMetadata.h"

namespace twistswingvalidatortest {

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes, std::uint16_t controlCount, std::uint16_t jointAttributeCount) {
    rl4::RigMetadata meta{memRes};
    meta.rawControlCount = controlCount;  // controlInputCount = raw + psd + ml + rbf; the rest stay zero
    meta.jointAttributeCount = jointAttributeCount;
    return meta;
}

// One setup: swing + twist each with a 4-component input quad and one blend weight -> one output quad (4 indices).
rl4::TwistSwingSetup makeValidSetup(rl4::MemoryResource* memRes) {
    rl4::TwistSwingSetup setup{memRes};
    const std::uint16_t inputQuad[] = {0u, 1u, 2u, 3u};
    const std::uint16_t outputQuad[] = {0u, 1u, 2u, 3u};
    const float oneWeight[] = {1.0f};
    setup.swingInputIndices.assign(inputQuad, inputQuad + 4);
    setup.swingBlendWeights.assign(oneWeight, oneWeight + 1);
    setup.swingOutputIndices.assign(outputQuad, outputQuad + 4);
    setup.twistInputIndices.assign(inputQuad, inputQuad + 4);
    setup.twistBlendWeights.assign(oneWeight, oneWeight + 1);
    setup.twistOutputIndices.assign(outputQuad, outputQuad + 4);
    return setup;
}

rl4::Vector<rl4::TwistSwingSetup> makeValidSetups(rl4::MemoryResource* memRes) {
    rl4::Vector<rl4::TwistSwingSetup> setups{memRes};
    setups.push_back(makeValidSetup(memRes));
    return setups;
}

}  // namespace twistswingvalidatortest

TEST(TwistSwingValidatorTest, AcceptsWellFormedSetup) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 4u);
    ASSERT_TRUE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsBlendWeightsWithoutAnyInputQuad) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    // Twist blend weights with BOTH input quads absent would blend an uninitialized rotation (merged
    // setups may drop the twist quad, but only while the swing quad is present).
    setups[0].swingInputIndices.clear();
    setups[0].swingOutputIndices.clear();
    setups[0].swingBlendWeights.clear();
    setups[0].twistInputIndices.clear();
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 4u);
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsSwingInputQuadWrongSize) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    setups[0].swingInputIndices.pop_back();  // 3 components, not empty and not 4
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 4u);
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsSwingOutputCountNotFourPerWeight) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    setups[0].swingOutputIndices.pop_back();  // 3 output indices for 1 blend weight (expected 4)
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 4u);
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsInputIndexBeyondControlCount) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 3u, 4u);  // controlInputCount 3, but input index 3 is present
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsOutputIndexBeyondAttributeCount) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 3u);  // jointAttributeCount 3, but output index 3 is present
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsSwingTwistAxisOutOfRange) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    // calculate() indexes a 3-entry function-pointer table by this axis and calls through it; anything >= 3 is an
    // out-of-bounds read + indirect call. dna::TwistAxis is {X, Y, Z}; forge a value past the last enumerator.
    setups[0].swingTwistAxis = static_cast<dna::TwistAxis>(3);
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 4u);
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}

TEST(TwistSwingValidatorTest, RejectsTwistTwistAxisOutOfRange) {
    pma::AlignedMemoryResource memRes;
    auto setups = twistswingvalidatortest::makeValidSetups(&memRes);
    setups[0].twistTwistAxis = static_cast<dna::TwistAxis>(7);
    auto meta = twistswingvalidatortest::makeMetadata(&memRes, 4u, 4u);
    ASSERT_FALSE(rl4::TwistSwingValidator::validate(setups, meta));
}
