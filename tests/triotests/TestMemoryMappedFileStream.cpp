// Copyright Epic Games, Inc. All Rights Reserved.

#include "triotests/TestStreams.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ios>
#include <limits>
#include <vector>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace {

// A multiple of both the page size and the Windows allocation granularity, so nothing is left to map at end of file - at any
// other size the trailing partial page keeps the mapping non-empty and hides the cases below
constexpr std::size_t alignedFileSize = 65536ul;
constexpr char fillByte = 0x5a;

class MemoryMappedFileStreamTest : public ::testing::Test {
protected:
    static const char* GetTestFileName() {
        return "TRiO_Test_MemoryMappedFileName.3l";
    }

    // At any other size the conditions under test are not reachable, so the size is asserted rather than assumed. Call through
    // ASSERT_NO_FATAL_FAILURE.
    static void CreateAlignedTestFile() {
        {
            std::ofstream file{GetTestFileName(), std::ios::binary};
            ASSERT_TRUE(file.is_open());
            const std::vector<char> data(alignedFileSize, fillByte);
            file.write(data.data(), static_cast<std::streamsize>(data.size()));
            ASSERT_TRUE(file.good());
        }

        std::ifstream written{GetTestFileName(), std::ios::binary | std::ios::ate};
        ASSERT_TRUE(written.is_open());
        ASSERT_EQ(written.tellg(), static_cast<std::streamoff>(alignedFileSize));
    }

    // Empties the file behind a closed stream's back - resize() would follow the position down, which is the very state the
    // reopen has to be tested against inheriting. Call through ASSERT_NO_FATAL_FAILURE.
    static void EmptyTestFile() {
        {
            std::ofstream file{GetTestFileName(), std::ios::binary | std::ios::trunc};
            ASSERT_TRUE(file.is_open());
        }

        std::ifstream emptied{GetTestFileName(), std::ios::binary | std::ios::ate};
        ASSERT_TRUE(emptied.is_open());
        ASSERT_EQ(emptied.tellg(), std::streamoff{0});
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

TEST_F(MemoryMappedFileStreamTest, SeekToEndOfAlignedFileThenBackToStart) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), alignedFileSize);

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->tell(), alignedFileSize);

    // A backend holds no view at end of file, which must not stop a seek back into it - ungated, as this has to work either way
    stream->seek(0ul);
    ASSERT_STATUS_OK();

    static const char expected[] = {fillByte, fillByte, fillByte, fillByte};
    char buffer[4] = {};
    ASSERT_EQ(stream->read(buffer, 4ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, expected, 4ul);
}

TEST_F(MemoryMappedFileStreamTest, AppendAtEndOfAlignedFileSucceeds) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();

    // A backend holds no view at end of file, which must not stop an append - ungated, as this has to work either way
    static const char appended[] = {0x01, 0x02};
    ASSERT_EQ(stream->write(appended, 2ul), 2ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), alignedFileSize + 2ul);

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();
    char buffer[2] = {};
    ASSERT_EQ(stream->read(buffer, 2ul), 2ul);
    ASSERT_ELEMENTS_EQ(buffer, appended, 2ul);
}

TEST_F(MemoryMappedFileStreamTest, ReadAtEndOfAlignedFileReturnsZeroWithoutError) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::Read);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();

    // Reading at end of file is not an error, it simply yields nothing
    char buffer[4] = {};
    ASSERT_EQ(stream->read(buffer, 4ul), 0ul);
    ASSERT_STATUS_OK();
}

TEST_F(MemoryMappedFileStreamTest, MappedRegionIsEmptyWithoutView) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::Read);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();

    // Nothing is addressable at end of file, and the region has to say so consistently with mappedData()
    ASSERT_EQ(stream->mappedData(), nullptr);
    ASSERT_EQ(stream->mappedOffset(), 0ul);
    ASSERT_EQ(stream->mappedSize(), 0ul);
    ASSERT_EQ(stream->size(), alignedFileSize);
}

// Everything below needs a mapping backend - without one the stream delegates to a plain file stream and holds no view. Gated
// on a build system fact rather than probed through the stream, so a broken backend fails these tests instead of disabling them.
#if defined(TRIO_MMAP_AVAILABLE) || defined(TRIO_WINDOWS_FILE_MAPPING_AVAILABLE)

class StalledWriter : public trio::Writable {
public:
    std::size_t write(const char* /* unused */, std::size_t /* unused */) override {
        return 0ul;
    }

    std::size_t write(trio::Readable* /* unused */, std::size_t /* unused */) override {
        return 0ul;
    }
};

class StalledReader : public trio::Readable {
public:
    std::size_t read(char* /* unused */, std::size_t /* unused */) override {
        return 0ul;
    }

    std::size_t read(trio::Writable* /* unused */, std::size_t /* unused */) override {
        return 0ul;
    }
};

TEST_F(MemoryMappedFileStreamTest, DataAtEndOfAlignedFileIsNull) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();
    // A mapping backend holds a view over the whole of a freshly opened non-empty file
    ASSERT_NE(stream->mappedData(), nullptr);

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();
    // No view is held here, which Mappable reports as "no addressable memory" rather than as an error
    ASSERT_EQ(stream->mappedData(), nullptr);
}

TEST_F(MemoryMappedFileStreamTest, MappedRegionSpansWholeFileAfterOpen) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    // The common case - the whole file is mapped in one view, so the region and the file coincide
    ASSERT_NE(stream->mappedData(), nullptr);
    ASSERT_EQ(stream->mappedOffset(), 0ul);
    ASSERT_EQ(stream->mappedSize(), alignedFileSize);
    ASSERT_EQ(stream->mappedSize(), stream->size());
}

TEST_F(MemoryMappedFileStreamTest, MappedRegionDelimitsPartialView) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();

    static const char appended[] = {0x01, 0x02};
    ASSERT_EQ(stream->write(appended, 2ul), 2ul);
    ASSERT_STATUS_OK();

    // The append remapped where it wrote, so the view covers only the tail - pairing mappedData() with size() here would read
    // alignedFileSize bytes past the end of the mapping
    ASSERT_EQ(stream->size(), alignedFileSize + 2ul);
    ASSERT_EQ(stream->mappedOffset(), alignedFileSize);
    ASSERT_EQ(stream->mappedSize(), 2ul);
    ASSERT_LT(stream->mappedSize(), stream->size());

    const char* mapped = stream->mappedData();
    ASSERT_NE(mapped, nullptr);
    ASSERT_ELEMENTS_EQ(mapped, appended, 2ul);
}

TEST_F(MemoryMappedFileStreamTest, ReopenOnEmptiedFileStartsFromBeginning) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();
    // The precondition the reopen has to survive - a position already zero here would leave the rest proving nothing
    ASSERT_EQ(stream->tell(), alignedFileSize);

    stream->close();
    // The file is opened unshared, so the truncation below only works if this released it
    ASSERT_STATUS_OK();
    // Emptied while nothing holds the file, so the reopen hits the delayed mapping path with a stale position
    ASSERT_NO_FATAL_FAILURE(EmptyTestFile());

    stream->open();
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 0ul);
    // Mapping is delayed for an empty file, and that path must still reset the position rather than inherit it
    ASSERT_EQ(stream->tell(), 0ul);

    static const char expected[] = {0x01, 0x02};
    ASSERT_EQ(stream->write(expected, 2ul), 2ul);
    ASSERT_STATUS_OK();
    // A stale position would have grown the file to alignedFileSize + 2 and left the payload behind a zero filled gap
    ASSERT_EQ(stream->size(), 2ul);

    stream->seek(0ul);
    ASSERT_STATUS_OK();
    char buffer[2] = {};
    ASSERT_EQ(stream->read(buffer, 2ul), 2ul);
    ASSERT_ELEMENTS_EQ(buffer, expected, 2ul);
}

TEST_F(MemoryMappedFileStreamTest, TruncationBelowPositionFollowsTheFileDown) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    stream->seek(alignedFileSize);
    ASSERT_STATUS_OK();

    // Truncating below the position must not strand it past the end of the file, a state seek() would never produce
    stream->resize(4ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 4ul);
    ASSERT_EQ(stream->tell(), 4ul);

    static const char appended[] = {0x01, 0x02};
    ASSERT_EQ(stream->write(appended, 2ul), 2ul);
    ASSERT_STATUS_OK();
    // A position left at the old end would have regrown the file and put the payload after a zero filled gap
    ASSERT_EQ(stream->size(), 6ul);

    stream->seek(0ul);
    ASSERT_STATUS_OK();
    static const char expected[] = {fillByte, fillByte, fillByte, fillByte, 0x01, 0x02};
    char buffer[6] = {};
    ASSERT_EQ(stream->read(buffer, 6ul), 6ul);
    ASSERT_ELEMENTS_EQ(buffer, expected, 6ul);
}

TEST_F(MemoryMappedFileStreamTest, ReadIntoStalledWritableReportsError) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::Read);
    stream->open();
    ASSERT_STATUS_OK();

    // A destination that never consumes must abort the transfer rather than spin on it
    StalledWriter destination;
    ASSERT_EQ(stream->read(&destination, 16ul), 0ul);
    ASSERT_STATUS_IS(trio::MemoryMappedFileStream::ReadError);
}

TEST_F(MemoryMappedFileStreamTest, WriteFromStalledReadableReportsError) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    // A source that never produces must abort the transfer rather than spin on it
    StalledReader source;
    ASSERT_EQ(stream->write(&source, 16ul), 0ul);
    ASSERT_STATUS_IS(trio::MemoryMappedFileStream::WriteError);
}

    // Nothing to wrap round where size_t is narrower than the position
    #if SIZE_MAX >= UINT64_MAX
TEST_F(MemoryMappedFileStreamTest, WriteOfASizeThatWouldWrapThePositionIsRefused) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    // Off zero, so the end position wraps rather than merely being unreachable
    stream->seek(1ul);
    ASSERT_STATUS_OK();

    // Holds as many bytes as the file could take, so dropping the guard below fails an assertion rather than reading past this
    const std::vector<char> source(alignedFileSize, 0x01);
    // The wrapped end position must not read as a request that fits in the file
    ASSERT_EQ(stream->write(source.data(), std::numeric_limits<std::size_t>::max()), 0ul);
    ASSERT_STATUS_IS(trio::MemoryMappedFileStream::WriteError);
    ASSERT_EQ(stream->size(), alignedFileSize);

    // Cleared so the read below answers for the file, not for the refusal
    sc::StatusProvider::reset();
    stream->seek(0ul);
    ASSERT_STATUS_OK();
    static const char untouched[] = {fillByte, fillByte, fillByte, fillByte};
    char buffer[4] = {};
    ASSERT_EQ(stream->read(buffer, 4ul), 4ul);
    // Refusing also has to leave the file as it was
    ASSERT_ELEMENTS_EQ(buffer, untouched, 4ul);
}
    #endif  // SIZE_MAX >= UINT64_MAX

TEST_F(MemoryMappedFileStreamTest, RefusedResizeKeepsTheViewUsable) {
    ASSERT_NO_FATAL_FAILURE(CreateAlignedTestFile());

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();

    // Beyond what any file can hold, so the resize is refused without touching the file
    stream->resize(std::numeric_limits<std::uint64_t>::max());
    ASSERT_STATUS_IS(trio::MemoryMappedFileStream::WriteError);
    ASSERT_EQ(stream->size(), alignedFileSize);
    ASSERT_EQ(stream->tell(), 0ul);

    // The refusal must not have cost the view: it still spans the unchanged file
    ASSERT_NE(stream->mappedData(), nullptr);
    ASSERT_EQ(stream->mappedOffset(), 0ul);
    ASSERT_EQ(stream->mappedSize(), alignedFileSize);

    // Cleared so the read below answers for the file, not for the refusal
    sc::StatusProvider::reset();
    // And the unchanged file is still readable at the unchanged position
    static const char untouched[] = {fillByte, fillByte, fillByte, fillByte};
    char buffer[4] = {};
    ASSERT_EQ(stream->read(buffer, 4ul), 4ul);
    ASSERT_STATUS_OK();
    ASSERT_ELEMENTS_EQ(buffer, untouched, 4ul);
}

#endif  // defined(TRIO_MMAP_AVAILABLE) || defined(TRIO_WINDOWS_FILE_MAPPING_AVAILABLE)

#if !defined(TRIO_MMAP_AVAILABLE) && !defined(TRIO_WINDOWS_FILE_MAPPING_AVAILABLE)
// The fallback backend fronts a FileStream: it can grow a file by appending zeros but has no way to shrink one
TEST_F(MemoryMappedFileStreamTest, FallbackResizeGrowsWithZerosAndRefusesToShrink) {
    static const char fixture[] = {'a', 'b', 'c', 'd'};
    {
        std::ofstream file{GetTestFileName(), std::ios::binary};
        ASSERT_TRUE(file.is_open());
        file.write(fixture, 4);
        ASSERT_TRUE(file.good());
    }

    auto stream = StreamFactory<trio::MemoryMappedFileStream>::create(GetTestFileName(), trio::AccessMode::ReadWrite);
    stream->open();
    ASSERT_STATUS_OK();
    stream->seek(2ul);
    ASSERT_STATUS_OK();

    // Growing keeps the position and pads with zeros
    stream->resize(6ul);
    ASSERT_STATUS_OK();
    ASSERT_EQ(stream->size(), 6ul);
    ASSERT_EQ(stream->tell(), 2ul);
    stream->close();
    ASSERT_STATUS_OK();
    static const char grown[] = {'a', 'b', 'c', 'd', 0, 0};
    {
        std::ifstream file{GetTestFileName(), std::ios::binary};
        char buffer[6] = {};
        file.read(buffer, 6);
        ASSERT_EQ(file.gcount(), 6);
        ASSERT_ELEMENTS_EQ(buffer, grown, 6ul);
    }

    // Shrinking is refused and changes nothing
    stream->open();
    ASSERT_STATUS_OK();
    stream->resize(2ul);
    ASSERT_STATUS_IS(trio::MemoryMappedFileStream::WriteError);
    ASSERT_EQ(stream->size(), 6ul);
}
#endif  // !defined(TRIO_MMAP_AVAILABLE) && !defined(TRIO_WINDOWS_FILE_MAPPING_AVAILABLE)

}  // namespace
