// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trio/Defs.h"
#include "trio/Stream.h"

namespace trio {

/**
    @brief Standard file stream.
    @note
        Writes are buffered; they reach the file on flush or close.
    @note
        Size and position describe the file as this stream sees it; a file modified through another handle while the stream
        is open is not tracked, and what such reads return depends on the C runtime.
*/
class TRIOAPI_TYPE FileStream : public BoundedIOStream {
public:
    /**
        @brief Factory method for creation of a FileStream instance.
        @param path
            UTF-8 encoded path to file to be opened.
        @param accessMode
            Control whether the file is opened for reading or writing.
        @param openMode
            Kept for source compatibility; the file is always opened in binary mode. OpenMode::Text is deprecated and
            behaves as OpenMode::Binary, so no newline translation takes place and positions are exact byte offsets.
        @param memRes
            The memory resource to be used for the allocation of the FileStream instance.
        @return
            Pointer to the newly created FileStream instance, never nullptr.
        @note
            If a custom memory resource is not given, a default allocation mechanism will be used.
        @note
            Creation only queries the size of the given path; opening is done by open. A missing or unreadable path is not an
            error here (the size reads as zero), so open is the first call that can fail - through trio::Status.
        @warning
            User is responsible for releasing the returned pointer by calling destroy.
        @see destroy
        @see open
    */
    TRIOAPI_MEMBER static FileStream* create(const char* path,
                                             AccessMode accessMode,
                                             OpenMode openMode,
                                             MemoryResource* memRes = nullptr);
    /**
        @brief Method for freeing a FileStream instance.
        @param instance
            Instance of FileStream to be freed.
        @see create
    */
    TRIOAPI_MEMBER static void destroy(FileStream* instance);

    FileStream() = default;
    TRIOAPI_MEMBER ~FileStream() override;

    FileStream(const FileStream&) = delete;
    FileStream& operator=(const FileStream&) = delete;

    FileStream(FileStream&&) = default;
    FileStream& operator=(FileStream&&) = default;
};

}  // namespace trio

namespace pma {

template<>
struct DefaultInstanceCreator<trio::FileStream> {
    using type = FactoryCreate<trio::FileStream>;
};

template<>
struct DefaultInstanceDestroyer<trio::FileStream> {
    using type = FactoryDestroy<trio::FileStream>;
};

}  // namespace pma
