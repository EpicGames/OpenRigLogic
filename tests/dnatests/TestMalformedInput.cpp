// Copyright Epic Games, Inc. All Rights Reserved.

#include "dnatests/Defs.h"
#include "dnatests/Fixturesv21.h"
#include "dnatests/Fixturesv28.h"

#include "dna/BinaryStreamReader.h"
#include "dna/BinaryStreamWriter.h"
#include "dna/StreamReader.h"
#include "dna/types/Aliases.h"

#include <pma/resources/AlignedMemoryResource.h>
#include <status/Provider.h>

namespace {

class MalformedInputTest : public ::testing::Test {
protected:
    void SetUp() override {
        sc::StatusProvider::reset();
    }

    void TearDown() override {
        sc::StatusProvider::reset();
    }
};

}  // namespace

TEST_F(MalformedInputTest, SignatureMismatchSetsError) {
    auto bytes = dna::RawV21::getBytes();
    // Corrupt the 3-byte "DNA" signature (bytes 0-2)
    bytes[0] = 0x00;
    bytes[1] = 0x00;
    bytes[2] = 0x00;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::SignatureMismatchError.code);
}

TEST_F(MalformedInputTest, UnsupportedGenerationOnV21FormatSetsInvalidDataError) {
    // Mutate a v21 fixture to have generation=99. Dispatcher falls through to v27 serializer
    // (IndexTable format), which misreads the SectionLookupTable as an IndexTable with
    // 0x27=39 entries, then seeks 624 bytes past EOF -> SeekError.
    // archive.isOk() == false catches this and maps it to InvalidDataError.
    auto bytes = dna::RawV21::getBytes();
    bytes[3] = 0x00;
    bytes[4] = 0x63;  // generation = 99

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, VersionMismatchSetsError) {
    // Minimal v22+ format DNA: signature + generation=99 (unsupported) + IndexTable(0 entries).
    // v22+ uses IndexTable; 0 entries means no section seeks, so the archive reads cleanly
    // and the version check is reached: supported() == (generation == 2) -> false.
    const std::vector<char> bytes = {
        '\x44',
        '\x4E',
        '\x41',  // "DNA" signature
        '\x00',
        '\x63',  // generation = 99 (unsupported)
        '\x00',
        '\x01',  // version = 1
        '\x00',
        '\x00',
        '\x00',
        '\x00'  // IndexTable: 0 entries
    };

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::VersionMismatchError.code);
}

TEST_F(MalformedInputTest, OversizedIndexTableCountSetsInvalidDataError) {
    // v22+ format, IndexTable entry count = 0xFFFFFFFF.
    // boundSize() fires before any allocation attempt: malformed=true -> InvalidDataError.
    const std::vector<char> bytes = {
        '\x44',
        '\x4E',
        '\x41',  // "DNA" signature
        '\x00',
        '\x02',  // generation = 2
        '\x00',
        '\x02',  // version = 2 (v22)
        '\xFF',
        '\xFF',
        '\xFF',
        '\xFF'  // IndexTable: 0xFFFFFFFF entries
    };

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, IndexTableEntryWithOffsetPastEOFSetsInvalidDataError) {
    // v22+ format, one IndexTable entry whose offset field = 0xDEADBEEF.
    // After reading the IndexTable, Layer::serialize calls proxy(index->offset) which
    // seeks to 0xDEADBEEF on a 27-byte stream -> SeekError -> archive.isOk()=false -> InvalidDataError.
    const std::vector<char> bytes = {
        '\x44',
        '\x4E',
        '\x41',  // "DNA" signature
        '\x00',
        '\x02',  // generation = 2
        '\x00',
        '\x02',  // version = 2 (v22)
        '\x00',
        '\x00',
        '\x00',
        '\x01',  // IndexTable: 1 entry
        // Index entry (id + version + offset + size, each uint32 big-endian):
        '\xDE',
        '\xAD',
        '\xBE',
        '\xEF',  // id
        '\x00',
        '\x00',
        '\x00',
        '\x00',  // entry version
        '\xDE',
        '\xAD',
        '\xBE',
        '\xEF',  // offset = 0xDEADBEEF (past EOF)
        '\x00',
        '\x00',
        '\x00',
        '\x04'  // size = 4
    };

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, OversizedContainerSetsInvalidDataError) {
    auto bytes = dna::RawV21::getBytes();
    // Replace the name string length field at the start of the descriptor (offset 0x27)
    // with 0xFFFFFFFF. Exceeds stream->size() -> boundSize() sets malformed=true.
    const std::size_t descriptorOffset = 0x27;
    bytes[descriptorOffset + 0] = '\xFF';
    bytes[descriptorOffset + 1] = '\xFF';
    bytes[descriptorOffset + 2] = '\xFF';
    bytes[descriptorOffset + 3] = '\xFF';

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, ExcessiveLODCountSetsInvalidDataError) {
    auto bytes = dna::RawV21::getBytes();
    // LOD Count field layout (from descriptor byte layout):
    //   header(39) + name_len(4) + name(4) + archetype(2) + gender(2) + age(2)
    //   + metadata_count(4) + 2x(key_len(4)+key(5)+val_len(4)+val(7))
    //   + translation_unit(2) + rotation_unit(2) + coord_x(2) + coord_y(2) + coord_z(2)
    //   = offset 107 (0x6B)
    // Setting LOD Count to 256 (0x0100) exceeds LODLimits::count() (33).
    const std::size_t lodCountOffset = 0x27 + 68;
    bytes[lodCountOffset + 0] = 0x01;
    bytes[lodCountOffset + 1] = 0x00;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, DescriptorPayloadWithOversizedMetadataCountSetsInvalidDataError) {
    // mutated = dna[:39] + payload + dna[39:]
    // where payload[0:4] = name_length=4, payload[4:8] = "AAAA",
    // and payload[8:256] = 0xDEADBEEF repeating.

    // The valid name passes the first string read. The metadata count field
    // (descriptor bytes 14-17 = 0xBEEFDEAD ~= 3.2B entries) triggers
    // boundSize() -> malformed=true -> InvalidDataError, with zero allocation.
    std::vector<char> payload(256, 0);
    payload[0] = 0x00;
    payload[1] = 0x00;
    payload[2] = 0x00;
    payload[3] = 0x04;
    payload[4] = 'A';
    payload[5] = 'A';
    payload[6] = 'A';
    payload[7] = 'A';
    for (std::size_t off = 8; off < 256; off += 4) {
        payload[off + 0] = '\xDE';
        payload[off + 1] = '\xAD';
        payload[off + 2] = '\xBE';
        payload[off + 3] = '\xEF';
    }

    auto original = dna::RawV21::getBytes();
    std::vector<char> mutated;
    mutated.insert(mutated.end(), original.begin(), original.begin() + 39);
    mutated.insert(mutated.end(), payload.begin(), payload.end());
    mutated.insert(mutated.end(), original.begin() + 39, original.end());

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(mutated.data(), mutated.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, MidFilePayloadInDefinitionSectionSetsInvalidDataError) {
    // Same as DescriptorPayloadWithOversizedMetadataCountSetsInvalidDataError but targeting the definition section
    // (offset 0x7E = 126), which is deeper into the file after the descriptor has been read cleanly.
    // payload[0:4] = 0xDEADBEEF is inserted as the first uint32 of the definition,
    // which is the joint-name LOD mapping count (~3.7B). boundSize() fires.
    std::vector<char> payload(256, 0);
    for (std::size_t off = 0; off < 256; off += 4) {
        payload[off + 0] = '\xDE';
        payload[off + 1] = '\xAD';
        payload[off + 2] = '\xBE';
        payload[off + 3] = '\xEF';
    }

    const std::size_t injectionOffset = 0x7E;  // start of definition section
    auto original = dna::RawV21::getBytes();
    std::vector<char> mutated;
    mutated.insert(mutated.end(), original.begin(), original.begin() + injectionOffset);
    mutated.insert(mutated.end(), payload.begin(), payload.end());
    mutated.insert(mutated.end(), original.begin() + injectionOffset, original.end());

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(mutated.data(), mutated.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

TEST_F(MalformedInputTest, EmptyStreamSetsError) {
    std::vector<char> bytes;
    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
}

TEST_F(MalformedInputTest, TruncatedHeaderSetsError) {
    auto bytes = dna::RawV21::getBytes();
    // Keep only the 7-byte signature+version prefix, omitting the section lookup table
    bytes.resize(7);
    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
}

TEST_F(MalformedInputTest, InvalidCoordinateSystemSetsError) {
    auto bytes = dna::RawV21::getBytes();
    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->write(bytes.data(), bytes.size());
    stream->seek(0);
    dna::Configuration config;
    config.coordinateSystem = {dna::Direction::left, dna::Direction::up, dna::Direction::left};
    config.coordinateSystemTransformPolicy = dna::CoordinateSystemTransformPolicy::Transform;
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get(), config);
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidConfigError.code);
}

// A joint group carrying fewer LOD bounds than the descriptor declares LODs: one row count per LOD is the contract
// (an empty array is the one tolerated exception - a group that is never evaluated), so this is rejected.
TEST_F(MalformedInputTest, ShortJointGroupLODArraySetsInvalidDataError) {
    const auto bytes = dna::RawV28::getBytes();
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->write(bytes.data(), bytes.size());
    source->seek(0);
    auto sourceReader = dna::makeScoped<dna::BinaryStreamReader>(source.get());
    sourceReader->read();
    ASSERT_TRUE(dna::Status::isOk());
    ASSERT_EQ(sourceReader->getLODCount(), 2u);
    ASSERT_EQ(sourceReader->getJointGroupLODs(0u).size(), 2u);

    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto writer = dna::makeScoped<dna::BinaryStreamWriter>(crafted.get());
    writer->setFrom(sourceReader.get());
    const std::uint16_t shortLODs[] = {sourceReader->getJointGroupLODs(0u)[0]};
    writer->setJointGroupLODs(0u, shortLODs, 1u);
    writer->write();
    ASSERT_TRUE(dna::Status::isOk());

    crafted->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(crafted.get());
    reader->read();

    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// A joint group whose per-group LOD array is empty, loaded with coordinate-system conversion enabled. The converter
// rebuilds the group's LOD array at the same length and used to assign its first entry unconditionally - a heap write
// past a zero-length array. The load must succeed and keep the array empty.
TEST_F(MalformedInputTest, EmptyJointGroupLODArraySurvivesCoordinateSystemConversion) {
    const auto bytes = dna::RawV28::getBytes();
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->write(bytes.data(), bytes.size());
    source->seek(0);
    auto sourceReader = dna::makeScoped<dna::BinaryStreamReader>(source.get());
    sourceReader->read();
    ASSERT_TRUE(dna::Status::isOk());
    ASSERT_EQ(sourceReader->getJointGroupLODs(0u).size(), 2u);
    ASSERT_EQ(sourceReader->getCoordinateSystem().x, dna::Direction::right);

    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto writer = dna::makeScoped<dna::BinaryStreamWriter>(crafted.get());
    writer->setFrom(sourceReader.get());
    const std::uint16_t noLODs[] = {0u};
    writer->setJointGroupLODs(0u, noLODs, 0u);
    writer->write();
    ASSERT_TRUE(dna::Status::isOk());

    crafted->seek(0);
    dna::Configuration config;
    config.coordinateSystem = {dna::Direction::left, dna::Direction::up, dna::Direction::front};  // differs from the fixture
    config.coordinateSystemTransformPolicy = dna::CoordinateSystemTransformPolicy::Transform;
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(crafted.get(), config);
    reader->read();

    ASSERT_TRUE(dna::Status::isOk());
    ASSERT_EQ(reader->getJointGroupLODs(0u).size(), 0u);

    // Control: the untouched fixture through the same conversion. The converter rebuilds every group's rows from its
    // joints, so the crafted group must come out with exactly the control's rows - only its LOD array differs.
    source->seek(0);
    auto convertedSource = dna::makeScoped<dna::BinaryStreamReader>(source.get(), config);
    convertedSource->read();
    ASSERT_TRUE(dna::Status::isOk());
    ASSERT_EQ(convertedSource->getCoordinateSystem().x, reader->getCoordinateSystem().x);
    ASSERT_EQ(convertedSource->getJointGroupLODs(0u).size(), 2u);
    const auto expectedRows = convertedSource->getJointGroupOutputIndices(0u);
    const auto craftedRows = reader->getJointGroupOutputIndices(0u);
    ASSERT_EQ(craftedRows.size(), expectedRows.size());
    for (std::size_t row = 0u; row < expectedRows.size(); ++row) {
        ASSERT_EQ(craftedRows[row], expectedRows[row]) << "row " << row;
    }
}

// An ML type with one more operation set than LOD mappings. The two arrays are indexed in lockstep per operation
// set; the constrained-LOD filter used to subscript the missing mapping (guarded by an assert only). The mismatch is
// rejected at deserialization for constrained and unconstrained reads alike.
TEST_F(MalformedInputTest, MLOperationSetWithoutLODMappingSetsInvalidDataError) {
    const auto bytes = dna::RawV28::getBytes();
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->write(bytes.data(), bytes.size());
    source->seek(0);
    auto sourceReader = dna::makeScoped<dna::BinaryStreamReader>(source.get());
    sourceReader->read();
    ASSERT_TRUE(dna::Status::isOk());
    ASSERT_EQ(sourceReader->getMLTypeCount(), 1u);
    const auto setCount = sourceReader->getMLOperationSetCount(0u);
    ASSERT_GT(setCount, 0u);

    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto writer = dna::makeScoped<dna::BinaryStreamWriter>(crafted.get());
    writer->setFrom(sourceReader.get());
    // Grows the operations matrix to setCount + 1 sets while lodMLOperationMappings keeps setCount entries.
    writer->setMLOperationType(0u, setCount, 0u, dna::MachineLearnedBehaviorOperationType::Gather);
    writer->write();
    ASSERT_TRUE(dna::Status::isOk());

    // Unconstrained read.
    crafted->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(crafted.get());
    reader->read();
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);

    // Constrained read (drop LOD 0 of the fixture's two), the path that indexed the missing mapping.
    crafted->seek(0);
    dna::Configuration config;
    config.maxLOD = 1u;
    auto constrainedReader = dna::makeScoped<dna::BinaryStreamReader>(crafted.get(), config);
    constrainedReader->read();
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

namespace {

// Loads the v28 fixture, lets `mutate` rewrite it through the public writer, then reads the result back (optionally
// with a reader configuration) and returns the reader for the caller's assertions.
template<typename TMutate>
pma::ScopedPtr<dna::BinaryStreamReader> rewriteV28(TMutate mutate,
                                                   trio::MemoryStream* crafted,
                                                   const dna::Configuration& config = dna::Configuration{}) {
    const auto bytes = dna::RawV28::getBytes();
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->write(bytes.data(), bytes.size());
    source->seek(0);
    auto sourceReader = dna::makeScoped<dna::BinaryStreamReader>(source.get());
    sourceReader->read();
    EXPECT_TRUE(dna::Status::isOk());

    auto writer = dna::makeScoped<dna::BinaryStreamWriter>(crafted);
    writer->setFrom(sourceReader.get());
    mutate(sourceReader.get(), writer.get());
    writer->write();
    EXPECT_TRUE(dna::Status::isOk());

    crafted->seek(0);
    auto reader = dna::makeScoped<dna::BinaryStreamReader>(crafted, config);
    reader->read();
    return reader;
}

}  // namespace

// Joint-group LOD row count one past the group's output rows (the original "lodSizes[0] = 5 with two output indices"
// case). The cache used to clamp it; a prefix past the rows is malformed and is now rejected at read time.
TEST_F(MalformedInputTest, JointGroupLODRowCountBeyondRowsSetsInvalidDataError) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* source, dna::BinaryStreamWriter* writer) {
            const auto rowCount = static_cast<std::uint16_t>(source->getJointGroupOutputIndices(0u).size());
            const std::uint16_t lods[] = {static_cast<std::uint16_t>(rowCount + 1u), 0u};
            writer->setJointGroupLODs(0u, lods, 2u);
        },
        crafted.get());
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// The matching valid shape: LOD 0 spanning exactly all rows still loads.
TEST_F(MalformedInputTest, JointGroupLODRowCountAtRowsLoads) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* source, dna::BinaryStreamWriter* writer) {
            const auto rowCount = static_cast<std::uint16_t>(source->getJointGroupOutputIndices(0u).size());
            const std::uint16_t lods[] = {rowCount, 0u};
            writer->setJointGroupLODs(0u, lods, 2u);
        },
        crafted.get());
    ASSERT_TRUE(dna::Status::isOk());
    ASSERT_EQ(reader->getJointGroupLODs(0u)[0], reader->getJointGroupOutputIndices(0u).size());
}

// A joint-group joint ID equal to the joint count (the original "jointCount = 1, jointIndices = {5}" case). It used to
// be skipped when populating the RBF joint map; it is malformed and is now rejected, RBF poses or not.
TEST_F(MalformedInputTest, JointGroupJointIndexBeyondJointCountSetsInvalidDataError) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* source, dna::BinaryStreamWriter* writer) {
            const std::uint16_t jointIndices[] = {source->getJointCount()};
            writer->setJointGroupJointIndices(0u, jointIndices, 1u);
        },
        crafted.get());
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// Non-empty ML joint outputs with no recognized attribute width (no parameter keys). The width is the divisor and
// modulus of the ML output remap; the case is rejected before either operation.
TEST_F(MalformedInputTest, MLJointOutputsWithZeroAttributeWidthSetsInvalidDataError) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* /*unused*/, dna::BinaryStreamWriter* writer) {
            const std::uint16_t none[] = {0u};
            writer->setMLJointsParameterKeys(none, 0u);
            writer->setMLJointsParameterValues(none, 0u);
            const std::uint16_t outputs[] = {3u};
            writer->setMLJointsInputIndices(outputs, 1u);
            writer->setMLJointsOutputIndices(outputs, 1u);
        },
        crafted.get());
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// A joint group whose coefficient array is shorter than rows x columns, loaded with coordinate-system conversion. The
// converter used to read coefficients for every declared row; it now converts only the rows the array covers, and the
// cache population that follows rejects the short matrix.
TEST_F(MalformedInputTest, ShortJointGroupValuesAreConvertedSafelyAndRejected) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    dna::Configuration config;
    config.coordinateSystem = {dna::Direction::left, dna::Direction::up, dna::Direction::front};
    config.coordinateSystemTransformPolicy = dna::CoordinateSystemTransformPolicy::Transform;
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* source, dna::BinaryStreamWriter* writer) {
            const auto values = source->getJointGroupValues(0u);
            ASSERT_GT(values.size(), 2u);
            writer->setJointGroupValues(0u, values.data(), static_cast<std::uint32_t>(values.size() / 2u));
        },
        crafted.get(),
        config);
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// ML joints parameter keys and values are parallel arrays; a mismatch used to be clamped to the shorter one.
TEST_F(MalformedInputTest, MLJointsParameterArrayLengthMismatchSetsInvalidDataError) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* /*unused*/, dna::BinaryStreamWriter* writer) {
            const std::uint16_t keys[] = {0u, 1u};
            const std::uint16_t values[] = {0u};
            writer->setMLJointsParameterKeys(keys, 2u);
            writer->setMLJointsParameterValues(values, 1u);
        },
        crafted.get());
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// Blend shape channel input and output indices are parallel arrays.
TEST_F(MalformedInputTest, BlendShapeChannelIndexArrayLengthMismatchSetsInvalidDataError) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* source, dna::BinaryStreamWriter* writer) {
            const auto inputs = source->getBlendShapeChannelInputIndices();
            ASSERT_GT(inputs.size(), 1u);
            writer->setBlendShapeChannelInputIndices(inputs.data(), static_cast<std::uint16_t>(inputs.size() - 1u));
        },
        crafted.get());
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}

// Animated map input and output indices are parallel arrays.
TEST_F(MalformedInputTest, AnimatedMapIndexArrayLengthMismatchSetsInvalidDataError) {
    auto crafted = pma::makeScoped<trio::MemoryStream>();
    auto reader = rewriteV28(
        [](const dna::BinaryStreamReader* source, dna::BinaryStreamWriter* writer) {
            const auto inputs = source->getAnimatedMapInputIndices();
            ASSERT_GT(inputs.size(), 1u);
            writer->setAnimatedMapInputIndices(inputs.data(), static_cast<std::uint16_t>(inputs.size() - 1u));
        },
        crafted.get());
    ASSERT_FALSE(dna::Status::isOk());
    ASSERT_EQ(dna::Status::get().code, dna::StreamReader::InvalidDataError.code);
}
