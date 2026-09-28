// Copyright Epic Games, Inc. All Rights Reserved.

#include "trio/streams/MemoryStreamImpl.h"

#include <pma/PolyAllocator.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cassert>
#include <cstdint>
#include <cstring>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace trio {

namespace {

class MemoryReader : public Readable {
public:
    explicit MemoryReader(const char* source_) :
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

class MemoryWriter : public Writable {
public:
    explicit MemoryWriter(char* destination_) :
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

MemoryStream::~MemoryStream() = default;

MemoryStream* MemoryStream::create(MemoryResource* memRes) {
    return create(0ul, memRes);
}

MemoryStream* MemoryStream::create(std::size_t initialSize, MemoryResource* memRes) {
    pma::PolyAllocator<MemoryStreamImpl> alloc{memRes};
    return alloc.newObject(initialSize, memRes);
}

void MemoryStream::destroy(MemoryStream* instance) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    auto stream = static_cast<MemoryStreamImpl*>(instance);
    pma::PolyAllocator<MemoryStreamImpl> alloc{stream->getMemoryResource()};
    alloc.deleteObject(stream);
}

MemoryStreamImpl::MemoryStreamImpl(std::size_t initialSize, MemoryResource* memRes_) :
    buffer{initialSize, static_cast<char>(0), memRes_},
    position{},
    memRes{memRes_} {
}

void MemoryStreamImpl::open() {
    position = 0ul;
}

void MemoryStreamImpl::close() {
    position = 0ul;
}

std::uint64_t MemoryStreamImpl::tell() {
    return position;
}

void MemoryStreamImpl::seek(std::uint64_t position_) {
    if ((position_ == 0ul) || (position_ <= size())) {
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
        position = static_cast<std::size_t>(position_);
#if !defined(__clang__) && defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
    } else {
        status->set(SeekError);
    }
}

std::size_t MemoryStreamImpl::read(char* destination, std::size_t size) {
    if (destination == nullptr) {
        status->set(ReadError);
        return 0ul;
    }

    MemoryWriter writer{destination};
    return read(&writer, size);
}

std::size_t MemoryStreamImpl::read(Writable* destination, std::size_t size) {
    if (destination == nullptr) {
        status->set(ReadError);
        return 0ul;
    }

    const std::size_t available = buffer.size() - position;
    const std::size_t bytesToRead = std::min(size, available);
    // A destination may take the bytes in pieces; only one that takes nothing has stopped consuming, which is an error as
    // in the file streams
    std::size_t bytesCopied = 0ul;
    while (bytesCopied != bytesToRead) {
        const std::size_t remaining = bytesToRead - bytesCopied;
        // Not trusted beyond what it was handed - an overshoot would put the position past the end of the buffer
        const std::size_t accepted = std::min(destination->write(&buffer[position + bytesCopied], remaining), remaining);
        if (accepted == 0ul) {
            status->set(ReadError);
            break;
        }
        bytesCopied += accepted;
    }
    position += bytesCopied;
    return bytesCopied;
}

std::size_t MemoryStreamImpl::write(const char* source, std::size_t size) {
    if (source == nullptr) {
        status->set(WriteError);
        return 0ul;
    }

    MemoryReader reader{source};
    return write(&reader, size);
}

std::size_t MemoryStreamImpl::write(Readable* source, std::size_t size) {
    if (source == nullptr) {
        status->set(WriteError);
        return 0ul;
    }
    const std::size_t previousSize = buffer.size();
    const std::size_t available = previousSize - position;
    if (available < size) {
        const std::size_t newSize = previousSize + (size - available);
        // Check for overflow / wrap-around
        if (newSize < previousSize) {
            status->set(WriteError);
            return 0ul;
        }
        // Deliberately the whole request: the source reads straight into the buffer, so the room has to exist beforehand.
        // That makes any caller supplied size an allocation of that size, which is documented on the class.
        buffer.resize(newSize);
    }
    // A source may deliver in pieces; only one that delivers nothing has stopped producing, which is an error as in the
    // file streams
    std::size_t bytesCopied = 0ul;
    while (bytesCopied != size) {
        const std::size_t remaining = size - bytesCopied;
        // Not trusted beyond what it was handed - an overshoot would put the position past what the resize made room for
        const std::size_t produced = std::min(source->read(&buffer[position + bytesCopied], remaining), remaining);
        if (produced == 0ul) {
            status->set(WriteError);
            break;
        }
        bytesCopied += produced;
    }
    position += bytesCopied;
    // The source may have produced less than the buffer was grown for - give the untouched tail back rather than let size()
    // and mappedData() report it as payload, but never shrink below what the buffer held on entry
    buffer.resize(std::max(previousSize, position));
    return bytesCopied;
}

std::uint64_t MemoryStreamImpl::size() {
    return buffer.size();
}

const char* MemoryStreamImpl::mappedData() {
    return (buffer.empty() ? nullptr : buffer.data());
}

std::uint64_t MemoryStreamImpl::mappedOffset() {
    // The whole buffer is addressable from its first byte, so this is fixed at zero and does not follow the position
    return 0ul;
}

std::size_t MemoryStreamImpl::mappedSize() {
    return buffer.size();
}

MemoryResource* MemoryStreamImpl::getMemoryResource() {
    return memRes;
}

}  // namespace trio
