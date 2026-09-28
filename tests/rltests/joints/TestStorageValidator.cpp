// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/bpcm/JointGroup.h"
#include "riglogic/joints/cpu/bpcm/Storage.h"
#include "riglogic/joints/cpu/quaternions/JointGroup.h"
#include "riglogic/riglogic/RigMetadata.h"
#include "riglogic/system/simd/SIMD.h"

// The scalar variant is always compiled, so no SSE/AVX/NEON build is needed.

// Named so the file-local helpers do not collide with sibling validator tests' helpers in the CI jumbo build.
namespace storagevalidatortest {

using TestTF512 = trimd::fallback::T512<trimd::scalar::F256>;
using Validate = rl4::bpcm::StorageValidator<float, TestTF512, trimd::scalar::F256, trimd::scalar::F128>;
using ValidateQuat = rl4::StorageValidator<float, TestTF512, trimd::scalar::F256, trimd::scalar::F128>;

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes, std::uint16_t lodCount, std::uint16_t jointAttributeCount) {
    rl4::RigMetadata meta{memRes};
    meta.lodCount = lodCount;
    meta.jointAttributeCount = jointAttributeCount;
    // getControlInputCount() = raw + psd + ml + rbf bounds every input index; the valid fixtures use input index
    // values 0..1, so keep the raw section wide enough for them.
    meta.rawControlCount = 4u;
    return meta;
}

rl4::bpcm::JointStorage makeValidBPCMStorage(rl4::MemoryResource* memRes) {
    // 2 columns, 3 live rows padded to 4 (the TF128 half block), 1 LOD, no rotations. Builder-shaped on purpose: the
    // validator bounds the kernel's block-strided walk, which an unpadded row count or unaligned boundary would fail.
    rl4::bpcm::JointStorage storage{memRes};
    storage.values.fp32.resize(2ul * 4ul);  // colCount * paddedRowCount
    storage.inputIndices.resize(2ul);       // colCount
    storage.outputIndices.resize(4ul);      // paddedRowCount
    storage.outputIndices[0] = 0u;
    storage.outputIndices[1] = 1u;
    storage.outputIndices[2] = 2u;
    storage.outputIndices[3] = 3u;  // values < jointAttributeCount
    storage.outputRotationIndices.resize(0ul);
    storage.outputRotationLODs.resize(1ul);  // one rotation LOD, no indices
    storage.outputRotationLODs[0] = 0u;
    storage.lodRegions.resize(1ul);
    storage.lodRegions[0].inputLODs = rl4::ColumnLOD{2u, 0u, 0u};             // <= colCount
    storage.lodRegions[0].outputLODs = rl4::PaddedBlockView{3u, 4u, 8u, 4u};  // 3 live rows, padded to 4

    rl4::bpcm::JointGroup group{};
    group.valuesOffset = 0u;
    group.inputIndicesOffset = 0u;
    group.outputIndicesOffset = 0u;
    group.lodsOffset = 0u;
    group.outputRotationIndicesOffset = 0u;
    group.outputRotationLODsOffset = 0u;
    group.valuesSize = 8u;
    group.colCount = 2u;
    group.rowCount = 4u;
    storage.jointGroups.push_back(group);
    return storage;
}

rl4::Vector<rl4::JointGroup> makeValidQuaternionStorage(rl4::MemoryResource* memRes) {
    rl4::Vector<rl4::JointGroup> groups{memRes};
    rl4::JointGroup group{memRes};
    group.colCount = 2u;
    group.rowCount = 16u;             // 3 live rows padded to 16 (the quaternion builder PadTo)
    group.inputIndices.resize(2ul);   // colCount
    group.outputIndices.resize(3ul);  // live rows (quaternion outputIndices are NOT padded)
    group.outputIndices[0] = 0u;
    group.outputIndices[1] = 1u;
    group.outputIndices[2] = 2u;           // values < jointAttributeCount
    group.values.fp32.resize(2ul * 16ul);  // colCount * paddedRowCount
    group.lods.resize(1ul);
    group.lods[0].inputLODs = rl4::ColumnLOD{2u, 0u, 0u};
    group.lods[0].outputLODs = rl4::PaddedBlockView{3u, 16u, 32u, 16u};
    groups.push_back(std::move(group));
    return groups;
}

}  // namespace storagevalidatortest

TEST(BPCMStorageValidatorTest, AcceptsWellFormedStorage) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    ASSERT_TRUE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsOutOfBoundsValuesOffset) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    storage.jointGroups[0].valuesOffset = 1u;  // 1 + 2*4 > 8
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsOutOfBoundsInputIndicesOffset) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    storage.jointGroups[0].inputIndicesOffset = 1u;  // 1 + 2 > 2
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsOutOfBoundsOutputIndicesOffset) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    storage.jointGroups[0].outputIndicesOffset = 1u;  // 1 + 4 > 4
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsOutputIndexBeyondAttributeCount) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 2u);  // attribute count 2, but outputIndices has value 2
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsFewerLODRegionsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 2u, 8u);  // metadata claims 2 LODs, storage carries 1 per group
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsInputLODBeyondColumnCount) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    storage.lodRegions[0].inputLODs.size = 3u;  // > colCount (2)
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsRowsWithoutColumns) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    // colCount == 0 skips the values-extent check, so a non-zero rowCount must be rejected outright.
    // Input LODs are zeroed too, so this is the only violation present.
    storage.jointGroups[0].colCount = 0u;
    storage.lodRegions[0].inputLODs = rl4::ColumnLOD{0u};
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsOutputLODBeyondRowCount) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    // 5 rows would make the kernel masked loop execute a whole 8-row block, walking past the 4 padded rows.
    storage.lodRegions[0].outputLODs.sizePaddedToLastFullBlock = 5u;
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsRotationLODBeyondAvailableIndices) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    storage.outputRotationLODs[0] = 1u;  // claims 1 rotation index, but none are stored
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsEmptyRotationContainersForQuaternionRig) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    // A quaternion-output rig runs the E2Q adapter, which reads outputRotationLODs[lod] per group; a
    // crafted snapshot with EMPTY rotation containers passes every per-group bound (they all no-op on
    // empty) and the adapter dereferences an empty vector.
    storage.outputRotationLODs.resize(0ul);
    ASSERT_TRUE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::Quaternions));
}

TEST(QuaternionStorageValidatorTest, AcceptsWellFormedStorage) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    ASSERT_TRUE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsInputLODBeyondColumnCount) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    groups[0].lods[0].inputLODs.size = 3u;  // > colCount (2)
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsRowsWithoutColumns) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    // colCount == 0 sidesteps the division-form values-extent check, so a non-zero padded row count must
    // be rejected outright. Input LODs are zeroed too, so this is the only violation present.
    groups[0].colCount = 0u;
    groups[0].inputIndices.clear();
    groups[0].lods[0].inputLODs = rl4::ColumnLOD{0u};
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsOutputLODBeyondRowCount) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    groups[0].lods[0].outputLODs.size = 4u;  // > live outputIndices (3)
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsValueExtentBeyondValueBuffer) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    // colCount * sizePaddedToLastFullBlock must fit the value buffer; shrink the buffer below that product.
    groups[0].values.fp32.resize(2ul);  // 2 < colCount(2) * paddedRows(16)
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsOutputIndexBeyondAttributeCount) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 2u);  // attribute count 2, but outputIndices has value 2
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsFewerLODRegionsThanMetadata) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 2u, 8u);  // metadata claims 2 LODs, group owns 1
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(BPCMStorageValidatorTest, RejectsInputIndexBeyondControlInputCount) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);  // controlInputCount == 4 (raw)
    storage.inputIndices[0] = 4u;                                     // gathers inputs[4], out of the [0, 4) control buffer
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(BPCMStorageValidatorTest, RejectsRotationIndexLeavingNoRoomForQuaternion) {
    pma::AlignedMemoryResource memRes;
    auto storage = storagevalidatortest::makeValidBPCMStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);  // jointAttributeCount == 8
    // One reachable rotation index whose start + 3 quaternion components spills past the joint output buffer.
    storage.outputRotationIndices.resize(1ul);
    storage.outputRotationIndices[0] = 6u;  // 6 + 3 == 9 >= 8
    storage.outputRotationLODs[0] = 1u;     // makes that rotation row reachable
    ASSERT_FALSE(storagevalidatortest::Validate{}(storage, meta, rl4::RotationType::EulerAngles));
}

TEST(QuaternionStorageValidatorTest, RejectsInputIndexBeyondControlInputCount) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);  // controlInputCount == 4 (raw)
    groups[0].inputIndices[0] = 4u;                                   // gathers inputs[4], out of the [0, 4) control buffer
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsColCountDisagreeingWithInputIndices) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    groups[0].colCount = 3u;  // kernel would stride the value buffer by 3, but inputIndices holds only 2
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}

TEST(QuaternionStorageValidatorTest, RejectsPaddedRowCountBelowOutputCount) {
    pma::AlignedMemoryResource memRes;
    auto groups = storagevalidatortest::makeValidQuaternionStorage(&memRes);
    auto meta = storagevalidatortest::makeMetadata(&memRes, 1u, 8u);
    groups[0].rowCount = 2u;  // padded row count < outputIndices.size() (3): padding would drop a live row
    ASSERT_FALSE(storagevalidatortest::ValidateQuat{}(groups, meta));
}
