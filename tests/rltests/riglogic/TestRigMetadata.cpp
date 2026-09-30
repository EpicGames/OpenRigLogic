// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/dna/FakeReader.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <cstdint>

namespace {

class JointCountReader : public dna::FakeReader {
public:
    explicit JointCountReader(std::uint16_t jointCount_) :
        jointCount{jointCount_} {
    }

    ~JointCountReader();

    std::uint16_t getJointCount() const override {
        return jointCount;
    }

private:
    std::uint16_t jointCount;
};

JointCountReader::~JointCountReader() = default;

}  // namespace

// Joint output indices are uint16 attribute indices, so jointCount x attributes-per-joint is the format limit; it must be
// rejected before the count is narrowed, or the wrapped count becomes the bound every later validator trusts.
TEST(RigMetadataTest, RejectsJointCountBeyondUint16AttributeSpace) {
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.rotationType = rl4::RotationType::Quaternions;  // 3 + 4 + 3 = 10 attributes per joint

    JointCountReader atLimit{6553u};
    auto meta = rl4::RigMetadata::create(config, &atLimit, &memRes);
    ASSERT_NE(meta, nullptr);
    ASSERT_EQ(meta->jointAttributeCount, 65530u);

    JointCountReader beyondLimit{6554u};
    ASSERT_EQ(rl4::RigMetadata::create(config, &beyondLimit, &memRes), nullptr);
}

TEST(RigMetadataTest, EulerLayoutLimitIsNineAttributesPerJoint) {
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.rotationType = rl4::RotationType::EulerAngles;  // 3 + 3 + 3 = 9 attributes per joint

    JointCountReader atLimit{7281u};  // 65529
    auto meta = rl4::RigMetadata::create(config, &atLimit, &memRes);
    ASSERT_NE(meta, nullptr);
    ASSERT_EQ(meta->jointAttributeCount, 65529u);

    JointCountReader beyondLimit{7282u};  // 65538
    ASSERT_EQ(rl4::RigMetadata::create(config, &beyondLimit, &memRes), nullptr);
}
