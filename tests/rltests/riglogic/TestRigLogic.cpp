// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/dna/DNAFixtures.h"

#include "riglogic/RigLogic.h"
#include "riglogic/TypeDefs.h"
#include "riglogic/utils/Extd.h"

#include <algorithm>

namespace {

class RigLogicTest : public ::testing::Test {
protected:
    void SetUp() override {
        const auto bytes = rltests::raw::getBytes();
        stream = pma::makeScoped<trio::MemoryStream>();
        stream->write(bytes.data(), bytes.size());
        stream->seek(0);

        reader = pma::makeScoped<dna::BinaryStreamReader>(stream.get());
        reader->read();

        rigLogic = pma::makeScoped<rl4::RigLogic>(reader.get());
        rigInstance = pma::makeScoped<rl4::RigInstance>(rigLogic.get(), &memRes);
    }

protected:
    pma::AlignedMemoryResource memRes;
    pma::ScopedPtr<trio::MemoryStream> stream;
    pma::ScopedPtr<dna::BinaryStreamReader> reader;
    pma::ScopedPtr<rl4::RigLogic> rigLogic;
    pma::ScopedPtr<rl4::RigInstance> rigInstance;
};

}  // namespace

TEST_F(RigLogicTest, EvaluateRigInstance) {
    const float guiControls[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    auto guiBuffer = rigInstance->getGUIControlValues();
    extd::copy(rl4::ConstArrayView<float>{guiControls, 9ul}, guiBuffer);
    rigLogic->mapGUIToRawControls(rigInstance.get());
    float rawControls[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    auto rawBuffer = rigInstance->getRawControlValues();
    extd::copy(rl4::ConstArrayView<float>{rawControls, 9ul}, rawBuffer);

    rigLogic->calculate(rigInstance.get());

    ASSERT_EQ(rigInstance->getJointOutputs().size(), reader->getJointRowCount());
    ASSERT_EQ(rigInstance->getBlendShapeOutputs().size(), reader->getBlendShapeChannelCount());
    ASSERT_EQ(rigInstance->getAnimatedMapOutputs().size(), reader->getAnimatedMapCount());
}

TEST_F(RigLogicTest, AccessJointVariableAttributeIndices) {
    for (std::uint16_t lod = 0u; lod < rigLogic->getLODCount(); ++lod) {
        auto actual = rigLogic->getJointVariableAttributeIndices(lod);
        auto expected = rl4::ConstArrayView<std::uint16_t>{rltests::decoded::jointVariableIndices[0ul][lod]};
        ASSERT_EQ(actual.size(), expected.size());
        // Element order is unspecified (built from a std::set, whose iteration order differs across compilers).
        for (const auto attrIndex : expected) {
            ASSERT_NE(std::find(actual.begin(), actual.end(), attrIndex), actual.end());
        }
    }
}

TEST_F(RigLogicTest, DumpStateThenRestore) {
    const float guiControls[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};

    auto dumpedState = pma::makeScoped<trio::MemoryStream>();
    rigLogic->dump(dumpedState.get());
    dumpedState->seek(0);
    auto cloneRigLogic = rl4::RigLogic::restore(dumpedState.get(), &memRes);
    auto cloneRigInstance = rl4::RigInstance::create(cloneRigLogic, &memRes);

    for (std::uint16_t lod = 0u; lod < rigLogic->getLODCount(); ++lod) {
        rigInstance->setLOD(lod);
        auto guiBuffer = rigInstance->getGUIControlValues();
        extd::copy(rl4::ConstArrayView<float>{guiControls, 9ul}, guiBuffer);
        rigLogic->mapGUIToRawControls(rigInstance.get());
        rigLogic->calculate(rigInstance.get());

        cloneRigInstance->setLOD(lod);
        auto cloneGuiBuffer = cloneRigInstance->getGUIControlValues();
        extd::copy(rl4::ConstArrayView<float>{guiControls, 9ul}, cloneGuiBuffer);
        cloneRigLogic->mapGUIToRawControls(cloneRigInstance);
        cloneRigLogic->calculate(cloneRigInstance);

        auto origJointOutputs = rigInstance->getJointOutputs();
        auto origBlendShapeOutputs = rigInstance->getBlendShapeOutputs();
        auto origAnimatedMapOutputs = rigInstance->getAnimatedMapOutputs();

        auto cloneJointOutputs = cloneRigInstance->getJointOutputs();
        auto cloneBlendShapeOutputs = cloneRigInstance->getBlendShapeOutputs();
        auto cloneAnimatedMapOutputs = cloneRigInstance->getAnimatedMapOutputs();

        ASSERT_EQ(origJointOutputs, cloneJointOutputs);
        ASSERT_EQ(origBlendShapeOutputs, cloneBlendShapeOutputs);
        ASSERT_EQ(origAnimatedMapOutputs, cloneAnimatedMapOutputs);
    }

    rl4::RigInstance::destroy(cloneRigInstance);
    rl4::RigLogic::destroy(cloneRigLogic);
}

TEST_F(RigLogicTest, RestoreRejectsEmptyStream) {
    auto emptyStream = pma::makeScoped<trio::MemoryStream>();
    emptyStream->seek(0);
    auto restored = rl4::RigLogic::restore(emptyStream.get(), &memRes);
    ASSERT_EQ(restored, nullptr);
}

TEST_F(RigLogicTest, RestoreRejectsGarbageStream) {
    auto garbageStream = pma::makeScoped<trio::MemoryStream>();
    const char garbage[] = {'n', 'o', 't', 'a', 's', 'n', 'a', 'p', 's', 'h', 'o', 't', '!', '!', '!', '!'};
    garbageStream->write(garbage, sizeof(garbage));
    garbageStream->seek(0);
    auto restored = rl4::RigLogic::restore(garbageStream.get(), &memRes);
    ASSERT_EQ(restored, nullptr);
}

TEST_F(RigLogicTest, RestoreRejectsCorruptMagic) {
    auto dumpedState = pma::makeScoped<trio::MemoryStream>();
    rigLogic->dump(dumpedState.get());
    dumpedState->seek(0);

    // bytes[0] lies in the 4-byte magic prefix.
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
    const std::size_t size = static_cast<std::size_t>(dumpedState->size());
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
    rl4::Vector<char> bytes(size, char{}, &memRes);
    dumpedState->read(bytes.data(), size);
    bytes[0] = static_cast<char>(bytes[0] ^ 0xFF);

    auto corruptStream = pma::makeScoped<trio::MemoryStream>();
    corruptStream->write(bytes.data(), bytes.size());
    corruptStream->seek(0);
    auto restored = rl4::RigLogic::restore(corruptStream.get(), &memRes);
    ASSERT_EQ(restored, nullptr);
}

TEST_F(RigLogicTest, RestoreRejectsVersionMismatch) {
    auto dumpedState = pma::makeScoped<trio::MemoryStream>();
    rigLogic->dump(dumpedState.get());
    dumpedState->seek(0);

    // Bytes 4-5 are the 2-byte major format version that follows the 4-byte magic.
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
    const std::size_t size = static_cast<std::size_t>(dumpedState->size());
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
    rl4::Vector<char> bytes(size, char{}, &memRes);
    dumpedState->read(bytes.data(), size);
    bytes[4] = static_cast<char>(bytes[4] ^ 0xFF);
    bytes[5] = static_cast<char>(bytes[5] ^ 0xFF);

    auto mismatchedStream = pma::makeScoped<trio::MemoryStream>();
    mismatchedStream->write(bytes.data(), bytes.size());
    mismatchedStream->seek(0);
    auto restored = rl4::RigLogic::restore(mismatchedStream.get(), &memRes);
    ASSERT_EQ(restored, nullptr);
}

TEST_F(RigLogicTest, RestoreRejectsOversizedLengthPrefix) {
    // A valid 8-byte header (magic + major + minor) followed by an absurdly large length prefix must be rejected by the
    // bounding archive instead of driving a multi-GB allocation.
    auto dumpedState = pma::makeScoped<trio::MemoryStream>();
    rigLogic->dump(dumpedState.get());
    dumpedState->seek(0);

    constexpr std::size_t headerSize = 8ul;
    char header[headerSize] = {};
    dumpedState->read(header, headerSize);

    auto poisonedStream = pma::makeScoped<trio::MemoryStream>();
    poisonedStream->write(header, headerSize);
    // The body must outlast the fixed-size configuration and metadata fields so an in-stream 0xFFFFFFFF length prefix
    // follows them (a past-EOF read clamps to zero). -1 is the 0xFF byte, spelled to avoid a signed-char truncation warning.
    const rl4::Vector<char> body(4096ul, static_cast<char>(-1), &memRes);
    poisonedStream->write(body.data(), body.size());
    poisonedStream->seek(0);
    auto restored = rl4::RigLogic::restore(poisonedStream.get(), &memRes);
    ASSERT_EQ(restored, nullptr);
}

TEST_F(RigLogicTest, RestoreRejectsInvalidConfigurationFlagByte) {
    // Configuration flags stream as single bytes and forming a bool from a byte other than 0/1 is UB, so the deserializer
    // must reject such a byte. The loadJoints byte is found without hardcoding the layout: two dumps differing only in
    // loadJoints first diverge exactly there.
    auto dumpedState = pma::makeScoped<trio::MemoryStream>();
    rigLogic->dump(dumpedState.get());
    dumpedState->seek(0);

    rl4::Configuration config{};
    config.loadJoints = false;
    auto noJointsRigLogic = pma::makeScoped<rl4::RigLogic>(reader.get(), config);
    auto noJointsState = pma::makeScoped<trio::MemoryStream>();
    noJointsRigLogic->dump(noJointsState.get());
    noJointsState->seek(0);

#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
    const std::size_t size = static_cast<std::size_t>(dumpedState->size());
    const std::size_t noJointsSize = static_cast<std::size_t>(noJointsState->size());
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
    rl4::Vector<char> bytes(size, char{}, &memRes);
    rl4::Vector<char> noJointsBytes(noJointsSize, char{}, &memRes);
    dumpedState->read(bytes.data(), size);
    noJointsState->read(noJointsBytes.data(), noJointsSize);

    std::size_t flagOffset = {};
    while ((flagOffset < std::min(size, noJointsSize)) && (bytes[flagOffset] == noJointsBytes[flagOffset])) {
        ++flagOffset;
    }
    ASSERT_LT(flagOffset, std::min(size, noJointsSize));

    bytes[flagOffset] = static_cast<char>(0x41);
    auto poisonedStream = pma::makeScoped<trio::MemoryStream>();
    poisonedStream->write(bytes.data(), bytes.size());
    poisonedStream->seek(0);
    auto restored = rl4::RigLogic::restore(poisonedStream.get(), &memRes);
    ASSERT_EQ(restored, nullptr);
}

TEST_F(RigLogicTest, DumpRestoreRoundTripAcrossLoadFlagCombinations) {
    // Every subsystem-skip combination must survive dump -> restore -> dump byte-identically: a disabled subsystem dumps
    // a Null shell, and restore() must accept this library's own product.
    for (std::uint8_t skips = 0u; skips < 64u; ++skips) {
        rl4::Configuration config{};
        config.loadJoints = (skips & 0x01u) == 0u;
        config.loadBlendShapes = (skips & 0x02u) == 0u;
        config.loadAnimatedMaps = (skips & 0x04u) == 0u;
        config.loadMachineLearnedBehavior = (skips & 0x08u) == 0u;
        config.loadRBFBehavior = (skips & 0x10u) == 0u;
        config.loadTwistSwingBehavior = (skips & 0x20u) == 0u;
        auto rig = pma::makeScoped<rl4::RigLogic>(reader.get(), config);
        ASSERT_NE(rig.get(), nullptr) << "create() rejected the fixture DNA for skips=" << static_cast<int>(skips);

        auto firstDump = pma::makeScoped<trio::MemoryStream>();
        rig->dump(firstDump.get());
        firstDump->seek(0);
        pma::ScopedPtr<rl4::RigLogic> restored{rl4::RigLogic::restore(firstDump.get(), &memRes)};
        ASSERT_NE(restored.get(), nullptr) << "restore() rejected our own dump for skips=" << static_cast<int>(skips);

        auto secondDump = pma::makeScoped<trio::MemoryStream>();
        restored->dump(secondDump.get());
        ASSERT_EQ(firstDump->size(), secondDump->size()) << "re-dump size differs for skips=" << static_cast<int>(skips);
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
        const std::size_t size = static_cast<std::size_t>(firstDump->size());
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
        rl4::Vector<char> first(size, char{}, &memRes);
        rl4::Vector<char> second(size, char{}, &memRes);
        firstDump->seek(0);
        firstDump->read(first.data(), size);
        secondDump->seek(0);
        secondDump->read(second.data(), size);
        ASSERT_EQ(first, second) << "re-dump bytes differ for skips=" << static_cast<int>(skips);
    }
}

TEST_F(RigLogicTest, CalculationStatsReportResolvedFloatingPointModel) {
    rl4::Stats stats{};
    rigLogic->collectCalculationStats(rigInstance.get(), &stats);
    ASSERT_EQ(stats.floatingPointModel, rl4::FloatingPointModel::Precise);

    // Stats reports the RESOLVED model: a Fast request downgrades to Precise unless the binary
    // was built with RL_BUILD_WITH_FAST.
    rl4::Configuration config{};
    config.floatingPointModel = rl4::FloatingPointModel::Fast;
    auto fastRigLogic = pma::makeScoped<rl4::RigLogic>(reader.get(), config);
    auto fastRigInstance = pma::makeScoped<rl4::RigInstance>(fastRigLogic.get(), &memRes);
    rl4::Stats fastStats{};
    fastRigLogic->collectCalculationStats(fastRigInstance.get(), &fastStats);
#ifdef RL_BUILD_WITH_FAST
    ASSERT_EQ(fastStats.floatingPointModel, rl4::FloatingPointModel::Fast);
#else
    ASSERT_EQ(fastStats.floatingPointModel, rl4::FloatingPointModel::Precise);
#endif  // RL_BUILD_WITH_FAST
}

TEST_F(RigLogicTest, JointOutputBufferInitialized) {
    rl4::Configuration config{};
    config.rotationType = rl4::RotationType::Quaternions;
    auto qRigLogic = pma::makeScoped<rl4::RigLogic>(reader.get(), config);
    auto qRigInstance = pma::makeScoped<rl4::RigInstance>(qRigLogic.get());
    auto jointOutputs = qRigInstance->getJointOutputs();
    static constexpr std::size_t qwOffset = 6;
    static constexpr std::size_t jointAttrCount = 10;
    for (std::size_t i = {}; i < jointOutputs.size(); ++i) {
        if (i % jointAttrCount == qwOffset) {
            ASSERT_EQ(jointOutputs[i], 1.0f);
        } else {
            ASSERT_EQ(jointOutputs[i], 0.0f);
        }
    }
}
