// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef TRIO_WINDOWS_FILE_MAPPING_AVAILABLE

    #include "trio/streams/MemoryMappedFileStreamWindows.h"

    #include "trio/utils/NativeString.h"
    #include "trio/utils/ScopedEnumEx.h"

    #include <pma/PolyAllocator.h>

    #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4365 4987)
    #endif
    #include <algorithm>
    #include <cassert>
    #include <cstddef>
    #include <cstdint>
    #include <cstring>
    #include <limits>
    #include <type_traits>
    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif

namespace trio {

namespace {

constexpr std::size_t minViewSizeWindows = 65536ul;

inline std::uint64_t getFileSizeWindows(const NativeCharacter* path) {
    WIN32_FILE_ATTRIBUTE_DATA w32fad;
    // A directory may report a size on some file systems, but is not openable as a stream, so it counts as missing here
    if ((GetFileAttributesEx(path, GetFileExInfoStandard, &w32fad) == 0) ||
        ((w32fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0u)) {
        return 0ul;
    }
    ULARGE_INTEGER size;
    size.HighPart = w32fad.nFileSizeHigh;
    size.LowPart = w32fad.nFileSizeLow;
    return static_cast<std::uint64_t>(size.QuadPart);
}

inline std::uint64_t getPageSizeWindows() {
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwAllocationGranularity;
}

inline std::uint64_t alignOffsetWindows(std::uint64_t offset) {
    const std::uint64_t pageSize = getPageSizeWindows();
    return offset / pageSize * pageSize;
}

class MemoryReaderWindows : public Readable {
public:
    explicit MemoryReaderWindows(const char* source_) :
        source{source_} {
    }

    std::size_t read(char* destination, std::size_t size) override {
        CopyMemory(destination, source, size);
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

class MemoryWriterWindows : public Writable {
public:
    explicit MemoryWriterWindows(char* destination_) :
        destination{destination_} {
    }

    std::size_t write(const char* source, std::size_t size) override {
        CopyMemory(destination, source, size);
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

MemoryMappedFileStreamWindows::MemoryMappedFileStreamWindows(const char* path_, AccessMode accessMode_, MemoryResource* memRes_) :
    filePath{NativeStringConverter::from(path_, memRes_)},
    fileAccessMode{accessMode_},
    memRes{memRes_},
    file{INVALID_HANDLE_VALUE},
    mapping{nullptr},
    mapped{nullptr},
    position{},
    fileSize{getFileSizeWindows(filePath.c_str())},
    viewOffset{},
    viewSize{},
    dirty{false} {
}

MemoryMappedFileStreamWindows::~MemoryMappedFileStreamWindows() {
    flush();
    unmapFile();
    closeFile();
}

const char* MemoryMappedFileStreamWindows::mappedData() {
    return static_cast<const char*>(mapped);
}

std::uint64_t MemoryMappedFileStreamWindows::mappedOffset() {
    return (mapped == nullptr ? 0ul : viewOffset);
}

std::size_t MemoryMappedFileStreamWindows::mappedSize() {
    return (mapped == nullptr ? 0ul : viewSize);
}

MemoryResource* MemoryMappedFileStreamWindows::getMemoryResource() {
    return memRes;
}

std::uint64_t MemoryMappedFileStreamWindows::size() {
    return fileSize;
}

void MemoryMappedFileStreamWindows::open() {
    status->reset();
    if (file != INVALID_HANDLE_VALUE) {
        status->set(AlreadyOpenError, filePath.c_str());
        return;
    }

    openFile();
    if (file == INVALID_HANDLE_VALUE) {
        status->set(OpenError, filePath.c_str());
        return;
    }

    // Retrieve file size
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) == 0) {
        fileSize = 0ul;
        closeFile();
        status->set(OpenError, filePath.c_str());
        return;
    }

    fileSize = static_cast<std::uint64_t>(size.QuadPart);

    // Write-only truncates like FileStream ("w"); done here, before any view exists, so resizeFile has nothing to unmap
    if ((fileAccessMode == AccessMode::Write) && (fileSize != 0ul)) {
        resizeFile(0ul);
        if (fileSize != 0ul) {
            closeFile();
            status->set(OpenError, filePath.c_str());
            return;
        }
    }

    // close() leaves the position behind, so reset it here - above the empty file return below, which would otherwise skip it
    position = 0ul;
    dirty = false;

    // Mapping of 0-length files is delayed until the file is resized to a non-zero size.
    if (fileSize == 0ul) {
        return;
    }

    // Create file mapping
    mapFile(0ul, fileSize);
    if (mapped == nullptr) {
        status->set(OpenError, filePath.c_str());
        unmapFile();
        closeFile();
        // The file size stays as read above - size() reports the file, not the view, so a failed mapping does not clear it
        return;
    }
}

void MemoryMappedFileStreamWindows::close() {
    flush();
    unmapFile();
    closeFile();
}

std::uint64_t MemoryMappedFileStreamWindows::tell() {
    return position;
}

void MemoryMappedFileStreamWindows::seek(std::uint64_t position_) {
    const bool seekable = ((position_ == 0ul) || (position_ <= size())) && (file != INVALID_HANDLE_VALUE);
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

std::size_t MemoryMappedFileStreamWindows::read(char* destination, std::size_t size) {
    if (destination == nullptr) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    MemoryWriterWindows writer{destination};
    return read(&writer, size);
}

std::size_t MemoryMappedFileStreamWindows::read(Writable* destination, std::size_t size) {
    if ((destination == nullptr) || !contains(fileAccessMode, AccessMode::Read)) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    if (mapped == nullptr) {
        // At or past the end of the file there is simply nothing to yield; anywhere earlier a missing view means a failed map
        const bool nothingToRead = (file != INVALID_HANDLE_VALUE) && (position >= fileSize);
        if (!nothingToRead) {
            status->set(ReadError, filePath.c_str());
        }
        return 0ul;
    }

    const std::uint64_t bytesAvailable = fileSize - position;
    const std::size_t bytesToRead = static_cast<std::size_t>(std::min(static_cast<std::uint64_t>(size), bytesAvailable));
    std::size_t bytesRead = 0ul;

    while (bytesRead != bytesToRead) {
        const std::size_t bytesRemaining = bytesToRead - bytesRead;
        std::size_t viewPosition = static_cast<std::size_t>(position - viewOffset);
        std::size_t bytesReadable = static_cast<std::size_t>(viewSize - viewPosition);
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
            viewPosition = static_cast<std::size_t>(position - viewOffset);
            bytesReadable = static_cast<std::size_t>(viewSize - viewPosition);
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

std::size_t MemoryMappedFileStreamWindows::write(const char* source, std::size_t size) {
    if (source == nullptr) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    MemoryReaderWindows reader{source};
    return write(&reader, size);
}

std::size_t MemoryMappedFileStreamWindows::write(Readable* source, std::size_t size) {
    if ((source == nullptr) || !contains(fileAccessMode, AccessMode::Write)) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    if (mapped == nullptr) {
        // At or past the end there is no view yet but the resize below maps one in; earlier it means a mapping failed
        const bool mappableOnWrite = (file != INVALID_HANDLE_VALUE) && (position >= fileSize);
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
        std::size_t viewPosition = static_cast<std::size_t>(position - viewOffset);
        std::size_t bytesWritable = static_cast<std::size_t>(viewSize - viewPosition);
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
            viewPosition = static_cast<std::size_t>(position - viewOffset);
            bytesWritable = static_cast<std::size_t>(viewSize - viewPosition);
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

void MemoryMappedFileStreamWindows::flush() {
    if ((mapped != nullptr) && (fileAccessMode != AccessMode::Read)) {
        if (!FlushViewOfFile(mapped, 0ul)) {
            status->set(WriteError, filePath.c_str());
            return;
        }
    }
    dirty = false;
}

void MemoryMappedFileStreamWindows::resize(std::uint64_t size) {
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

void MemoryMappedFileStreamWindows::openFile() {
    DWORD access{GENERIC_READ};
    access |= (contains(fileAccessMode, AccessMode::Write) ? GENERIC_WRITE : access);

    // 0 == no sharing in any way
    DWORD sharing{};

    // Non-existing files are created unless in read-only mode. Write-only truncates in open() rather than through
    // CREATE_ALWAYS, which refuses existing hidden or system files (as fopen("w") does on this runtime); here they are
    // truncated like any other file
    DWORD creationDisposition{};
    if (fileAccessMode == AccessMode::Read) {
        creationDisposition = static_cast<DWORD>(OPEN_EXISTING);
    } else {
        creationDisposition = static_cast<DWORD>(OPEN_ALWAYS);
    }

    file = CreateFile(filePath.c_str(), access, sharing, nullptr, creationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
}

void MemoryMappedFileStreamWindows::closeFile() {
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        file = INVALID_HANDLE_VALUE;
    }
}

void MemoryMappedFileStreamWindows::mapFile(std::uint64_t offset, std::uint64_t size) {
    // Create file mapping
    const auto protect = static_cast<DWORD>(contains(fileAccessMode, AccessMode::Write) ? PAGE_READWRITE : PAGE_READONLY);
    mapping = CreateFileMapping(file, nullptr, protect, 0u, 0u, nullptr);
    if (mapping == nullptr) {
        return;
    }

    // Map a view of the file mapping into the address space
    DWORD desiredAccess{};
    desiredAccess |= (contains(fileAccessMode, AccessMode::Write) ? FILE_MAP_WRITE : desiredAccess);
    desiredAccess |= (contains(fileAccessMode, AccessMode::Read) ? FILE_MAP_READ : desiredAccess);

    ULARGE_INTEGER alignedOffset{};
    alignedOffset.QuadPart = static_cast<decltype(alignedOffset.QuadPart)>(alignOffsetWindows(offset));

    // Make sure size does not exceed system limits
    std::size_t safeSize =
        static_cast<std::size_t>(std::min(static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()), size));
    // Increase to-be-mapped size by the difference caused by alignment (guard against wrap-around on overflow)
    safeSize = std::max(static_cast<std::size_t>(safeSize + (offset - alignedOffset.QuadPart)), safeSize);

    // Try mapping requested size, but if it fails keep repeating by halving the view size each time (e.g. if not enough VA space)
    std::size_t nextSize = safeSize;
    do {
        safeSize = nextSize;
        mapped = MapViewOfFile(mapping, desiredAccess, alignedOffset.HighPart, alignedOffset.LowPart, safeSize);
        if (mapped != nullptr) {
            break;
        }
        nextSize = safeSize / 2ul;
    } while (nextSize >= minViewSizeWindows);

    if (mapped == nullptr) {
        // No view, so the mapping object is of no further use - released here, as callers only ever test mapped and would
        // otherwise leave a kernel handle open for as long as the stream lives
        CloseHandle(mapping);
        mapping = nullptr;
        return;
    }

    viewOffset = alignedOffset.QuadPart;
    viewSize = safeSize;
    // The halving stops at minViewSizeWindows, never below the alignment granularity, so offset is inside even a reduced view
    assert((viewOffset <= offset) && ((offset - viewOffset) < viewSize));
}

void MemoryMappedFileStreamWindows::unmapFile() {
    // Not reported through the status channel: this also runs from the destructor and on every view rotation, and both calls
    // can only fail on an address or handle this class never mapped - a bug rather than a runtime condition, so asserted.
    if (mapped != nullptr) {
        const BOOL unmapped = UnmapViewOfFile(mapped);
        assert(unmapped != 0);
        static_cast<void>(unmapped);
        mapped = nullptr;
    }

    if (mapping != nullptr) {
        const BOOL closed = CloseHandle(mapping);
        assert(closed != 0);
        static_cast<void>(closed);
        mapping = nullptr;
    }

    viewOffset = 0ul;
    viewSize = 0ul;
}

void MemoryMappedFileStreamWindows::resizeFile(std::uint64_t size) {
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    // The seek below takes a signed offset, so anything above its range would move backwards - refused instead
    if (size > static_cast<std::uint64_t>(std::numeric_limits<LONGLONG>::max())) {
        return;
    }

    // Seek to the new size
    LARGE_INTEGER moveBy{};
    moveBy.QuadPart = static_cast<decltype(moveBy.QuadPart)>(size);
    if (SetFilePointerEx(file, moveBy, nullptr, FILE_BEGIN) == 0) {
        return;
    }

    // Resize the file to it's current position
    if (SetEndOfFile(file) == 0) {
        return;
    }

    fileSize = size;
}

}  // namespace trio

#endif  // TRIO_WINDOWS_FILE_MAPPING_AVAILABLE
