// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trio/Defs.h"
#include "trio/Stream.h"

#include <cstdint>

namespace trio {

/**
    @brief In-memory stream.
    @note
        The stream's storage is contiguous, so the mapped region always spans the whole stream, and does so regardless of open
        state. Any write that grows the stream may reallocate and thereby invalidate the pointer.
    @note
        A write from a Readable grows the storage to the whole requested size before any byte is read, and keeps only what the
        source delivered. A size taken from untrusted input is therefore committed as memory whether or not that much data
        exists to fill it - validate such sizes before passing them in.
*/
class TRIOAPI_TYPE MemoryStream : public BoundedIOStream, public Mappable {
public:
    /**
        @brief Factory method for creation of a MemoryStream instance.
        @param memRes
            The memory resource to be used for the allocation of the MemoryStream instance.
        @note
            If a custom memory resource is not given, a default allocation mechanism will be used.
        @warning
            User is responsible for releasing the returned pointer by calling destroy.
        @see destroy
    */
    TRIOAPI_MEMBER static MemoryStream* create(MemoryResource* memRes = nullptr);
    /**
        @brief Factory method for creation of a MemoryStream instance.
        @param initialSize
            Initial size of the memory stream.
        @param memRes
            The memory resource to be used for the allocation of the MemoryStream instance.
        @note
            If a custom memory resource is not given, a default allocation mechanism will be used.
        @warning
            User is responsible for releasing the returned pointer by calling destroy.
        @see destroy
    */
    TRIOAPI_MEMBER static MemoryStream* create(std::size_t initialSize, MemoryResource* memRes = nullptr);
    /**
        @brief Method for freeing a MemoryStream instance.
        @param instance
            Instance of MemoryStream to be freed.
        @see create
    */
    TRIOAPI_MEMBER static void destroy(MemoryStream* instance);

    MemoryStream() = default;
    TRIOAPI_MEMBER ~MemoryStream() override;

    MemoryStream(const MemoryStream&) = delete;
    MemoryStream& operator=(const MemoryStream&) = delete;

    MemoryStream(MemoryStream&&) = default;
    MemoryStream& operator=(MemoryStream&&) = default;
};

}  // namespace trio

namespace pma {

template<>
struct DefaultInstanceCreator<trio::MemoryStream> {
    using type = FactoryCreate<trio::MemoryStream>;
};

template<>
struct DefaultInstanceDestroyer<trio::MemoryStream> {
    using type = FactoryDestroy<trio::MemoryStream>;
};

}  // namespace pma
