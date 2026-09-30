// Copyright Epic Games, Inc. All Rights Reserved.

#include "triotests/TestStreams.h"

TYPED_TEST(StreamTest, OpenExistingFileForRead) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, OpenExistingFileForWrite) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, OpenExistingFileForReadWrite) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, OpenNonExistingFileForRead) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_IS(TestFixture::TStream::OpenError);
}

TYPED_TEST(StreamTest, OpenNonExistingFileForWrite) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, OpenNonExistingFileForReadWrite) {
    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, OpenEmptyFile) {
    TestFixture::CreateTestFile();

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, OpenAlreadyOpenFile) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_IS(TestFixture::TStream::AlreadyOpenError);
}

TYPED_TEST(StreamTest, ReadSuccess) {
    static const char fixture[4ul] = {'t', 'e', 's', 't'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // Read all data
    char buffer[8ul] = {};
    ASSERT_EQ(stream->read(buffer, 4ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, fixture, 4ul);

    // End of stream reached
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, ReadMoreThanBytesAvailable) {
    static const char fixture[4ul] = {'t', 'e', 's', 't'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // Read all data
    char buffer[8ul] = {};
    ASSERT_EQ(stream->read(buffer, 8ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, fixture, 4ul);
}

TYPED_TEST(StreamTest, ReadFromEmptyFile) {
    TestFixture::CreateTestFile();

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, ReadFromUnopenedStream) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
}

TYPED_TEST(StreamTest, ReadFromWriteOnlyStream) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
}

TYPED_TEST(StreamTest, ReadIntoNullBuffer) {
    TestFixture::CreateTestFile();

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    char* destination = nullptr;
    ASSERT_EQ(stream->read(destination, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
}

TYPED_TEST(StreamTest, ReadIntoNullWritable) {
    TestFixture::CreateTestFile();

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    trio::Writable* destination = nullptr;
    ASSERT_EQ(stream->read(destination, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
}

TYPED_TEST(StreamTest, SeekSuccess) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 0ul);

    stream->seek(2ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 2ul);

    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 2ul), 2ul);
    ASSERT_EQ(stream->tell(), 4ul);

    const char expected[] = {'s', 't'};
    ASSERT_ELEMENTS_EQ(buffer, expected, 2ul);
}

TYPED_TEST(StreamTest, SeekToEOF) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 0ul);

    stream->seek(4ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 4ul);
}

TYPED_TEST(StreamTest, SeekOutOfBounds) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 0ul);

    stream->seek(5ul);
    ASSERT_STATUS_IS(TestFixture::TStream::SeekError);
    ASSERT_EQ(stream->tell(), 0ul);
}

TYPED_TEST(StreamTest, SeekUnopenedStream) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->seek(1ul);
    ASSERT_STATUS_IS(TestFixture::TStream::SeekError);
}

TYPED_TEST(StreamTest, WriteSuccess) {
    static const char fixture[] = {'t', 'e', 's', 't'};
    static const char expected[] = {'h', 'e', 'l', 'l', 'o'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // Write data
    ASSERT_EQ(stream->write(expected, 5ul), 5ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 5ul);
    ASSERT_EQ(stream->tell(), 5ul);
    stream->close();
    ASSERT_STATUS_OK();

    // Verify data
    TestFixture::CompareTestFile(expected, 5u);
}

TYPED_TEST(StreamTest, WriteIntoEmptyFile) {
    static const char fixture[] = {'t', 'e', 's', 't'};
    TestFixture::CreateTestFile();

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    ASSERT_EQ(stream->write(fixture, 4ul), 4ul);
    ASSERT_STATUS_OK();

    stream->close();
    ASSERT_STATUS_OK();

    // Verify data
    TestFixture::CompareTestFile(fixture, 4u);
}

TYPED_TEST(StreamTest, WriteIntoUnopenedStream) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    ASSERT_EQ(stream->write("test", 4ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
}

TYPED_TEST(StreamTest, WriteIntoReadOnlyStream) {
    TestFixture::CreateTestFile("test", 4ul);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    ASSERT_EQ(stream->write("hello", 5ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
}

TYPED_TEST(StreamTest, WriteFromNullBuffer) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    const char* source = nullptr;
    ASSERT_EQ(stream->write(source, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
}

TYPED_TEST(StreamTest, WriteFromNullReadable) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    trio::Readable* source = nullptr;
    ASSERT_EQ(stream->write(source, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
}

TYPED_TEST(StreamTest, ReadAfterClose) {
    TestFixture::CreateTestFile("test", 4ul);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    stream->close();
    ASSERT_STATUS_OK();

    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
}

TYPED_TEST(StreamTest, WriteAfterClose) {
    TestFixture::CreateTestFile("test", 4ul);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    stream->close();
    ASSERT_STATUS_OK();

    ASSERT_EQ(stream->write("hello", 5ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
}

TYPED_TEST(StreamTest, SeekAfterClose) {
    TestFixture::CreateTestFile("test", 4ul);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    stream->close();
    ASSERT_STATUS_OK();

    stream->seek(2ul);
    ASSERT_STATUS_IS(TestFixture::TStream::SeekError);
}

TYPED_TEST(StreamTest, ReadIntoWritableLargerThanInternalBuffer) {
    // Spans several 4 KiB chunks: the count reported must be the total, not that of the last chunk.
    // Letters only - CreateTestFile writes in text mode, so newline bytes would be translated on Windows.
    static constexpr std::size_t fixtureSize = 10000ul;
    std::vector<char> fixture(fixtureSize);
    for (std::size_t i = 0ul; i < fixtureSize; ++i) {
        fixture[i] = static_cast<char>('a' + static_cast<char>(i % 26ul));
    }
    TestFixture::CreateTestFile(fixture.data(), static_cast<std::streamsize>(fixtureSize));

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    auto destination = pma::makeScoped<trio::MemoryStream>();
    destination->open();
    ASSERT_EQ(stream->read(destination.get(), fixtureSize), fixtureSize);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), fixtureSize);
    ASSERT_EQ(destination->size(), fixtureSize);

    std::vector<char> result(fixtureSize);
    destination->seek(0ul);
    ASSERT_EQ(destination->read(result.data(), fixtureSize), fixtureSize);
    ASSERT_ELEMENTS_EQ(result, fixture, fixtureSize);
}

namespace {

// Accepts at most `capacity` bytes in total, then refuses - models a destination that stops consuming mid-transfer
class CappedWritable : public trio::Writable {
public:
    explicit CappedWritable(std::size_t capacity_) :
        capacity{capacity_} {
    }

    std::size_t write(const char* source, std::size_t size) override {
        const std::size_t accepted = std::min(size, capacity - data.size());
        data.insert(data.end(), source, source + accepted);
        return accepted;
    }

    std::size_t write(trio::Readable* source, std::size_t size) override {
        // Take from the source only what can be kept, so the returned count matches what the source gave up; its count is
        // not trusted beyond what it was handed
        const std::size_t room = std::min(size, capacity - data.size());
        std::vector<char> chunk(room + 1ul);
        return write(chunk.data(), std::min(source->read(chunk.data(), room), room));
    }

    std::vector<char> data;

private:
    std::size_t capacity;
};

// Takes at most `pieceSize` bytes per call but never refuses - models a destination that consumes in pieces
class PiecewiseWritable : public trio::Writable {
public:
    explicit PiecewiseWritable(std::size_t pieceSize_) :
        pieceSize{pieceSize_} {
    }

    std::size_t write(const char* source, std::size_t size) override {
        const std::size_t accepted = std::min(size, pieceSize);
        data.insert(data.end(), source, source + accepted);
        return accepted;
    }

    std::size_t write(trio::Readable* source, std::size_t size) override {
        // Take from the source only one piece, so the returned count matches what the source gave up; its count is not
        // trusted beyond what it was handed
        const std::size_t piece = std::min(size, pieceSize);
        std::vector<char> chunk(piece + 1ul);
        return write(chunk.data(), std::min(source->read(chunk.data(), piece), piece));
    }

    std::vector<char> data;

private:
    std::size_t pieceSize;
};

}  // namespace

namespace {

// Behaviour the typed suite cannot pin because the two stream types legitimately differ there; each test states its reason
class FileStreamTest : public ::testing::Test {
protected:
    static const char* GetTestFileName() {
        return "TRiO_Test_FileStreamOnly.3l";
    }

    void SetUp() override {
        std::remove(GetTestFileName());
        sc::StatusProvider::reset();
    }

    void TearDown() override {
        std::remove(GetTestFileName());
        sc::StatusProvider::reset();
    }
};

}  // namespace

TEST_F(FileStreamTest, WriteIsVisibleAfterFlush) {
    static const char expected[] = {'h', 'e', 'l', 'l', 'o'};

    auto stream = pma::makeScoped<trio::FileStream>(GetTestFileName(), trio::AccessMode::Write, trio::OpenMode::Binary);
    stream->open();
    ASSERT_EQ(stream->write(expected, 5ul), 5ul);
    ASSERT_STATUS_OK();

    // Before close, only a flush makes the bytes observable through another handle; the mapped stream opens without sharing
    stream->flush();
    ASSERT_STATUS_OK();
    {
        std::ifstream file{GetTestFileName(), std::ios_base::binary};
        char buffer[5ul] = {};
        file.read(buffer, 5);
        ASSERT_EQ(file.gcount(), 5);
        ASSERT_ELEMENTS_EQ(buffer, expected, 5ul);
    }

    stream->close();
    ASSERT_STATUS_OK();
}

TEST_F(FileStreamTest, TextModeBehavesAsBinary) {
    // No newline translation on any runtime: bytes written are the bytes on disk, and positions are byte offsets
    static const char raw[] = {'a', '\n', 'b', '\r', '\n', 'c'};
    {
        auto stream = pma::makeScoped<trio::FileStream>(GetTestFileName(), trio::AccessMode::Write, trio::OpenMode::Text);
        stream->open();
        ASSERT_EQ(stream->write(raw, 6ul), 6ul);
        ASSERT_EQ(stream->size(), 6ul);
        stream->close();
        ASSERT_STATUS_OK();
    }
    {
        std::ifstream file{GetTestFileName(), std::ios_base::binary | std::ios_base::ate};
        ASSERT_EQ(static_cast<std::size_t>(file.tellg()), 6ul);
    }

    auto stream = pma::makeScoped<trio::FileStream>(GetTestFileName(), trio::AccessMode::ReadWrite, trio::OpenMode::Text);
    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 6ul);
    char buffer[6ul] = {};
    ASSERT_EQ(stream->read(buffer, 6ul), 6ul);
    ASSERT_ELEMENTS_EQ(buffer, raw, 6ul);

    // Read then write at a tracked offset lands exactly there, which translation would have broken
    stream->seek(3ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->write("X", 1ul), 1ul);
    stream->close();
    ASSERT_STATUS_OK();
    static const char expected[] = {'a', '\n', 'b', 'X', '\n', 'c'};
    std::ifstream file{GetTestFileName(), std::ios_base::binary};
    char actual[6ul] = {};
    file.read(actual, 6);
    ASSERT_EQ(file.gcount(), 6);
    ASSERT_ELEMENTS_EQ(actual, expected, 6ul);
}

TYPED_TEST(StreamTest, FlushOnReadOnlyStreamIsHarmless) {
    TestFixture::CreateTestFile("test", 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    // Nothing to push out, before and after open
    stream->flush();
    ASSERT_STATUS_OK();
    stream->open();
    ASSERT_STATUS_OK();
    stream->flush();
    ASSERT_STATUS_OK();
}

TYPED_TEST(StreamTest, ReadIntoWritableThatConsumesInPieces) {
    static const char fixture[8ul] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    TestFixture::CreateTestFile(fixture, 8u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // A destination that takes less than offered per call, but never nothing, still receives everything
    PiecewiseWritable destination{3ul};
    ASSERT_EQ(stream->read(&destination, 8ul), 8ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 8ul);
    ASSERT_EQ(destination.data.size(), 8ul);
    ASSERT_ELEMENTS_EQ(destination.data, fixture, 8ul);
}

TYPED_TEST(StreamTest, ReadIntoWritableThatStopsConsuming) {
    static const char fixture[8ul] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    TestFixture::CreateTestFile(fixture, 8u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // Only the bytes the destination accepted count, and the shortfall is an error
    CappedWritable destination{3ul};
    ASSERT_EQ(stream->read(&destination, 8ul), 3ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
    ASSERT_EQ(stream->tell(), 3ul);
    ASSERT_EQ(destination.data.size(), 3ul);
    ASSERT_ELEMENTS_EQ(destination.data, fixture, 3ul);

    // The stream must resume right after what was delivered, not after what it had fetched internally
    sc::StatusProvider::reset();
    char buffer[5ul] = {};
    ASSERT_EQ(stream->read(buffer, 5ul), 5ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture + 3), 5ul);
}

TYPED_TEST(StreamTest, ReadIntoWritableMoreThanBytesAvailable) {
    static const char fixture[4ul] = {'t', 'e', 's', 't'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // Only bytes actually read may reach the destination
    auto destination = pma::makeScoped<trio::MemoryStream>();
    destination->open();
    ASSERT_EQ(stream->read(destination.get(), 8ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 4ul);
    ASSERT_EQ(destination->size(), 4ul);

    char buffer[8ul] = {};
    destination->seek(0ul);
    ASSERT_EQ(destination->read(buffer, 8ul), 4ul);
    ASSERT_ELEMENTS_EQ(buffer, fixture, 4ul);
}

TYPED_TEST(StreamTest, WriteFromReadableShorterThanRequested) {
    static const char fixture[4ul] = {'t', 'e', 's', 't'};
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->open();
    ASSERT_EQ(source->write(fixture, 4ul), 4ul);
    source->seek(0ul);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    // Only bytes the source produced may reach the file; the shortfall is an error, as the source stopped early
    ASSERT_EQ(stream->write(source.get(), 8ul), 4ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
    ASSERT_EQ(stream->tell(), 4ul);

    stream->close();
    TestFixture::CompareTestFile(fixture, 4u);
}

// FileStream only: the mmap stream grows the file to the requested size up front by design, so the length check cannot be typed
TEST_F(FileStreamTest, WriteFromReadableShorterThanRequestedDoesNotPadFile) {
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->open();
    ASSERT_EQ(source->write("test", 4ul), 4ul);
    source->seek(0ul);

    auto stream = pma::makeScoped<trio::FileStream>(GetTestFileName(), trio::AccessMode::Write, trio::OpenMode::Binary);
    stream->open();
    ASSERT_EQ(stream->write(source.get(), 8ul), 4ul);
    ASSERT_EQ(stream->size(), 4ul);
    stream->close();

    std::ifstream file{GetTestFileName(), std::ios_base::binary | std::ios_base::ate};
    ASSERT_EQ(static_cast<std::size_t>(file.tellg()), 4ul);
}

namespace {

// Claims more than it was handed - a hostile counterpart whose count must not be trusted
class OvershootingWritable : public trio::Writable {
public:
    std::size_t write(const char* source, std::size_t size) override {
        data.insert(data.end(), source, source + size);
        return size + 100ul;
    }

    std::size_t write(trio::Readable* /*unused*/, std::size_t /*unused*/) override {
        return 0ul;
    }

    std::vector<char> data;
};

class OvershootingReadable : public trio::Readable {
public:
    explicit OvershootingReadable(const char* data_, std::size_t size_) :
        data{data_},
        size{size_} {
    }

    std::size_t read(char* destination, std::size_t count) override {
        const std::size_t available = std::min(count, size - position);
        std::copy(data + position, data + position + available, destination);
        position += available;
        return available + 100ul;
    }

    std::size_t read(trio::Writable* /*unused*/, std::size_t /*unused*/) override {
        return 0ul;
    }

private:
    const char* data;
    std::size_t size;
    std::size_t position = 0ul;
};

// A missing file must not read as an empty one, so it reports the largest size instead
std::size_t FileLength(const char* path) {
    std::ifstream file{path, std::ios_base::binary | std::ios_base::ate};
    return file.is_open() ? static_cast<std::size_t>(file.tellg()) : static_cast<std::size_t>(-1);
}

}  // namespace

TYPED_TEST(StreamTest, ReadIntoWritableThatStopsInLaterChunk) {
    // The stop happens after the first internal 4 KiB chunk, so the rewind must span several chunks
    static constexpr std::size_t fixtureSize = 10000ul;
    std::vector<char> fixture(fixtureSize);
    for (std::size_t i = 0ul; i < fixtureSize; ++i) {
        fixture[i] = static_cast<char>('a' + static_cast<char>(i % 26ul));
    }
    TestFixture::CreateTestFile(fixture.data(), static_cast<std::streamsize>(fixtureSize));

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    stream->open();
    ASSERT_STATUS_OK();

    CappedWritable destination{5000ul};
    ASSERT_EQ(stream->read(&destination, fixtureSize), 5000ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
    ASSERT_EQ(stream->tell(), 5000ul);
    ASSERT_ELEMENTS_EQ(destination.data, fixture, 5000ul);

    sc::StatusProvider::reset();
    char buffer[10ul] = {};
    ASSERT_EQ(stream->read(buffer, 10ul), 10ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture.data() + 5000), 10ul);
}

TYPED_TEST(StreamTest, ReadIntoWritableThatAcceptsNothing) {
    static const char fixture[8ul] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    TestFixture::CreateTestFile(fixture, 8u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    stream->open();
    ASSERT_STATUS_OK();

    CappedWritable destination{0ul};
    ASSERT_EQ(stream->read(&destination, 8ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
    ASSERT_EQ(stream->tell(), 0ul);

    // Nothing was delivered, so nothing may have been consumed
    sc::StatusProvider::reset();
    char buffer[8ul] = {};
    ASSERT_EQ(stream->read(buffer, 8ul), 8ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, fixture, 8ul);
}

TYPED_TEST(StreamTest, ReadIntoWritableThatStopsOnShortFinalChunk) {
    static const char fixture[8ul] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    TestFixture::CreateTestFile(fixture, 8u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    stream->open();
    ASSERT_STATUS_OK();

    // The request exceeds the file, so the end is hit inside the chunk the destination refuses; the rewind must undo that too
    CappedWritable destination{3ul};
    ASSERT_EQ(stream->read(&destination, 100ul), 3ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
    ASSERT_EQ(stream->tell(), 3ul);

    sc::StatusProvider::reset();
    char buffer[10ul] = {};
    ASSERT_EQ(stream->read(buffer, 10ul), 5ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture + 3), 5ul);
}

TYPED_TEST(StreamTest, WriteFromReadableThatStopsThenRead) {
    static const char fixture[] = {'a', 'b', 'c', 'd', 'e', 'f'};
    TestFixture::CreateTestFile(fixture, 6u);

    auto source = pma::makeScoped<trio::MemoryStream>();
    source->open();
    ASSERT_EQ(source->write("test", 4ul), 4ul);
    source->seek(0ul);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    ASSERT_EQ(stream->write(source.get(), 8ul), 4ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);
    ASSERT_EQ(stream->tell(), 4ul);

    // The read after the short write continues from where the delivered bytes ended
    sc::StatusProvider::reset();
    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 2ul), 2ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture + 4), 2ul);

    stream->close();
    static const char expected[] = {'t', 'e', 's', 't', 'e', 'f'};
    TestFixture::CompareTestFile(expected, 6u);
}

TYPED_TEST(StreamTest, ZeroSizeTransfersAreNoOps) {
    static const char fixture[] = {'a', 'b', 'c', 'd'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    char buffer[4ul] = {};
    CappedWritable destination{10ul};
    auto source = pma::makeScoped<trio::MemoryStream>();
    source->open();
    ASSERT_EQ(stream->read(buffer, 0ul), 0ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->read(&destination, 0ul), 0ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->write(buffer, 0ul), 0ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->write(source.get(), 0ul), 0ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 0ul);
    ASSERT_EQ(stream->size(), 4ul);

    ASSERT_EQ(stream->read(buffer, 4ul), 4ul);
    ASSERT_ELEMENTS_EQ(buffer, fixture, 4ul);
    stream->close();
    TestFixture::CompareTestFile(fixture, 4u);
}

TYPED_TEST(StreamTest, OvershootingCounterpartsAreClamped) {
    static const char fixture[8ul] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    TestFixture::CreateTestFile(fixture, 8u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    // Counts are trusted only up to what was handed over, in both directions
    OvershootingWritable destination;
    ASSERT_EQ(stream->read(&destination, 8ul), 8ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 8ul);
    ASSERT_EQ(destination.data.size(), 8ul);

    static const char appended[] = {'W', 'X', 'Y', 'Z'};
    OvershootingReadable source{appended, 4ul};
    ASSERT_EQ(stream->write(&source, 4ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 12ul);
    ASSERT_EQ(stream->size(), 12ul);

    stream->close();
    static const char expected[] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a', 'W', 'X', 'Y', 'Z'};
    TestFixture::CompareTestFile(expected, 12u);
}

TYPED_TEST(StreamTest, WriteModeTruncatesExistingFile) {
    static const char fixture[] = {'a', 'b', 'c', 'd'};
    static const char written[] = {'x', 'y', 'z'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    ASSERT_EQ(stream->size(), 4ul);

    // Write-only opens like "w": the previous content is gone, not overlaid
    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 0ul);
    ASSERT_EQ(stream->tell(), 0ul);
    ASSERT_EQ(stream->write(written, 3ul), 3ul);
    stream->close();
    ASSERT_STATUS_OK();
    ASSERT_EQ(FileLength(TestFixture::GetTestFileName()), 3ul);
    TestFixture::CompareTestFile(written, 3u);

    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 0ul);
    stream->close();
    ASSERT_EQ(FileLength(TestFixture::GetTestFileName()), 0ul);
}

TYPED_TEST(StreamTest, SeekPastEndIsRefusedAndWritesContinueAtPosition) {
    static const char fixture[] = {'a', 'b', 'c', 'd'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(5ul);
    ASSERT_STATUS_IS(TestFixture::TStream::SeekError);
    ASSERT_EQ(stream->tell(), 0ul);

    // The refused seek left the position alone, so this overwrites the first byte; seeking to the end then appends
    sc::StatusProvider::reset();
    ASSERT_EQ(stream->write("Z", 1ul), 1ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 1ul);
    stream->seek(4ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->write("E", 1ul), 1ul);
    ASSERT_EQ(stream->size(), 5ul);

    stream->close();
    static const char expected[] = {'Z', 'b', 'c', 'd', 'E'};
    TestFixture::CompareTestFile(expected, 5u);
}

TYPED_TEST(StreamTest, WriteOnlyStreamRefusesReadsAfterSeek) {
    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Write);
    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->write("hello", 5ul), 5ul);

    stream->seek(0ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 0ul);

    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
    sc::StatusProvider::reset();
    CappedWritable destination{10ul};
    ASSERT_EQ(stream->read(&destination, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);

    // Refused reads leave the stream writable at the seeked position
    sc::StatusProvider::reset();
    ASSERT_EQ(stream->write("J", 1ul), 1ul);
    ASSERT_STATUS_OK();
    stream->close();
    static const char expected[] = {'J', 'e', 'l', 'l', 'o'};
    TestFixture::CompareTestFile(expected, 5u);
}

TYPED_TEST(StreamTest, CloseTwiceAndFlushAfterCloseAreHarmless) {
    static const char fixture[] = {'a', 'b', 'c', 'd'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_EQ(stream->write("Z", 1ul), 1ul);

    stream->close();
    ASSERT_STATUS_OK();
    stream->close();
    ASSERT_STATUS_OK();
    stream->flush();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 1ul);

    char buffer[1ul] = {};
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
    sc::StatusProvider::reset();
    ASSERT_EQ(stream->write("Z", 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::WriteError);

    static const char expected[] = {'Z', 'b', 'c', 'd'};
    TestFixture::CompareTestFile(expected, 4u);
}

TYPED_TEST(StreamTest, FlushBetweenReadsAndWritesKeepsPosition) {
    static const char fixture[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
    TestFixture::CreateTestFile(fixture, 10u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    char buffer[3ul] = {};
    ASSERT_EQ(stream->read(buffer, 3ul), 3ul);
    // A flush right after a read has nothing to push out and must not move the position
    stream->flush();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->read(buffer, 3ul), 3ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture + 3), 3ul);
    ASSERT_EQ(stream->write("Z", 1ul), 1ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 7ul);

    stream->close();
    ASSERT_STATUS_OK();
    static const char expected[] = {'a', 'b', 'c', 'd', 'e', 'f', 'Z', 'h', 'i', 'j'};
    TestFixture::CompareTestFile(expected, 10u);
}

TYPED_TEST(StreamTest, FlushAfterWriteThenReadThenWrite) {
    static const char fixture[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
    TestFixture::CreateTestFile(fixture, 10u);

    auto stream =
        StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    ASSERT_EQ(stream->write("QQ", 2ul), 2ul);
    // The flush pushes the pending output out; the following read must start right behind it
    stream->flush();
    ASSERT_STATUS_OK();
    char buffer[2ul] = {};
    ASSERT_EQ(stream->read(buffer, 2ul), 2ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture + 2), 2ul);
    ASSERT_EQ(stream->write("W", 1ul), 1ul);
    ASSERT_STATUS_OK();

    stream->close();
    ASSERT_STATUS_OK();
    static const char expected[] = {'Q', 'Q', 'c', 'd', 'W', 'f', 'g', 'h', 'i', 'j'};
    TestFixture::CompareTestFile(expected, 10u);
}

TYPED_TEST(StreamTest, OpenDirectoryIsRefused) {
#if !defined(_WIN32) && !defined(TRIO_FSTAT_AVAILABLE)
    // Without fstat only an mmap-backed stream refuses at open; the file stream, and the fallback mapped stream built on
    // it, report the error on the first transfer instead
    #if defined(TRIO_MMAP_AVAILABLE)
    if (std::is_same<typename TestFixture::TStream, trio::FileStream>::value) {
        GTEST_SKIP();
    }
    #else
    GTEST_SKIP();
    #endif
#endif
    // The working directory always exists and is never a regular file
    auto stream = StreamFactory<typename TestFixture::TStream>::create(".", trio::AccessMode::Read);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 0ul);

    stream->open();
    ASSERT_STATUS_IS(TestFixture::TStream::OpenError);

    char buffer[2ul] = {};
    sc::StatusProvider::reset();
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_IS(TestFixture::TStream::ReadError);
}

TYPED_TEST(StreamTest, SeekAfterReadingPastEnd) {
    static const char fixture[4ul] = {'t', 'e', 's', 't'};
    TestFixture::CreateTestFile(fixture, 4u);

    auto stream = StreamFactory<typename TestFixture::TStream>::create(TestFixture::GetTestFileName(), trio::AccessMode::Read);
    ASSERT_STATUS_OK();

    stream->open();
    ASSERT_STATUS_OK();

    char buffer[8ul] = {};
    ASSERT_EQ(stream->read(buffer, 8ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->read(buffer, 1ul), 0ul);
    ASSERT_STATUS_OK();

    // Running into the end must not pin the stream there
    stream->seek(1ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), 1ul);
    ASSERT_EQ(stream->read(buffer, 3ul), 3ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, (fixture + 1), 3ul);
}
