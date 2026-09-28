// Copyright Epic Games, Inc. All Rights Reserved.

#include "tersetests/Defs.h"

#include "terse/archives/Traits.h"

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstdint>
#include <vector>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace {

enum class SomeEnum : std::uint16_t {
};

}  // namespace

static_assert(terse::traits::is_batchable<std::vector<std::uint8_t>>::value, "arithmetic elements are batchable");
static_assert(terse::traits::is_batchable<std::vector<float>>::value, "arithmetic elements are batchable");
static_assert(terse::traits::is_batchable<std::vector<SomeEnum>>::value, "enum elements are batchable");
static_assert(!terse::traits::is_batchable<std::vector<bool>>::value, "bool has no stable wire representation");
// Pointers are scalar types, but raw addresses must never reach the wire.
static_assert(!terse::traits::is_batchable<std::vector<int*>>::value, "pointer elements are not batchable");
static_assert(!terse::traits::is_batchable<std::vector<std::nullptr_t>>::value, "nullptr_t elements are not batchable");
