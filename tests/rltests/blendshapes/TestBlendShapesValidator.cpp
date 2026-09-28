// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/blendshapes/BlendShapesValidator.h"
#include "riglogic/riglogic/RigMetadata.h"

namespace blendshapesvalidatortest {

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes,
                              std::uint16_t lodCount,
                              std::uint16_t controlCount,
                              std::uint16_t blendShapeCount) {
    rl4::RigMetadata meta{memRes};
    meta.lodCount = lodCount;
    meta.rawControlCount = controlCount;  // controlInputCount = raw + psd + ml + rbf; the rest stay zero
    meta.blendShapeCount = blendShapeCount;
    return meta;
}

template<typename T, std::size_t N>
rl4::Vector<T> makeVector(const T (&values)[N], rl4::MemoryResource* memRes) {
    return rl4::Vector<T>{values, values + N, memRes};
}

}  // namespace blendshapesvalidatortest

TEST(BlendShapesValidatorTest, AcceptsWellFormedData) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};      // each <= both index arrays (2), count == lodCount (2)
    const std::uint16_t inputIndices[] = {0u, 1u};   // < controlCount (4)
    const std::uint16_t outputIndices[] = {0u, 1u};  // < blendShapeCount (2)
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_TRUE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}

TEST(BlendShapesValidatorTest, RejectsMoreLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u, 0u};  // 3 LODs > lodCount (2)
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}

TEST(BlendShapesValidatorTest, RejectsFewerLODsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    // 1 LOD < lodCount (2): calculate(lod=1) would read lods[1] out of bounds and use it as the scatter loop bound.
    const std::uint16_t lodValues[] = {2u};
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}

TEST(BlendShapesValidatorTest, RejectsLODBeyondInputIndexCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {3u, 1u};  // 3 > inputIndices.size() (2)
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 1u, 0u};
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}

TEST(BlendShapesValidatorTest, RejectsLODBeyondOutputIndexCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {3u, 1u};  // 3 > outputIndices.size() (2)
    const std::uint16_t inputIndices[] = {0u, 1u, 0u};
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}

TEST(BlendShapesValidatorTest, RejectsInputIndexBeyondControlCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};
    const std::uint16_t inputIndices[] = {0u, 4u};  // 4 >= controlInputCount (4)
    const std::uint16_t outputIndices[] = {0u, 1u};
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}

TEST(BlendShapesValidatorTest, RejectsOutputIndexBeyondBlendShapeCount) {
    pma::AlignedMemoryResource memRes;
    const std::uint16_t lodValues[] = {2u, 1u};
    const std::uint16_t inputIndices[] = {0u, 1u};
    const std::uint16_t outputIndices[] = {0u, 2u};  // 2 >= blendShapeCount (2)
    auto lods = blendshapesvalidatortest::makeVector(lodValues, &memRes);
    auto in = blendshapesvalidatortest::makeVector(inputIndices, &memRes);
    auto out = blendshapesvalidatortest::makeVector(outputIndices, &memRes);
    auto meta = blendshapesvalidatortest::makeMetadata(&memRes, 2u, 4u, 2u);
    ASSERT_FALSE(rl4::BlendShapesValidator::validate(lods, in, out, meta));
}
