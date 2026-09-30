// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/animatedmaps/AnimatedMapsValidator.h"
#include "riglogic/conditionaltable/ConditionalTable.h"
#include "riglogic/riglogic/RigMetadata.h"

namespace animatedmapsvalidatortest {

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes,
                              std::uint16_t lodCount,
                              std::uint16_t controlCount,
                              std::uint16_t animatedMapCount) {
    rl4::RigMetadata meta{memRes};
    meta.lodCount = lodCount;
    meta.rawControlCount = controlCount;  // controlInputCount = raw + psd + ml + rbf; the rest stay zero
    meta.animatedMapCount = animatedMapCount;
    return meta;
}

template<typename T, std::size_t N>
rl4::Vector<T> makeVector(const T (&values)[N], rl4::MemoryResource* memRes) {
    return rl4::Vector<T>{values, values + N, memRes};
}

// The public ctor derives rowCount / interval-skip / range maps, so the table is always structurally valid.
rl4::ConditionalTable makeConditionalTable(rl4::MemoryResource* memRes, std::uint16_t inputCount, std::uint16_t outputCount) {
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    const float fromValues[] = {0.0f, 0.0f};
    const float toValues[] = {1.0f, 1.0f};
    const float slopeValues[] = {1.0f, 1.0f};
    const float cutValues[] = {0.0f, 0.0f};
    return rl4::ConditionalTable{makeVector(inputIndices, memRes),
                                 makeVector(outputIndices, memRes),
                                 makeVector(fromValues, memRes),
                                 makeVector(toValues, memRes),
                                 makeVector(slopeValues, memRes),
                                 makeVector(cutValues, memRes),
                                 inputCount,
                                 outputCount,
                                 memRes};
}

}  // namespace animatedmapsvalidatortest

TEST(AnimatedMapsValidatorTest, AcceptsWellFormedData) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};      // each <= outputIndices.size() (2), count <= lodCount (2)
    const std::uint16_t inputIndices[] = {0u, 1u};   // < controlCount (4)
    const std::uint16_t outputIndices[] = {0u, 1u};  // < animatedMapCount (2)
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_TRUE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RejectsMoreLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u, 0u};  // 3 LODs > lodCount (2); registerControls would subscript OOB
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RejectsFewerLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u};  // 1 LOD < lodCount (2); calculate(lod=1) would subscript lods[1] OOB
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RejectsLODBeyondOutputIndexCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {3u, 2u};  // count matches lodCount (2); 3 > outputIndices.size() (2) is the sole violation
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RejectsOutputCountBeyondAnimatedMapCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 0u};  // each < animatedMapCount, so only the COUNT check can reject
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 1u);  // animatedMapCount 1 < outputCount 2
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RejectsInputIndexBeyondControlCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};
    const std::uint16_t inputIndices[] = {0u, 1u};  // input index 1 present
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 1u, 2u);  // controlInputCount 1
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RejectsOutputIndexBeyondAnimatedMapCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 5u};  // 5 >= animatedMapCount, but outputCount (2) still fits
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto in = animatedmapsvalidatortest::makeVector(inputIndices, &memRes);
    auto out = animatedmapsvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, in, out, 2u, meta));
}

TEST(AnimatedMapsValidatorTest, RestoreAcceptsWellFormedTable) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};  // each <= outputIndices.size() (2), count <= lodCount (2)
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto conditionals = animatedmapsvalidatortest::makeConditionalTable(&memRes, 4u, 2u);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_TRUE(rl4::AnimatedMapsValidator::validate(lods, conditionals, meta));
}

TEST(AnimatedMapsValidatorTest, RestoreRejectsMoreLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u, 0u};  // 3 LODs > lodCount (2)
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto conditionals = animatedmapsvalidatortest::makeConditionalTable(&memRes, 4u, 2u);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, conditionals, meta));
}

TEST(AnimatedMapsValidatorTest, RestoreRejectsLODBeyondTableOutputIndexCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {
        3u,
        2u};  // count matches lodCount (2); 3 > table outputIndices.size() (2) is the sole violation
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto conditionals = animatedmapsvalidatortest::makeConditionalTable(&memRes, 4u, 2u);
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, conditionals, meta));
}

TEST(AnimatedMapsValidatorTest, RestoreRejectsTableInputIndexBeyondControlCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};
    auto lods = animatedmapsvalidatortest::makeVector(lodValues, &memRes);
    auto conditionals = animatedmapsvalidatortest::makeConditionalTable(&memRes, 4u, 2u);  // table input index 1 present
    auto meta = animatedmapsvalidatortest::makeMetadata(&memRes,
                                                        2u,
                                                        1u,
                                                        2u);  // controlInputCount 1: rejects via ConditionalTableValidator
    ASSERT_FALSE(rl4::AnimatedMapsValidator::validate(lods, conditionals, meta));
}
