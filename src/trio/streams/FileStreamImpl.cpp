// Copyright Epic Games, Inc. All Rights Reserved.

#include "trio/streams/FileStreamImpl.h"

#include "trio/utils/NativeString.h"
#include "trio/utils/ScopedEnumEx.h"

#include <pma/PolyAllocator.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#ifdef _WIN32
    #include <share.h>
#elif defined(TRIO_FSTAT_AVAILABLE)
    #include <sys/stat.h>
#endif
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace trio {

namespace {

constexpr std::size_t bufferSize = 4096ul;
// positionedFor value that contains neither direction: forces the next transfer through a positioning call to filePos
constexpr AccessMode unpositioned = static_cast<AccessMode>(0);

// Mode strings must match NativeCharacter: wide on Unicode Windows builds, narrow everywhere else
#if defined(_WIN32) && defined(UNICODE)
    #define TRIO_NATIVE_TEXT(text) L##text
#else
    #define TRIO_NATIVE_TEXT(text) text
#endif

inline std::FILE* openFile(const NativeCharacter* path, const NativeCharacter* mode) {
    // _SH_DENYNO is what fopen and the MSVC filebuf use; the _fsopen spelling only dodges the C4996 deprecation of fopen
#if defined(_WIN32) && defined(UNICODE)
    return ::_wfsopen(path, mode, _SH_DENYNO);
#elif defined(_WIN32)
    return ::_fsopen(path, mode, _SH_DENYNO);
#else
    std::FILE* file = std::fopen(path, mode);
    #ifdef TRIO_FSTAT_AVAILABLE
    // Some C runtimes open a directory for reading and only fail on the first transfer; refuse it here so the caller sees
    // an open failure, as with the mapped streams. Without fstat the first transfer reports the error instead.
    struct stat info{};
    if ((file != nullptr) && (::fstat(::fileno(file), &info) == 0) && S_ISDIR(info.st_mode)) {
        std::fclose(file);
        return nullptr;
    }
    #endif  // TRIO_FSTAT_AVAILABLE
    return file;
#endif
}

// Same table std::basic_filebuf maps openmode through: in -> "r", out -> "w" (truncates), in|out -> "r+" (does not).
// Always binary: newline translation would make byte counts and file offsets disagree, so OpenMode::Text is not honoured.
inline const NativeCharacter* fileMode(AccessMode accessMode) {
    if (accessMode == AccessMode::ReadWrite) {
        return TRIO_NATIVE_TEXT("r+b");
    }
    if (contains(accessMode, AccessMode::Write)) {
        return TRIO_NATIVE_TEXT("wb");
    }
    return TRIO_NATIVE_TEXT("rb");
}

// 64-bit positioning. Standard fseek/ftell already cover it wherever long is 64-bit (every LP64 target), and some libcs
// declare nothing else. Where long is 32-bit, _WIN32 provides _fseeki64 and glibc the explicit 64-bit variants; any
// other ILP32 libc is left at the width of its off_t, as fseeko is all it offers. Not via _FILE_OFFSET_BITS - that
// macro must precede the first system header, which a unity build cannot promise.
inline int seekFile(std::FILE* file, std::uint64_t offset, int origin) {
#if defined(_WIN32)
    return ::_fseeki64(file, static_cast<std::int64_t>(offset), origin);
#elif LONG_MAX >= INT64_MAX
    return std::fseek(file, static_cast<long>(offset), origin);
#elif defined(__GLIBC__)
    return ::fseeko64(file, static_cast<off64_t>(offset), origin);
#else
    return ::fseeko(file, static_cast<off_t>(offset), origin);
#endif
}

inline std::int64_t tellFile(std::FILE* file) {
#if defined(_WIN32)
    return ::_ftelli64(file);
#elif LONG_MAX >= INT64_MAX
    return std::ftell(file);
#elif defined(__GLIBC__)
    return ::ftello64(file);
#else
    return ::ftello(file);
#endif
}

// A FileStream is not shared between threads, so the per-call FILE lock buys nothing. glibc is the one libc where it
// shows: its locked fread trails the libstdc++ filebuf by ~20% on small reads, the unlocked one matches it.
inline std::size_t readFile(char* destination, std::size_t size, std::FILE* file) {
#if defined(__GLIBC__)
    return ::fread_unlocked(destination, 1ul, size, file);
#else
    return std::fread(destination, 1ul, size, file);
#endif
}

inline std::size_t writeFile(const char* source, std::size_t size, std::FILE* file) {
#if defined(__GLIBC__)
    return ::fwrite_unlocked(source, 1ul, size, file);
#else
    return std::fwrite(source, 1ul, size, file);
#endif
}

inline bool atEndOfFile(std::FILE* file) {
#if defined(__GLIBC__)
    return ::feof_unlocked(file) != 0;
#else
    return std::feof(file) != 0;
#endif
}

inline bool inError(std::FILE* file) {
#if defined(__GLIBC__)
    return ::ferror_unlocked(file) != 0;
#else
    return std::ferror(file) != 0;
#endif
}

inline std::uint64_t sizeFile(std::FILE* file) {
    if (seekFile(file, 0ul, SEEK_END) != 0) {
        return 0ul;
    }
    const std::int64_t end = tellFile(file);
    return (end > 0 ? static_cast<std::uint64_t>(end) : 0ul);
}

inline std::uint64_t getFileSize(const NativeCharacter* path) {
    std::FILE* file = openFile(path, TRIO_NATIVE_TEXT("rb"));
    if (file == nullptr) {
        return 0ul;
    }
    const std::uint64_t result = sizeFile(file);
    std::fclose(file);
    return result;
}

// "r+" requires an existing file. Append mode creates a missing one and never truncates an existing one, in a single
// open - a probe-then-create pair would truncate a file that appeared in between.
inline void ensureFileExists(const NativeCharacter* path) {
    std::FILE* file = openFile(path, TRIO_NATIVE_TEXT("ab"));
    if (file != nullptr) {
        std::fclose(file);
    }
}

}  // namespace

FileStream::~FileStream() = default;

FileStream* FileStream::create(const char* path, AccessMode accessMode, OpenMode openMode, MemoryResource* memRes) {
    pma::PolyAllocator<FileStreamImpl> alloc{memRes};
    return alloc.newObject(path, accessMode, openMode, memRes);
}

void FileStream::destroy(FileStream* instance) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    auto stream = static_cast<FileStreamImpl*>(instance);
    pma::PolyAllocator<FileStreamImpl> alloc{stream->getMemoryResource()};
    alloc.deleteObject(stream);
}

FileStreamImpl::FileStreamImpl(const char* path_, AccessMode accessMode_, OpenMode /*unused*/, MemoryResource* memRes_) :
    file{nullptr},
    filePath{NativeStringConverter::from(path_, memRes_)},
    fileAccessMode{accessMode_},
    fileSize{getFileSize(filePath.c_str())},
    filePos{},
    positionedFor{AccessMode::ReadWrite},
    memRes{memRes_} {
}

FileStreamImpl::~FileStreamImpl() {
    FileStreamImpl::close();
}

bool FileStreamImpl::reposition(AccessMode direction) {
    // C update streams ("r+") demand a positioning call between a read and a write in either order.
    // Consecutive transfers in one direction need none, so seek only when the direction flips - fseek discards the
    // stdio buffer, and doing it on every call would turn each small transfer into a syscall.
    if (!contains(positionedFor, direction) && (seekFile(file, filePos, SEEK_SET) != 0)) {
        // Without it the transfer would land at the stdio position rather than at filePos
        return false;
    }
    positionedFor = direction;
    return true;
}

void FileStreamImpl::open() {
    status->reset();
    if (file != nullptr) {
        status->set(AlreadyOpenError, filePath.c_str());
        return;
    }

    if (fileAccessMode == AccessMode::ReadWrite) {
        ensureFileExists(filePath.c_str());
    }

    file = openFile(filePath.c_str(), fileMode(fileAccessMode));
    if (file == nullptr) {
        status->set(OpenError, filePath.c_str());
        return;
    }

    fileSize = sizeFile(file);
    seek(0ul);
}

void FileStreamImpl::flush() {
    // Output is pending only when the last transfer was a write; fflush after input is undefined for C update streams, and
    // on a closed or read-only stream there is nothing to push out, so like the mapped streams this is silent there.
    if ((file != nullptr) && (positionedFor == AccessMode::Write) && (std::fflush(file) != 0)) {
        status->set(WriteError, filePath.c_str());
    }
}

void FileStreamImpl::close() {
    if (file != nullptr) {
        // Buffered output reaches the disk only now, so this is the last place a write failure can still surface
        const bool flushed = (std::fclose(file) == 0);
        file = nullptr;
        if (!flushed && contains(fileAccessMode, AccessMode::Write)) {
            status->set(WriteError, filePath.c_str());
        }
    }
}

std::uint64_t FileStreamImpl::tell() {
    return filePos;
}

void FileStreamImpl::seek(std::uint64_t position) {
    const bool seekable = ((position == 0ul) || (position <= size())) && (file != nullptr) && !inError(file);
    if (!seekable) {
        status->set(SeekError, filePath.c_str());
        return;
    }

    if (seekFile(file, position, SEEK_SET) != 0) {
        status->set(SeekError, filePath.c_str());
        return;
    }

    filePos = position;
    positionedFor = AccessMode::ReadWrite;
}

std::size_t FileStreamImpl::read(char* destination, std::size_t size) {
    if ((destination == nullptr) || (file == nullptr) || inError(file) || !contains(fileAccessMode, AccessMode::Read)) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    if (atEndOfFile(file)) {
        return 0ul;
    }

    if (!reposition(AccessMode::Read)) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    const std::size_t bytesRead = readFile(destination, size, file);
    if (inError(file)) {
        status->set(ReadError, filePath.c_str());
    }

    filePos += bytesRead;
    return bytesRead;
}

std::size_t FileStreamImpl::read(Writable* destination, std::size_t size) {
    if ((destination == nullptr) || (file == nullptr) || inError(file) || !contains(fileAccessMode, AccessMode::Read)) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    if (atEndOfFile(file)) {
        return 0ul;
    }

    if (!reposition(AccessMode::Read)) {
        status->set(ReadError, filePath.c_str());
        return 0ul;
    }

    char buffer[bufferSize];
    std::size_t bytesRead = 0ul;
    while (size > 0ul) {
        const std::size_t chunkSize = std::min(size, bufferSize);
        const std::size_t chunkRead = readFile(buffer, chunkSize, file);
        if (chunkRead == 0ul) {
            break;
        }
        // A destination may take a chunk in pieces; only one that takes nothing has stopped consuming
        std::size_t chunkWritten = 0ul;
        while (chunkWritten != chunkRead) {
            const std::size_t remaining = chunkRead - chunkWritten;
            // Not trusted beyond what it was handed - an overshoot would count bytes the destination never received
            const std::size_t accepted = std::min(destination->write(buffer + chunkWritten, remaining), remaining);
            if (accepted == 0ul) {
                break;
            }
            chunkWritten += accepted;
        }
        bytesRead += chunkWritten;
        size -= chunkWritten;
        if (chunkWritten != chunkRead) {
            // The refused bytes already left the C stream, so move it back to the position the caller observes. That is a
            // positioning call, hence either direction may follow.
            status->set(ReadError, filePath.c_str());
            if (seekFile(file, filePos + bytesRead, SEEK_SET) == 0) {
                positionedFor = AccessMode::ReadWrite;
            } else {
                // The next transfer must seek first, so the failure resurfaces as an error instead of skewed data
                positionedFor = unpositioned;
                // A short final chunk left the EOF indicator set; only a successful seek clears it, and it must not let the
                // next read return 0 quietly before the forced seek can report the failure
                if (!inError(file)) {
                    std::clearerr(file);
                }
            }
            break;
        }
    }

    if (inError(file)) {
        status->set(ReadError, filePath.c_str());
    }

    filePos += bytesRead;
    return bytesRead;
}

std::size_t FileStreamImpl::write(const char* source, std::size_t size) {
    if ((source == nullptr) || (file == nullptr) || inError(file) || !contains(fileAccessMode, AccessMode::Write)) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    if (!reposition(AccessMode::Write)) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    // Whatever did go out has moved the stdio position, so the tracked one follows it even on a short write
    const std::size_t bytesWritten = writeFile(source, size, file);
    fileSize = std::max(filePos + bytesWritten, fileSize);
    filePos += bytesWritten;
    if (bytesWritten != size) {
        status->set(WriteError, filePath.c_str());
    }
    return bytesWritten;
}

std::size_t FileStreamImpl::write(Readable* source, std::size_t size) {
    if ((source == nullptr) || (file == nullptr) || inError(file) || !contains(fileAccessMode, AccessMode::Write)) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    if (!reposition(AccessMode::Write)) {
        status->set(WriteError, filePath.c_str());
        return 0ul;
    }

    char buffer[bufferSize];
    std::size_t bytesWritten = 0ul;
    while (bytesWritten != size) {
        const std::size_t chunkSize = std::min(size - bytesWritten, bufferSize);
        // Not trusted beyond what it was handed - an overshoot would write stack bytes the source never filled in
        const std::size_t chunkRead = std::min(source->read(buffer, chunkSize), chunkSize);
        if (chunkRead == 0ul) {
            // Source stopped producing, so the loop cannot make progress
            status->set(WriteError, filePath.c_str());
            break;
        }
        const std::size_t chunkWritten = writeFile(buffer, chunkRead, file);
        bytesWritten += chunkWritten;
        if (chunkWritten != chunkRead) {
            status->set(WriteError, filePath.c_str());
            break;
        }
    }

    fileSize = std::max(filePos + bytesWritten, fileSize);
    filePos += bytesWritten;
    return bytesWritten;
}

std::uint64_t FileStreamImpl::size() {
    return fileSize;
}

MemoryResource* FileStreamImpl::getMemoryResource() {
    return memRes;
}

}  // namespace trio

// Macros ignore namespaces, so a unity build would otherwise carry it into every file compiled after this one
#undef TRIO_NATIVE_TEXT
