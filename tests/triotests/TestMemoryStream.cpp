// Copyright Epic Games, Inc. All Rights Reserved.

#include "triotests/Defs.h"

#include "status/Provider.h"
#include "trio/Concepts.h"
#include "trio/Stream.h"
#include "trio/streams/MemoryStream.h"

#include <pma/ScopedPtr.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace {

// Runs out of data before the requested size is met, which a Readable is free to do
class ShortReader : public trio::Readable {
public:
    ShortReader(const char* source_, std::size_t available_) :
        source{source_},
        available{available_} {
    }

    std::size_t read(char* destination, std::size_t size) override {
        const std::size_t produced = std::min(size, available);
        std::memcpy(destination, source, produced);
        source += produced;
        available -= produced;
        return produced;
    }

    std::size_t read(trio::Writable* destination, std::size_t size) override {
        const std::size_t offered = std::min(size, available);
        const std::size_t written = destination->write(source, offered);
        // available is unsigned, and both overloads size their next chunk from it - a destination claiming more than it was
        // offered would wrap it round and run either of them past the end of source
        EXPECT_LE(written, offered);
        const std::size_t produced = std::min(written, offered);
        source += produced;
        available -= produced;
        return produced;
    }

private:
    const char* source;
    std::size_t available;
};

class MemoryStreamMappableTest : public ::testing::Test {
protected:
    void SetUp() override {
        sc::StatusProvider::reset();
    }

    void TearDown() override {
        sc::StatusProvider::reset();
    }
};

TEST_F(MemoryStreamMappableTest, IsMappable) {
    auto stream = pma::makeScoped<trio::MemoryStream>();
    // Fails to compile if MemoryStream stops modelling the concept
    trio::Mappable* mappable = stream.get();
    ASSERT_NE(mappable, nullptr);
}

TEST_F(MemoryStreamMappableTest, DataOnEmptyStreamIsNull) {
    auto stream = pma::makeScoped<trio::MemoryStream>();
    ASSERT_EQ(stream->size(), 0ul);
    ASSERT_EQ(stream->mappedData(), nullptr);
}

TEST_F(MemoryStreamMappableTest, DataOnPreallocatedStreamIsZeroFilled) {
    static constexpr std::size_t initialSize = 4ul;
    auto stream = pma::makeScoped<trio::MemoryStream>(initialSize);
    ASSERT_EQ(stream->size(), initialSize);

    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    static const char expected[initialSize] = {};
    ASSERT_ELEMENTS_EQ(mapped, expected, initialSize);
}

TEST_F(MemoryStreamMappableTest, DataIsAvailableBeforeOpen) {
    // Unlike a memory mapped file, an in-memory stream is addressable without being opened first
    auto stream = pma::makeScoped<trio::MemoryStream>(2ul);
    ASSERT_NE(stream->mappedData(), nullptr);
}

TEST_F(MemoryStreamMappableTest, DataReflectsWrittenBytes) {
    static const char expected[] = {0x00, 0x01, 0x02, 0x03};
    static constexpr std::size_t expectedSize = 4ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(expected, expectedSize), expectedSize);
    ASSERT_TRUE(trio::Status::isOk());
    ASSERT_EQ(stream->size(), expectedSize);

    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, expectedSize);
}

TEST_F(MemoryStreamMappableTest, DataSpansWholeStreamRegardlessOfPosition) {
    static const char expected[] = {0x0a, 0x0b, 0x0c, 0x0d};
    static constexpr std::size_t expectedSize = 4ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(expected, expectedSize), expectedSize);
    stream->seek(3ul);
    ASSERT_EQ(stream->tell(), 3ul);

    // Seeking moves the stream position; the addressable region still starts at offset zero
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, expectedSize);
}

TEST_F(MemoryStreamMappableTest, ZeroLengthWriteOnEmptyStreamIsNoOp) {
    static const char source[] = {0x01};

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    // Must not subscript the empty buffer while computing the destination address
    ASSERT_EQ(stream->write(source, 0ul), 0ul);
    ASSERT_TRUE(trio::Status::isOk());
    ASSERT_EQ(stream->size(), 0ul);
    ASSERT_EQ(stream->tell(), 0ul);
    ASSERT_EQ(stream->mappedData(), nullptr);
}

TEST_F(MemoryStreamMappableTest, ZeroLengthWriteAtEndOfStreamIsNoOp) {
    static const char expected[] = {0x01, 0x02};
    static constexpr std::size_t expectedSize = 2ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(expected, expectedSize), expectedSize);
    ASSERT_EQ(stream->tell(), expectedSize);

    // position == size(), so the destination address is one-past-the-end
    ASSERT_EQ(stream->write(expected, 0ul), 0ul);
    ASSERT_TRUE(trio::Status::isOk());
    ASSERT_EQ(stream->size(), expectedSize);
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, expectedSize);
}

TEST_F(MemoryStreamMappableTest, DataStaysCorrectAcrossGrowingWrite) {
    static const char first[] = {0x01, 0x02};
    static const char second[] = {0x03, 0x04};
    static const char expected[] = {0x01, 0x02, 0x03, 0x04};

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(first, 2ul), 2ul);
    ASSERT_EQ(stream->write(second, 2ul), 2ul);
    ASSERT_EQ(stream->size(), 4ul);

    // The growing write may have reallocated, so mappedData() must be re-read rather than cached
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, 4ul);
}

TEST_F(MemoryStreamMappableTest, ShortWriteDoesNotInflateStream) {
    static const char source[] = {0x01, 0x02};
    ShortReader reader{source, 2ul};

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    // The buffer is grown for the whole request but only partly filled - the rest must not survive as zero-filled payload,
    // and a source that stops early is reported, as in the file streams
    ASSERT_EQ(stream->write(&reader, 8ul), 2ul);
    ASSERT_EQ(trio::Status::get(), trio::MemoryStream::WriteError);
    ASSERT_EQ(stream->size(), 2ul);
    ASSERT_EQ(stream->tell(), 2ul);
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, source, 2ul);
}

TEST_F(MemoryStreamMappableTest, ShortWriteMidStreamKeepsTrailingBytes) {
    static const char initial[] = {0x01, 0x02, 0x03, 0x04};
    static const char overwrite[] = {0x0f};
    static const char expected[] = {0x01, 0x02, 0x0f, 0x04};
    static constexpr std::size_t expectedSize = 4ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(initial, expectedSize), expectedSize);
    stream->seek(2ul);

    ShortReader reader{overwrite, 1ul};
    // Dropping the unfilled tail must not cut into what the stream already held, only into what this write grew it by
    ASSERT_EQ(stream->write(&reader, expectedSize), 1ul);
    ASSERT_EQ(trio::Status::get(), trio::MemoryStream::WriteError);
    ASSERT_EQ(stream->size(), expectedSize);
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, expectedSize);
}

namespace {

// Accepts at most `capacity` bytes in total, then refuses
class StallingWriter : public trio::Writable {
public:
    explicit StallingWriter(std::size_t capacity_) :
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

}  // namespace

TEST_F(MemoryStreamMappableTest, ReadIntoWritableThatStopsConsuming) {
    static const char expected[] = {0x01, 0x02, 0x03, 0x04};
    static constexpr std::size_t expectedSize = 4ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(expected, expectedSize), expectedSize);
    stream->seek(0ul);

    // Only what the destination accepted counts, the stall is reported, and the position stops where delivery stopped
    StallingWriter destination{3ul};
    ASSERT_EQ(stream->read(&destination, expectedSize), 3ul);
    ASSERT_EQ(trio::Status::get(), trio::MemoryStream::ReadError);
    ASSERT_EQ(stream->tell(), 3ul);
    ASSERT_EQ(destination.data.size(), 3ul);
    ASSERT_ELEMENTS_EQ(destination.data, expected, 3ul);
}

TEST_F(MemoryStreamMappableTest, FlushIsHarmless) {
    static const char expected[] = {0x01, 0x02};

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(expected, 2ul), 2ul);

    // Nothing is held back in memory, so flushing through the common stream interface changes nothing
    trio::BoundedIOStream* base = stream.get();
    base->flush();
    ASSERT_TRUE(trio::Status::isOk());
    ASSERT_EQ(stream->size(), 2ul);
    ASSERT_EQ(stream->tell(), 2ul);
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, 2ul);
}

TEST_F(MemoryStreamMappableTest, MappedRegionAlwaysSpansWholeStream) {
    static const char expected[] = {0x0a, 0x0b, 0x0c, 0x0d};
    static constexpr std::size_t expectedSize = 4ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->mappedOffset(), 0ul);
    ASSERT_EQ(stream->mappedSize(), 0ul);

    ASSERT_EQ(stream->write(expected, expectedSize), expectedSize);
    stream->seek(3ul);
    // The premise of everything below - the region assertions are position-independent, so a no-op seek would still pass them
    ASSERT_TRUE(trio::Status::isOk());
    ASSERT_EQ(stream->tell(), 3ul);

    // The buffer is contiguous, so unlike a memory mapped file the region covers the whole stream whatever the position is
    ASSERT_EQ(stream->mappedOffset(), 0ul);
    ASSERT_EQ(stream->mappedSize(), expectedSize);
    ASSERT_EQ(stream->mappedSize(), stream->size());
}

TEST_F(MemoryStreamMappableTest, DataSurvivesClose) {
    static const char expected[] = {0x07, 0x08};
    static constexpr std::size_t expectedSize = 2ul;

    auto stream = pma::makeScoped<trio::MemoryStream>();
    stream->open();
    ASSERT_EQ(stream->write(expected, expectedSize), expectedSize);
    stream->close();

    // close() only rewinds an in-memory stream, it does not release the buffer - both halves asserted below
    ASSERT_EQ(stream->tell(), 0ul);
    ASSERT_EQ(stream->size(), expectedSize);
    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, expected, expectedSize);
}

}  // namespace
