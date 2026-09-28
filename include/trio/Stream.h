// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trio/Concepts.h"
#include "trio/Defs.h"
#include "trio/types/Aliases.h"
#include "trio/types/Parameters.h"

#include <cstdint>

namespace trio {

class TRIOAPI_TYPE BoundedIOStream : public Controllable,
                                     public Readable,
                                     public Writable,
                                     public Seekable,
                                     public Bounded,
                                     public Buffered {
public:
    using AccessMode = trio::AccessMode;
    using OpenMode = trio::OpenMode;

    TRIOAPI_MEMBER static const sc::StatusCode OpenError;
    TRIOAPI_MEMBER static const sc::StatusCode ReadError;
    TRIOAPI_MEMBER static const sc::StatusCode WriteError;
    TRIOAPI_MEMBER static const sc::StatusCode AlreadyOpenError;
    TRIOAPI_MEMBER static const sc::StatusCode SeekError;

public:
    TRIOAPI_MEMBER virtual ~BoundedIOStream();
    /**
        @brief Flush the changes to filesystem.
        @note
            Streams that hold nothing back inherit this as a no-op, so every stream can be flushed uniformly and existing
            subclasses need not implement it.
    */
    TRIOAPI_MEMBER void flush() override;
};

}  // namespace trio
