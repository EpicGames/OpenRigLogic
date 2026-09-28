// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef TRIO_MMAP_AVAILABLE

    #ifdef TRIO_LARGE_FILE_SUPPORT_AVAILABLE
        #define _FILE_OFFSET_BITS 64
    #endif  // TRIO_LARGE_FILE_SUPPORT

    #include "trio/streams/MemoryMappedFileStreamUnix.h"
    #include "trio/utils/NativeString.h"
    #include "trio/utils/ScopedEnumEx.h"

    #include <pma/PolyAllocator.h>

    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>

    #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4365 4987)
    #endif
    #include <algorithm>
    #include <cassert>
    #include <cstdint>
    #include <cstdio>
    #include <cstring>
    #include <limits>
    #include <type_traits>
    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif

namespace trio {

namespace {

constexpr std::size_t minViewSizeUnix = 65536ul;

inline std::uint64_t getFileSizeUnix(const NativeCharacter* path) {
    struct stat st{};
    // A directory reports a size too, but is not openable as a stream, so it counts as missing here as in FileStream
    if ((::stat(path, &st) != 0) || S_ISDIR(st.st_mode)) {
        return 0ul;
    }
    return static_cast<std::uint64_t>(st.st_size);
}

inline std::uint64_t getPageSizeUnix() {
    #ifdef TRIO_PAGE_SIZE_UNIX
    return static_cast<std::uint64_t>(TRIO_PAGE_SIZE_UNIX);
    #else
    return static_cast<std::uint64_t>(sysconf(_SC_PAGE_SIZE));
    #endif  // TRIO_PAGE_SIZE_UNIX
}

inline std::uint64_t alignOffsetUnix(std::uint64_t offset) {
    const std::uint64_t pageSize = getPageSizeUnix();
    return offset / pageSize * pageSize;
}

class MemoryReaderUnix : public Readable {
public:
    explicit MemoryReaderUnix(const char* source_) :
        source{source_} {
    }

    std::size_t read(char* destination, std::size_t size) override {
        std::memcpy(destination, source, size);
        source += size;
        return size;
    }

    std::size_t read(Writable* destination, std::size_t size) override {
        destination->write(source, size);
        source += size;
        return size;
    }

private:
    const char* source;
};

class MemoryWriterUnix : public Writable {
public:
    explicit MemoryWriterUnix(char* destination_) :
        destination{destination_} {
    }

    std::size_t write(const char* source, std::size_t size) override {
        std::memcpy(destination, source, size);
        destination += size;
        return size;
    }

    std::size_t write(Readable* source, std::size_t size) override {
        source->read(destination, size);
        destination += size;
        return size;
    }

private:
    char* destination;
};

}  // namespace

MemoryMappedFileStreamUnix::MemoryMappedFileStreamUnix(const char* path_, AccessMode accessMode_, MemoryResource* memRes_) :
    filePath{NativeStringConverter::from(path_, memRes_)},
    fileAccessMode{accessMode_},
    memRes{memRes_},
    file{-1},
    mapped{nullptr},
    position{},
    fileSize{getFileSizeUnix(filePath.c_str())},
    viewOffset{},
    viewSize{},
    dirty{false} {
}

MemoryMappedFileStreamUnix::~MemoryMappedFileStreamUnix() {
    MemoryMappedFileStreamUnix::close();
}

const char* MemoryMappedFileStreamUnix::mappedData() {
    if (mapped == nullptr) {
        return nullptr;
    }
    return static_cast<const char*>(mapped);
}

std::uint64_t MemoryMappedFileStreamUnix::mappedOffset() {
    return (mapped == nullptr ? 0ul : viewOffset);
}

std::size_t MemoryMappedFileStreamUnix::mappedSize() {
    return (mapped == nullptr ? 0ul : viewSize);
}

MemoryResource* MemoryMappedFileStreamUnix::getMemoryResource() {
    return memRes;
}

std::uint64_t MemoryMappedFileStreamUnix::size() {
    return fileSize;
}

void MemoryMappedFileStreamUnix::open() {
    status->reset();
    if (file != -1) {
        status->set(AlreadyOpenError, filePath.c_str());
        return;
    }

    openFile();
    if (file == -1) {
        status->set(OpenError, filePath.c_str());
        return;
    }

    struct stat st{};
    // open() accepts a directory for reading; refuse it here rather than let the mapping below fail for it
    if ((::fstat(file, &st) != 0) || S_ISDIR(st.st_mode)) {
        fileSize = 0ul;
        closeFile();
        status->set(OpenError, filePath.c_str());
        return;
    }

    fileSize = static_cast<std::uint64_t>(st.st_size);
    // close() leaves the position behind, so reset it here - above the empty file return below, which would otherwise skip it
    position = 0ul;
    dirty = false;

    // Mapping of 0-length files is delayed until the file is resized to a non-zero size.
    if (fileSize == 0ul) {
        return;
    }

    mapFile(0ul, fileSize);
    if (mapped == nullptr) {
        status->set(OpenError, filePath.c_str());
        unmapFile();
        closeFile();
        // The file size stays as read above - size() reports the file, not the view, so a failed mapping does not clear it
        return;
    }
}

void MemoryMappedFileStreamUnix::close() {
    flush();
    unmapFile();
    closeFile();
}

std::uint64_t MemoryMappedFileStreamUnix::tell() {
    return position;
}

void MemoryMappedFileStreamUnix::seek(std::uint64_t position_) {
    const bool seekable = ((position_ == 0ul) || (position_ <= size())) && (file != -1);
    if (!seekable) {
        status->set(SeekError, filePath.c_str());
        return;
    }

    // The position cannot be committed before the view it leaves is flushed and unmapped, or read()/write() index outside it
    const bool leavingView = (position_ < viewOffset) || (position_ >= (viewOffset + viewSize));
    if (leavingView) {
        // Only a dirty view has anything to write back, and flushing a clean one could latch a WriteError behind a good seek
        if (dirty) {
            flush();
            if (dirty) {
                // Left as flush() reported it - the lost writes matter more than a SeekError, and the position stays put
                return;
            }
        }
        unmapFile();
    }

    position = position_;

    // Nothing left to map at end of file - a later write remaps once it has resized
    if (leavingView && (position < fileSize)) {
        mapFile(position, fileSize - position);
        if (mapped == nullptr) {
            status->set(SeekError, filePath.c_str());
        }
    }
}

std::size_t MemoryMappedFileStreamUnix::read(char* destination, std::size_t size) {
    if (destination == nullptr) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    MemoryWriterUnix writer{destination};
    return read(&writer, size);
}

std::size_t MemoryMappedFileStreamUnix::read(Writable* destination, std::size_t size) {
    if ((destination == nullptr) || !contains(fileAccessMode, AccessMode::Read)) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    if (mapped == nullptr) {
        // At or past the end of the file there is simply nothing to yield; anywhere earlier a missing view means a failed map
        const bool nothingToRead = (file != -1) && (position >= fileSize);
        if (!nothingToRead) {
            status->set(ReadError, filePath.c_str());
        }
        return 0ul;
    }

    const std::uint64_t bytesAvailable = fileSize - position;
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuseless-cast"
    #endif
    const std::size_t bytesToRead = static_cast<std::size_t>(std::min(static_cast<std::uint64_t>(size), bytesAvailable));
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
    std::size_t bytesRead = 0ul;

    while (bytesRead != bytesToRead) {
        const std::size_t bytesRemaining = bytesToRead - bytesRead;
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuseless-cast"
    #endif
        std::size_t viewPosition = static_cast<std::size_t>(position - viewOffset);
        std::size_t bytesReadable = static_cast<std::size_t>(viewSize - viewPosition);
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
        // If the view is exhausted during reading, remap a new view till the end of file if possible,
        // starting at the current position
        if (bytesReadable == 0ul) {
            // Only a dirty view has anything to write back, and flushing a clean one could report WriteError out of a read
            if (dirty) {
                flush();
                if (dirty) {
                    break;
                }
            }
            unmapFile();
            mapFile(position, fileSize - position);
            if (mapped == nullptr) {
                // Failed to map new view
                status->set(ReadError, filePath.c_str());
                break;
            }
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuseless-cast"
    #endif
            viewPosition = static_cast<std::size_t>(position - viewOffset);
            bytesReadable = static_cast<std::size_t>(viewSize - viewPosition);
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
        }

        const std::size_t chunkSize = std::min(bytesRemaining, bytesReadable);
        const std::size_t chunkWritten = destination->write(static_cast<char*>(mapped) + viewPosition, chunkSize);
        // Not trusted beyond what it was handed - the loop ends on an exact match, so an overshoot would never terminate
        const std::size_t chunkCopied = std::min(chunkWritten, chunkSize);
        if (chunkCopied == 0ul) {
            // Destination stopped consuming, so the loop cannot make progress
            status->set(ReadError, filePath.c_str());
            break;
        }
        bytesRead += chunkCopied;
        position += chunkCopied;
    }

    return bytesRead;
}

std::size_t MemoryMappedFileStreamUnix::write(const char* source, std::size_t size) {
    if (source == nullptr) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    MemoryReaderUnix reader{source};
    return write(&reader, size);
}

std::size_t MemoryMappedFileStreamUnix::write(Readable* source, std::size_t size) {
    if ((source == nullptr) || !contains(fileAccessMode, AccessMode::Write)) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    if (mapped == nullptr) {
        // At or past the end there is no view yet but the resize below maps one in; earlier it means a mapping failed
        const bool mappableOnWrite = (file != -1) && (position >= fileSize);
        if (!mappableOnWrite) {
            status->set(WriteError, filePath.c_str());
            return 0ul;
        }
    }

    if (size == 0ul) {
        return 0ul;
    }

    // Wrapped round, the end position would read as a small request and the loop would overrun both the source and the file
    const std::uint64_t endPosition = position + size;
    if (endPosition < position) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    if (endPosition > fileSize) {
        resize(endPosition);
        // resize sets the status; the size alone is not conclusive, as the file can grow on disk and still fail to remap
        if ((fileSize != endPosition) || (mapped == nullptr)) {
            return 0ul;
        }
    }

    std::size_t bytesWritten = 0ul;

    while (bytesWritten != size) {
        const std::size_t bytesRemaining = size - bytesWritten;
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuseless-cast"
    #endif
        std::size_t viewPosition = static_cast<std::size_t>(position - viewOffset);
        std::size_t bytesWritable = static_cast<std::size_t>(viewSize - viewPosition);
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
        // If the view is exhausted during writing, remap a new view till the end of file if possible,
        // starting at the current position
        if (bytesWritable == 0ul) {
            // Only a dirty view has anything to write back, and dirty is set per chunk below so this sees the writes just made
            if (dirty) {
                flush();
                if (dirty) {
                    break;
                }
            }
            unmapFile();
            mapFile(position, fileSize - position);
            if (mapped == nullptr) {
                // Failed to map new view
                status->set(WriteError, filePath.c_str());
                break;
            }
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuseless-cast"
    #endif
            viewPosition = static_cast<std::size_t>(position - viewOffset);
            bytesWritable = static_cast<std::size_t>(viewSize - viewPosition);
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif
        }

        const std::size_t chunkSize = std::min(bytesRemaining, bytesWritable);
        const std::size_t chunkRead = source->read(static_cast<char*>(mapped) + viewPosition, chunkSize);
        // Not trusted beyond what it was handed - the loop ends on an exact match, so an overshoot would never terminate
        const std::size_t chunkCopied = std::min(chunkRead, chunkSize);
        if (chunkCopied == 0ul) {
            // Source stopped producing, so the loop cannot make progress
            status->set(WriteError, filePath.c_str());
            break;
        }
        bytesWritten += chunkCopied;
        position += chunkCopied;
        // Per chunk, so the rotation above sees this call's writes - only a successful flush() clears it again
        dirty = true;
    }

    return bytesWritten;
}

void MemoryMappedFileStreamUnix::flush() {
    if (mapped != nullptr) {
        if (::msync(mapped, viewSize, MS_SYNC) != 0) {
            status->set(WriteError, filePath.c_str());
            return;
        }
    }
    dirty = false;
}

void MemoryMappedFileStreamUnix::resize(std::uint64_t size) {
    if (fileAccessMode == AccessMode::Read) {
        status->set(WriteError, filePath.c_str());
        return;
    }

    flush();
    if (dirty) {
        return;
    }

    unmapFile();
    resizeFile(size);
    // A refused or failed resize leaves the file as it was, so a view is mapped again below and the stream stays usable;
    // like any remap it invalidates earlier mappedData pointers
    if (fileSize != size) {
        status->set(WriteError, filePath.c_str());
    }

    // Truncating below the position would strand it past the end of the file - follow the file down, as a seek would have to
    position = std::min(position, fileSize);

    // Truncated down to the position - as in seek, a later write remaps once it has grown the file again
    if (position < fileSize) {
        mapFile(position, fileSize - position);
        if (mapped == nullptr) {
            status->set(WriteError, filePath.c_str());
            return;
        }
    }
}

void MemoryMappedFileStreamUnix::openFile() {
    int openFlags{};
    if (fileAccessMode == AccessMode::ReadWrite) {
        openFlags = O_RDWR | O_CREAT;
    } else if (fileAccessMode == AccessMode::Read) {
        openFlags = O_RDONLY;
    } else if (fileAccessMode == AccessMode::Write) {
        // mmap needs also read permission to the underlying file descriptor; write-only truncates like FileStream ("w")
        openFlags = O_RDWR | O_CREAT | O_TRUNC;
    }

    const int mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    file = ::open(filePath.c_str(), openFlags, mode);
}

void MemoryMappedFileStreamUnix::closeFile() {
    if (file != -1) {
        ::close(file);
        file = -1;
    }
}

void MemoryMappedFileStreamUnix::mapFile(std::uint64_t offset, std::uint64_t size) {
    int prot{};
    prot |= (contains(fileAccessMode, AccessMode::Write) ? PROT_WRITE : prot);
    prot |= (contains(fileAccessMode, AccessMode::Read) ? PROT_READ : prot);

    const int flags = (fileAccessMode == AccessMode::Read ? MAP_PRIVATE : MAP_SHARED);

    const std::uint64_t alignedOffset = alignOffsetUnix(offset);

    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuseless-cast"
    #endif
    // Make sure size does not exceed system limits
    std::size_t safeSize =
        static_cast<std::size_t>(std::min(static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()), size));
    // Increase to-be-mapped size by the difference caused by alignment (guard against wrap-around on overflow)
    safeSize = std::max(static_cast<std::size_t>(safeSize + (offset - alignedOffset)), safeSize);
    #if !defined(__clang__) && defined(__GNUC__)
        #pragma GCC diagnostic pop
    #endif

    // Try mapping requested size, but if it fails keep repeating by halving the view size each time (e.g. if not enough VA space)
    std::size_t nextSize = safeSize;
    do {
        safeSize = nextSize;
        mapped = ::mmap(nullptr, safeSize, prot, flags, file, static_cast<off_t>(alignedOffset));
        if (mapped != reinterpret_cast<void*>(-1)) {
            break;
        }
        nextSize = safeSize / 2ul;
    } while (nextSize >= minViewSizeUnix);

    // mmap signals failure with MAP_FAILED rather than a null pointer - normalized here, as every caller tests for nullptr
    if (mapped == reinterpret_cast<void*>(-1)) {
        mapped = nullptr;
        return;
    }

    viewOffset = alignedOffset;
    viewSize = safeSize;
    // The halving stops at minViewSizeUnix, never below the page size, so offset is inside even a reduced view
    assert((viewOffset <= offset) && ((offset - viewOffset) < viewSize));
}

void MemoryMappedFileStreamUnix::unmapFile() {
    // Not reported through the status channel: this also runs from the destructor and on every view rotation, and munmap can
    // only fail on a range this class never mapped - a bug rather than a runtime condition, so it is asserted instead.
    if (mapped != nullptr) {
        const int unmapped = ::munmap(mapped, viewSize);
        assert(unmapped == 0);
        static_cast<void>(unmapped);
        mapped = nullptr;
    }

    viewOffset = 0ul;
    viewSize = 0ul;
}

void MemoryMappedFileStreamUnix::resizeFile(std::uint64_t size) {
    if (file == -1) {
        return;
    }

    // Without large file support off_t is narrower, and a truncated cast would resize to some other length - refused instead
    if (size > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
        return;
    }

    if (::ftruncate(file, static_cast<off_t>(size)) != 0) {
        return;
    }

    fileSize = size;
}

}  // namespace trio

#endif  // TRIO_MMAP_AVAILABLE
