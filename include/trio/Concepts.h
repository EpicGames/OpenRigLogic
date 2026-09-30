// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trio/Defs.h"

#include <cstddef>
#include <cstdint>

namespace trio {

class Writable;

class TRIOAPI_TYPE Readable {
public:
    /**
        @brief Read bytes from stream into the given buffer.
        @param destination
            Destination buffer into which the data is going to be read from the stream.
        @param size
            Number of bytes to read from stream.
        @return
            Number of bytes read.
    */
    virtual std::size_t read(char* destination, std::size_t size) = 0;
    /**
        @brief Read bytes from this stream into the given stream.
        @param destination
            Destination stream into which the data is going to be read from this stream.
        @param size
            Number of bytes to read from stream.
        @return
            Number of bytes read.
    */
    virtual std::size_t read(Writable* destination, std::size_t size) = 0;

protected:
    TRIOAPI_MEMBER virtual ~Readable();
};

class TRIOAPI_TYPE Writable {
public:
    /**
        @brief Writes bytes from the given buffer to the stream.
        @param source
            Source buffer from which the data is going to be written to the stream.
        @param size
            Number of bytes to write to the stream.
        @return
            Number of bytes written.
    */
    virtual std::size_t write(const char* source, std::size_t size) = 0;
    /**
        @brief Writes bytes from the given stream to this stream.
        @param source
            Source stream from which the data is going to be written into this stream.
        @param size
            Number of bytes to write to the stream.
        @return
            Number of bytes written.
    */
    virtual std::size_t write(Readable* source, std::size_t size) = 0;

protected:
    TRIOAPI_MEMBER virtual ~Writable();
};

class TRIOAPI_TYPE Seekable {
public:
    /**
        @brief Get the current position in the stream.
        @return
            Position in the stream relative to it's start, with 0 denoting the start position.
    */
    virtual std::uint64_t tell() = 0;
    /**
        @brief Set the current position in the stream.
        @param position
            Position in the stream relative to it's start, with 0 denoting the start position.
    */
    virtual void seek(std::uint64_t position) = 0;

protected:
    TRIOAPI_MEMBER virtual ~Seekable();
};

class TRIOAPI_TYPE Openable {
public:
    /**
        @brief Open access to the stream.
    */
    virtual void open() = 0;

protected:
    TRIOAPI_MEMBER virtual ~Openable();
};

class TRIOAPI_TYPE Closeable {
public:
    /**
        @brief Close access to the stream.
    */
    virtual void close() = 0;

protected:
    TRIOAPI_MEMBER virtual ~Closeable();
};

class TRIOAPI_TYPE Controllable : public Openable, public Closeable {
protected:
    TRIOAPI_MEMBER virtual ~Controllable();
};

class TRIOAPI_TYPE Bounded {
public:
    /**
        @brief Obtain size of stream in bytes.
        @return
            Size in bytes.
    */
    virtual std::uint64_t size() = 0;

protected:
    TRIOAPI_MEMBER virtual ~Bounded();
};

class TRIOAPI_TYPE Buffered {
public:
    /**
        @brief Flush the changes to filesystem.
    */
    virtual void flush() = 0;

protected:
    TRIOAPI_MEMBER virtual ~Buffered();
};

class TRIOAPI_TYPE Resizable {
public:
    /**
        @brief Resize file to the requested size.
    */
    virtual void resize(std::uint64_t size) = 0;

protected:
    TRIOAPI_MEMBER virtual ~Resizable();
};

class TRIOAPI_TYPE Mappable {
public:
    /**
        @brief Direct pointer to the stream's underlying memory.
        @return
            Pointer to the first byte of the currently addressable region, or
            nullptr when the stream exposes no directly addressable memory
            (e.g. it is empty, it is not open, or the platform has no
            memory-mapping backend).
        @note
            The region is delimited by mappedOffset and mappedSize, and is not
            necessarily the whole stream. The pointer stays valid until an
            operation moves or reallocates that storage - for a memory mapped
            file, a seek or read that remaps the view; for an in-memory stream,
            a write that grows the buffer.
        @see mappedOffset
        @see mappedSize
    */
    virtual const char* mappedData() = 0;
    /**
        @brief Offset within the stream at which the region returned by
            mappedData begins.
        @return
            Offset in bytes relative to the start of the stream. This is 0 both
            for a region that begins at the first byte of the stream and for a
            stream with no addressable region at all - mappedData tells the two
            apart by returning nullptr in the latter case.
        @see mappedData
        @see mappedSize
    */
    virtual std::uint64_t mappedOffset() = 0;
    /**
        @brief Number of bytes addressable through the pointer returned by
            mappedData.
        @return
            Extent of the addressable region in bytes, or 0 when no region is
            addressable.
        @note
            This can be less than the whole stream, in which case reading past
            it is undefined even though the stream holds more bytes - seek to
            the wanted offset to bring that part of the stream into the region.
        @see mappedData
        @see mappedOffset
    */
    virtual std::size_t mappedSize() = 0;

protected:
    TRIOAPI_MEMBER virtual ~Mappable();
};

}  // namespace trio
