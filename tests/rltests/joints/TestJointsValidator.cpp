// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/JointsValidator.h"
#include "riglogic/riglogic/RigMetadata.h"

// Joints::load() deserializes all members raw, and getJointIndicesForLOD/getVariableAttributeIndices are reachable
// from the public API with an unclamped lod, so both matrices must cover the rig's LOD count in BOTH directions.

namespace jointsvalidatortest {

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes, std::uint16_t lodCount, std::uint16_t jointAttributeCount) {
    rl4::RigMetadata meta{memRes};
    meta.lodCount = lodCount;
    meta.jointAttributeCount = jointAttributeCount;
    return meta;
}

rl4::Matrix<std::uint16_t> makeIndexMatrix(rl4::MemoryResource* memRes, std::size_t lodCount) {
    rl4::Matrix<std::uint16_t> matrix{memRes};
    matrix.resize(lodCount);
    for (std::size_t lod = 0ul; lod < lodCount; ++lod) {
        matrix[lod].push_back(0u);
        matrix[lod].push_back(1u);
    }
    return matrix;
}

rl4::Vector<float> makeNeutralValues(rl4::MemoryResource* memRes, std::size_t count) {
    return rl4::Vector<float>{count, 0.0f, memRes};
}

}  // namespace jointsvalidatortest

TEST(JointsValidatorTest, AcceptsWellFormedData) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 8ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 8u);
    ASSERT_TRUE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

TEST(JointsValidatorTest, RejectsFewerAttributeLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 8ul);
    // 1 LOD < lodCount (2): getVariableAttributeIndices(1) would subscript out of bounds.
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 1ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 8u);
    ASSERT_FALSE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

TEST(JointsValidatorTest, RejectsFewerJointIndexLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 8ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    // 1 LOD < lodCount (2): getJointIndicesForLOD(1) would subscript out of bounds.
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 1ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 8u);
    ASSERT_FALSE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

TEST(JointsValidatorTest, RejectsMoreLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 8ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 3ul);  // 3 > lodCount (2)
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 3ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 8u);
    ASSERT_FALSE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

TEST(JointsValidatorTest, RejectsNeutralValuesShorterThanAttributeCount) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 7ul);  // < jointAttributeCount (8)
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 8u);
    ASSERT_FALSE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

// A zero-LOD Null-shell dump legitimately carries an empty neutral buffer alongside a non-zero
// jointAttributeCount; its matrices are forced empty by the lodCount equality, so nothing can index the
// missing buffer.
TEST(JointsValidatorTest, AcceptsEmptyNeutralValuesForDegenerateRig) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 0ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 0ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 0ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 0u, 18u);
    ASSERT_TRUE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

// With live LODs the exemption must NOT apply: create() always sizes the neutral buffer to cover the attribute count,
// so an empty buffer beside validated attribute indices (which point into it) only occurs in a hostile payload.
TEST(JointsValidatorTest, RejectsEmptyNeutralValuesWithLiveIndices) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 0ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 18u);
    ASSERT_FALSE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

// The loadJoints=false Null shell on a DNA that HAS joints: both matrices sized to lodCount with empty rows, neutral
// buffer empty, metadata keeping the DNA's real jointAttributeCount. dump() emits this shape; restore() must accept it.
TEST(JointsValidatorTest, AcceptsNullShellWhenJointsNotLoaded) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 0ul);
    rl4::Matrix<std::uint16_t> attrs{&memRes};
    attrs.resize(2ul);
    rl4::Matrix<std::uint16_t> joints{&memRes};
    joints.resize(2ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 18u);
    ASSERT_TRUE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, false));
}

// Over-wide is harmless: nothing derives a loop bound from the neutral buffer's size.
TEST(JointsValidatorTest, AcceptsNeutralValuesWiderThanAttributeCount) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 16ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 8u);
    ASSERT_TRUE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}

TEST(JointsValidatorTest, RejectsAttributeIndexBeyondAttributeCount) {
    pma::AlignedMemoryResource memRes;
    auto neutral = jointsvalidatortest::makeNeutralValues(&memRes, 2ul);
    auto attrs = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    auto joints = jointsvalidatortest::makeIndexMatrix(&memRes, 2ul);
    // attribute index 1 is fine, but shrink the buffer so jointAttributeCount == 1 makes it out of range
    auto meta = jointsvalidatortest::makeMetadata(&memRes, 2u, 1u);
    neutral.resize(1ul);
    ASSERT_FALSE(rl4::JointsValidator::validate(neutral, attrs, joints, meta, true));
}
