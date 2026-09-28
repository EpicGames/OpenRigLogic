// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trio/Defs.h"
#include "trio/Stream.h"

#include <cstdint>

namespace trio {

/**
    @brief Memory mapped file stream.
    @note
        The mapped region normally spans the whole file, but a large file may be mapped in smaller views, in which case seeking
        outside the current one remaps and mappedOffset/mappedSize delimit the new region. No view is held at end of file or
        before an empty file has been resized, and mappedData reports nullptr for both.
    @note
        A write that grows the file resizes it before any byte is transferred, so a write that stops early leaves the file at
        its grown size with a zero-filled tail, while returning the number of bytes actually written. Use resize() to trim it.
    @note
        Opening for AccessMode::Write truncates an existing file, as FileStream does; AccessMode::ReadWrite keeps its content.
    @note
        On platforms without a memory mapping facility this stream is backed by a plain FileStream: mappedData reports
        nullptr, flush forwards to it, resize grows the file by appending zeros and refuses to shrink it with WriteError.
*/
class TRIOAPI_TYPE MemoryMappedFileStream : public BoundedIOStream, public Resizable, public Mappable {
public:
    /**
        @brief Factory method for creation of a MemoryMappedFileStream instance.
        @param path
            UTF-8 encoded path to file to be opened.
        @param accessMode
            Control whether the file is opened for reading or writing.
        @param memRes
            The memory resource to be used for the allocation of the MemoryMappedFileStream instance.
        @return
            Pointer to the newly created MemoryMappedFileStream instance, never nullptr.
        @note
            If a custom memory resource is not given, a default allocation mechanism will be used.
        @note
            Creation only queries the size of the given path; opening and mapping are done by open. A missing or unreadable
            path is not an error here (the size reads as zero), so open is the first call that can fail - through trio::Status.
        @warning
            User is responsible for releasing the returned pointer by calling destroy.
        @see destroy
        @see open
    */
    TRIOAPI_MEMBER static MemoryMappedFileStream* create(const char* path,
                                                         AccessMode accessMode,
                                                         MemoryResource* memRes = nullptr);
    /**
        @brief Method for freeing a MemoryMappedFileStream instance.
        @param instance
            Instance of MemoryMappedFileStream to be freed.
        @see create
    */
    TRIOAPI_MEMBER static void destroy(MemoryMappedFileStream* instance);

    MemoryMappedFileStream() = default;
    TRIOAPI_MEMBER ~MemoryMappedFileStream() override;

    MemoryMappedFileStream(const MemoryMappedFileStream&) = delete;
    MemoryMappedFileStream& operator=(const MemoryMappedFileStream&) = delete;

    MemoryMappedFileStream(MemoryMappedFileStream&&) = default;
    MemoryMappedFileStream& operator=(MemoryMappedFileStream&&) = default;
};

}  // namespace trio

namespace pma {

template<>
struct DefaultInstanceCreator<trio::MemoryMappedFileStream> {
    using type = FactoryCreate<trio::MemoryMappedFileStream>;
};

template<>
struct DefaultInstanceDestroyer<trio::MemoryMappedFileStream> {
    using type = FactoryDestroy<trio::MemoryMappedFileStream>;
};

}  // namespace pma
