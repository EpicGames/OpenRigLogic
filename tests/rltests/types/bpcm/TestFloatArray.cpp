// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/SerializationContext.h"
#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/system/simd/Utils.h"
#include "riglogic/types/BoundedInputArchive.h"
#include "riglogic/types/bpcm/FloatArray.h"

#include <cstdint>

namespace {

#ifdef RL_BUILD_WITH_HALF_FLOATS

// The half-float archive format is fp32, so both directions stage through fp32; that staging buffer must be
// released afterwards, or every restored FloatArray keeps dead storage twice the size of its live fp16 data.
TEST(FloatArrayTest, HalfFloatRoundTripReleasesFloatStaging) {
    rl4::Configuration config;
    config.floatingPointType = rl4::FloatingPointType::HalfFloat;
    if (rl4::getActiveFeatures(config).floatingPointType != rl4::FloatingPointType::HalfFloat) {
        GTEST_SKIP() << "CPU does not support half floats at runtime.";
    }
    rl4::SerializationContext context{&config, nullptr};
    pma::AlignedMemoryResource memRes;

    rl4::FloatArray dumped{&memRes};
    dumped.fp16.resize(16ul, static_cast<std::uint16_t>(0x3C00u));
    auto stream = pma::makeScoped<trio::MemoryStream>();
    terse::BinaryOutputArchive<rl4::BoundedIOStream> output{stream.get()};
    output.setUserData(&context);
    output << dumped;
    ASSERT_EQ(dumped.fp32.capacity(), 0ul);

    stream->seek(0ul);
    rl4::FloatArray restored{&memRes};
    rl4::BoundedInputArchive input{stream.get()};
    input.setUserData(&context);
    input >> restored;
    ASSERT_TRUE(input.isOk());
    ASSERT_EQ(restored.fp16, dumped.fp16);
    ASSERT_EQ(restored.fp32.capacity(), 0ul);
}

#endif  // RL_BUILD_WITH_HALF_FLOATS

}  // namespace
