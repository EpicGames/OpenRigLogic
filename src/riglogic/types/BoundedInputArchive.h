// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/types/Aliases.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <terse/Archive.h>
#include <terse/archives/binary/InputArchive.h>

#include <cstddef>
#include <cstdint>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace rl4 {

// Checks every deserialized container length against the stream size before allocating: a larger length is
// corruption, so boundSize() returns 0 and sets malformed, and a truncated or tampered snapshot fails cleanly
// (isOk() == false) instead of attempting a huge allocation.
class BoundedInputArchive : public terse::ExtendableBinaryInputArchive<BoundedInputArchive,
                                                                       BoundedIOStream,
                                                                       std::uint32_t,
                                                                       std::uint32_t,
                                                                       terse::Endianness::Network> {
private:
    using BaseArchive = terse::ExtendableBinaryInputArchive<BoundedInputArchive,
                                                            BoundedIOStream,
                                                            std::uint32_t,
                                                            std::uint32_t,
                                                            terse::Endianness::Network>;
    friend terse::Archive<BoundedInputArchive>;
    friend BaseArchive;

public:
    explicit BoundedInputArchive(BoundedIOStream* stream_) :
        BaseArchive{this, stream_},
        stream{stream_},
        malformed{false} {
    }

    bool isOk() {
        // BaseArchive carries its own malformed state (e.g. an invalid bool wire byte) - shadowing
        // isOk() must not mask it.
        return !malformed && BaseArchive::isOk();
    }

    // Flag corruption from a higher-level check boundSize() cannot express (e.g. a deserialized offset outside its
    // container). Flagging does not abort deserialization: restore() streams the whole chain and then bails on
    // isOk() before any RigLogicImpl is built, so a flagged evaluator is never evaluated.
    void markMalformed() {
        malformed = true;
    }

    std::size_t boundSize(std::size_t size) {
        if (size > stream->size()) {
            malformed = true;
            return 0ul;
        }
        return size;
    }

private:
    BoundedIOStream* stream;
    bool malformed;
};

}  // namespace rl4
