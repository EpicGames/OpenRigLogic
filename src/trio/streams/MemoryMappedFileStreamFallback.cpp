// Copyright Epic Games, Inc. All Rights Reserved.

#if !defined(TRIO_WINDOWS_FILE_MAPPING_AVAILABLE) && !defined(TRIO_MMAP_AVAILABLE)

    #include "trio/streams/MemoryMappedFileStreamFallback.h"

    #include "trio/utils/ScopedEnumEx.h"

    #include <pma/ScopedPtr.h>

    #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4365 4987)
    #endif
    #include <algorithm>
    #include <cstddef>
    #include <cstdint>
    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif

namespace trio {

MemoryMappedFileStreamFallback::MemoryMappedFileStreamFallback(const char* path_,
                                                               AccessMode accessMode_,
                                                               MemoryResource* memRes_) :
    stream{pma::makeScoped<FileStream>(path_, accessMode_, OpenMode::Binary, memRes_)},
    memRes{memRes_} {
}

MemoryMappedFileStreamFallback::~MemoryMappedFileStreamFallback() {
    MemoryMappedFileStreamFallback::close();
}

void MemoryMappedFileStreamFallback::open() {
    stream->open();
}

void MemoryMappedFileStreamFallback::close() {
    stream->close();
}

std::uint64_t MemoryMappedFileStreamFallback::tell() {
    return stream->tell();
}

void MemoryMappedFileStreamFallback::seek(std::uint64_t position) {
    stream->seek(position);
}

std::size_t MemoryMappedFileStreamFallback::read(char* destination, std::size_t size) {
    return stream->read(destination, size);
}

std::size_t MemoryMappedFileStreamFallback::read(Writable* destination, std::size_t size) {
    return stream->read(destination, size);
}

std::size_t MemoryMappedFileStreamFallback::write(const char* source, std::size_t size) {
    return stream->write(source, size);
}

std::size_t MemoryMappedFileStreamFallback::write(Readable* source, std::size_t size) {
    return stream->write(source, size);
}

void MemoryMappedFileStreamFallback::flush() {
    stream->flush();
}

void MemoryMappedFileStreamFallback::resize(std::uint64_t size) {
    // Growing is appending zeros through the fronted stream; shrinking has no portable path through it, so that is refused
    // rather than silently ignored
    const std::uint64_t current = stream->size();
    if (size < current) {
        status->set(WriteError);
        return;
    }
    if (size == current) {
        return;
    }

    // Like the mapped backends the position survives the resize
    const std::uint64_t position = stream->tell();
    stream->seek(current);
    static const char zeros[4096] = {};
    for (std::uint64_t remaining = size - current; remaining > 0ul;) {
        const std::size_t chunk = static_cast<std::size_t>(std::min<std::uint64_t>(remaining, sizeof(zeros)));
        if (stream->write(zeros, chunk) != chunk) {
            // The fronted stream has reported the failure and refuses further transfers and seeks until reopened, so a
            // seek back would only replace that report with a SeekError; the position stays where the writing stopped
            return;
        }
        remaining -= chunk;
    }
    stream->seek(position);
}

std::uint64_t MemoryMappedFileStreamFallback::size() {
    return stream->size();
}

const char* MemoryMappedFileStreamFallback::mappedData() {
    return nullptr;
}

std::uint64_t MemoryMappedFileStreamFallback::mappedOffset() {
    return 0ul;
}

std::size_t MemoryMappedFileStreamFallback::mappedSize() {
    return 0ul;
}

MemoryResource* MemoryMappedFileStreamFallback::getMemoryResource() {
    return memRes;
}

}  // namespace trio

#endif
