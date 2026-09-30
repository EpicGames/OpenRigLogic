// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/joints/cpu/bpcm/Storage.h"
#include "riglogic/joints/cpu/quaternions/JointGroup.h"
#include "riglogic/riglogic/RigMetadata.h"

// The kernels advance their output cursor and value pointer in WHOLE blocks, overshooting an unaligned boundary by up
// to a block: a boundary passing a raw <= extent check can still run past the buffer, so validators bound the walk.

namespace storagewalktest {

using QTF128 = trimd::scalar::F128;            // 4 lanes: quat fullStep 32, halfStep 16
using QTF256 = trimd::fallback::T256<QTF128>;  // 8 lanes
using QTF512 = trimd::fallback::T512<QTF256>;

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes, std::uint16_t lodCount, std::uint16_t jointAttributeCount) {
    rl4::RigMetadata meta{memRes};
    meta.lodCount = lodCount;
    meta.jointAttributeCount = jointAttributeCount;
    // getControlInputCount() bounds every input index; the fixtures use input index 0.
    meta.rawControlCount = 4u;
    return meta;
}

rl4::JointGroup makeQuaternionGroup(rl4::MemoryResource* memRes,
                                    std::uint32_t colCount,
                                    std::uint32_t outputCount,
                                    std::uint32_t paddedRowCount,
                                    const rl4::LODRegion& region) {
    rl4::JointGroup group{memRes};
    group.colCount = colCount;
    group.rowCount = paddedRowCount;
    group.values.resize<float>(static_cast<std::size_t>(paddedRowCount) * colCount);
    group.inputIndices.assign(colCount, 0u);
    group.outputIndices.resize(outputCount);
    for (std::uint32_t row = 0u; row < outputCount; ++row) {
        group.outputIndices[row] = static_cast<std::uint16_t>(row);
    }
    group.lods.assign(1ul, region);
    return group;
}

bool validateQuaternion(const rl4::JointGroup& group, const rl4::RigMetadata& meta, rl4::MemoryResource* memRes) {
    rl4::Vector<rl4::JointGroup> groups{memRes};
    groups.push_back(group);
    return rl4::StorageValidator<float, QTF512, QTF256, QTF128>{}(groups, meta);
}

rl4::bpcm::JointStorage makeBPCMStorage(rl4::MemoryResource* memRes,
                                        std::uint32_t colCount,
                                        std::uint32_t paddedRowCount,
                                        const rl4::LODRegion& region) {
    rl4::bpcm::JointStorage storage{memRes};
    storage.values.resize<float>(static_cast<std::size_t>(paddedRowCount) * colCount);
    storage.inputIndices.assign(colCount, 0u);
    storage.outputIndices.assign(paddedRowCount, 0u);
    storage.lodRegions.assign(1ul, region);
    rl4::bpcm::JointGroup group{};
    group.valuesSize = paddedRowCount * colCount;
    group.colCount = colCount;
    group.rowCount = paddedRowCount;
    storage.jointGroups.push_back(group);
    return storage;
}

bool validateBPCM(const rl4::bpcm::JointStorage& storage, const rl4::RigMetadata& meta) {
    return rl4::bpcm::StorageValidator<float, QTF512, QTF256, QTF128>{}(storage, meta, rl4::RotationType::EulerAngles);
}

}  // namespace storagewalktest

TEST(QuaternionStorageWalkTest, AcceptsBuilderShapedRegions) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // 4 live rows padded to 16 (the builder's PadTo): the whole LOD runs in half blocks.
    const rl4::LODRegion region{rl4::ColumnLOD{1u}, rl4::RowLOD{4u, 16u, 32u, 16u}};
    const auto group = storagewalktest::makeQuaternionGroup(&memRes, 1u, 4u, 16u, region);
    ASSERT_TRUE(storagewalktest::validateQuaternion(group, meta, &memRes));
}

TEST(QuaternionStorageWalkTest, RejectsUnalignedLastFullBlockBoundary) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // sizePaddedToLastFullBlock = 1 passes every raw <= extent comparison, but the kernel's masked
    // full-block loop still executes a whole 32-row block, walking the value pointer to 32 > 16.
    rl4::LODRegion region{rl4::ColumnLOD{1u}, rl4::RowLOD{}};
    region.outputLODs.size = 16u;
    region.outputLODs.sizePaddedToLastFullBlock = 1u;
    region.outputLODs.sizePaddedToSecondLastFullBlock = 0u;
    const auto group = storagewalktest::makeQuaternionGroup(&memRes, 1u, 16u, 16u, region);
    ASSERT_FALSE(storagewalktest::validateQuaternion(group, meta, &memRes));
}

TEST(QuaternionStorageWalkTest, RejectsUnalignedSecondLastFullBlockBoundary) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // The first loop reads FULL blocks of outputIndices unmasked; an unaligned boundary of 1 makes it
    // read a whole 32-entry block from a 16-entry buffer.
    rl4::LODRegion region{rl4::ColumnLOD{1u}, rl4::RowLOD{}};
    region.outputLODs.size = 16u;
    region.outputLODs.sizePaddedToLastFullBlock = 0u;
    region.outputLODs.sizePaddedToSecondLastFullBlock = 1u;
    const auto group = storagewalktest::makeQuaternionGroup(&memRes, 1u, 16u, 16u, region);
    ASSERT_FALSE(storagewalktest::validateQuaternion(group, meta, &memRes));
}

TEST(QuaternionStorageWalkTest, RejectsMaskedBlockReadsBeyondLiveIndices) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // The walk itself fits the padded rows (64), but the masked loop iterates twice and its second
    // iteration reads live indices at cursor 32 from a 16-entry outputIndices buffer.
    rl4::LODRegion region{rl4::ColumnLOD{1u}, rl4::RowLOD{}};
    region.outputLODs.size = 4u;
    region.outputLODs.sizePaddedToLastFullBlock = 64u;
    region.outputLODs.sizePaddedToSecondLastFullBlock = 0u;
    const auto group = storagewalktest::makeQuaternionGroup(&memRes, 1u, 16u, 64u, region);
    ASSERT_FALSE(storagewalktest::validateQuaternion(group, meta, &memRes));
}

TEST(BPCMStorageWalkTest, AcceptsBuilderShapedRegions) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // 6 live rows padded to 8 (TF128 walk: full block 8, half block 4).
    const rl4::LODRegion region{rl4::ColumnLOD{1u}, rl4::RowLOD{6u, 8u, 8u, 4u}};
    const auto storage = storagewalktest::makeBPCMStorage(&memRes, 1u, 8u, region);
    ASSERT_TRUE(storagewalktest::validateBPCM(storage, meta));
}

TEST(BPCMStorageWalkTest, RejectsUnalignedColumnBoundary) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // sizeAlignedTo4 = 3 passes a raw <= colCount comparison, but the 4-column unroll steps a whole
    // 4-column group, reading inputIndices[3] and a fourth column of values from 3-column extents.
    rl4::LODRegion region{rl4::ColumnLOD{3u, 3u, 0u}, rl4::RowLOD{4u, 4u, 8u, 4u}};
    const auto storage = storagewalktest::makeBPCMStorage(&memRes, 3u, 4u, region);
    ASSERT_FALSE(storagewalktest::validateBPCM(storage, meta));
}

TEST(BPCMStorageWalkTest, RejectsUnalignedBlockBoundary) {
    pma::AlignedMemoryResource memRes;
    auto meta = storagewalktest::makeMetadata(&memRes, 1u, 64u);
    // Boundary 1 passes a raw <= rowCount comparison, but the kernel executes a whole 8-row block,
    // consuming 8 * colCount values from a 4 * colCount buffer.
    rl4::LODRegion region{rl4::ColumnLOD{1u}, rl4::RowLOD{}};
    region.outputLODs.size = 4u;
    region.outputLODs.sizePaddedToLastFullBlock = 1u;
    region.outputLODs.sizePaddedToSecondLastFullBlock = 0u;
    const auto storage = storagewalktest::makeBPCMStorage(&memRes, 1u, 4u, region);
    ASSERT_FALSE(storagewalktest::validateBPCM(storage, meta));
}
