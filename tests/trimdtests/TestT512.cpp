// Copyright Epic Games, Inc. All Rights Reserved.

#include "trimdtests/Defs.h"

#include "trimd/TRiMD.h"

#include <cstring>
#include <type_traits>

// Type list mirrors the F512 alias chain in TRiMD.h: each available F256 (and
// the native AVX-512 type when enabled) gets a T512 variant exercised here.
// scalar fallback is always present.
#if defined(TRIMD_ENABLE_AVX512F) && defined(TRIMD_ENABLE_AVX) && defined(TRIMD_ENABLE_SSE) && defined(TRIMD_ENABLE_NEON)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>,
                                   trimd::fallback::T512<trimd::sse::F256>,
                                   trimd::fallback::T512<trimd::avx::F256>,
                                   trimd::fallback::T512<trimd::neon::F256>,
                                   trimd::avx512::F512>;
#elif defined(TRIMD_ENABLE_AVX512F) && defined(TRIMD_ENABLE_AVX) && defined(TRIMD_ENABLE_SSE)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>,
                                   trimd::fallback::T512<trimd::sse::F256>,
                                   trimd::fallback::T512<trimd::avx::F256>,
                                   trimd::avx512::F512>;
#elif defined(TRIMD_ENABLE_AVX) && defined(TRIMD_ENABLE_SSE) && defined(TRIMD_ENABLE_NEON)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>,
                                   trimd::fallback::T512<trimd::sse::F256>,
                                   trimd::fallback::T512<trimd::avx::F256>,
                                   trimd::fallback::T512<trimd::neon::F256>>;
#elif defined(TRIMD_ENABLE_AVX) && defined(TRIMD_ENABLE_SSE)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>,
                                   trimd::fallback::T512<trimd::sse::F256>,
                                   trimd::fallback::T512<trimd::avx::F256>>;
#elif defined(TRIMD_ENABLE_AVX)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>, trimd::fallback::T512<trimd::avx::F256>>;
#elif defined(TRIMD_ENABLE_SSE)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>, trimd::fallback::T512<trimd::sse::F256>>;
#elif defined(TRIMD_ENABLE_NEON)
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>, trimd::fallback::T512<trimd::neon::F256>>;
#else
using T512Types = ::testing::Types<trimd::fallback::T512<trimd::scalar::F256>>;
#endif

template<typename TF512>
static TF512 frombits(uint32_t b0,
                      uint32_t b1,
                      uint32_t b2,
                      uint32_t b3,
                      uint32_t b4,
                      uint32_t b5,
                      uint32_t b6,
                      uint32_t b7,
                      uint32_t b8,
                      uint32_t b9,
                      uint32_t b10,
                      uint32_t b11,
                      uint32_t b12,
                      uint32_t b13,
                      uint32_t b14,
                      uint32_t b15) {
    return TF512{trimd::bitcast<float>(b0),
                 trimd::bitcast<float>(b1),
                 trimd::bitcast<float>(b2),
                 trimd::bitcast<float>(b3),
                 trimd::bitcast<float>(b4),
                 trimd::bitcast<float>(b5),
                 trimd::bitcast<float>(b6),
                 trimd::bitcast<float>(b7),
                 trimd::bitcast<float>(b8),
                 trimd::bitcast<float>(b9),
                 trimd::bitcast<float>(b10),
                 trimd::bitcast<float>(b11),
                 trimd::bitcast<float>(b12),
                 trimd::bitcast<float>(b13),
                 trimd::bitcast<float>(b14),
                 trimd::bitcast<float>(b15)};
}

// Returns a 16-element mask where only `oneAt` is 0xFFFFFFFF and the rest are 0.
template<typename TF512>
static TF512 maskOneSet(std::size_t oneAt) {
    uint32_t bits[16] = {};
    bits[oneAt] = 0xFFFFFFFFu;
    return frombits<TF512>(bits[0],
                           bits[1],
                           bits[2],
                           bits[3],
                           bits[4],
                           bits[5],
                           bits[6],
                           bits[7],
                           bits[8],
                           bits[9],
                           bits[10],
                           bits[11],
                           bits[12],
                           bits[13],
                           bits[14],
                           bits[15]);
}

// Same pattern but inverted: `zeroAt` is 0, rest are 0xFFFFFFFF.
template<typename TF512>
static TF512 maskOneClear(std::size_t zeroAt) {
    uint32_t bits[16] = {0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu,
                         0xFFFFFFFFu};
    bits[zeroAt] = 0u;
    return frombits<TF512>(bits[0],
                           bits[1],
                           bits[2],
                           bits[3],
                           bits[4],
                           bits[5],
                           bits[6],
                           bits[7],
                           bits[8],
                           bits[9],
                           bits[10],
                           bits[11],
                           bits[12],
                           bits[13],
                           bits[14],
                           bits[15]);
}

// Bit-level equality via storage round-trip. Plain SIMD stores preserve all 32
// bits per lane (no FP semantics applied), so this matches per-storage memcmp.
template<typename T>
static bool equal(const T& lhs, const T& rhs) {
    float l[T::size()] = {};
    float r[T::size()] = {};
    lhs.unalignedStore(l);
    rhs.unalignedStore(r);
    return std::memcmp(l, r, sizeof(l)) == 0;
}

template<typename T>
class T512Test : public ::testing::Test {
protected:
    using T512 = T;

    void SetUp() override {
#ifdef TRIMD_ENABLE_AVX512F
        if (std::is_same<T512, trimd::avx512::F512>::value) {
            const trimd::CPUFeatures features = trimd::getCPUFeatures();
            if (!features.AVX512F) {
                GTEST_SKIP() << "AVX-512F not supported on this CPU";
            }
        }
#endif  // TRIMD_ENABLE_AVX512F
    }
};

TYPED_TEST_SUITE(T512Test, T512Types, );

TYPED_TEST(T512Test, CheckSize) {
    ASSERT_EQ(TestFixture::T512::size(), 16ul);
}

TYPED_TEST(T512Test, Equality) {
    using F512 = typename TestFixture::T512;

    F512 v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 vsame{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);
    ASSERT_TRUE(equal(v == vsame, allset));

    // Verify each lane is independently mask-driven: differ in exactly one lane,
    // expect that lane bit clear in the equality mask.
    for (std::size_t lane = 0; lane < 16; ++lane) {
        F512 diff = vsame;
        float bumped[16];
        diff.unalignedStore(bumped);
        bumped[lane] += 0.5f;
        diff.unalignedLoad(bumped);
        ASSERT_TRUE(equal(v == diff, maskOneClear<F512>(lane))) << "lane " << lane;
    }
}

TYPED_TEST(T512Test, Inequality) {
    using F512 = typename TestFixture::T512;

    F512 v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 vsame{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    ASSERT_TRUE(equal(v != vsame, allclear));

    for (std::size_t lane = 0; lane < 16; ++lane) {
        F512 diff = vsame;
        float bumped[16];
        diff.unalignedStore(bumped);
        bumped[lane] += 0.5f;
        diff.unalignedLoad(bumped);
        ASSERT_TRUE(equal(v != diff, maskOneSet<F512>(lane))) << "lane " << lane;
    }
}

TYPED_TEST(T512Test, LessThan) {
    using F512 = typename TestFixture::T512;

    F512 v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 v1same = v1;
    F512 vlarger{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    F512 vsmaller{0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f};

    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);

    ASSERT_TRUE(equal(v1 < v1same, allclear));
    ASSERT_TRUE(equal(v1 < vlarger, allset));
    ASSERT_TRUE(equal(v1 < vsmaller, allclear));
}

TYPED_TEST(T512Test, LessThanOrEqual) {
    using F512 = typename TestFixture::T512;

    F512 v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 v1same = v1;
    F512 vlarger{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    F512 vsmaller{0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f};

    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);

    ASSERT_TRUE(equal(v1 <= v1same, allset));
    ASSERT_TRUE(equal(v1 <= vlarger, allset));
    ASSERT_TRUE(equal(v1 <= vsmaller, allclear));
}

TYPED_TEST(T512Test, GreaterThan) {
    using F512 = typename TestFixture::T512;

    F512 v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 v1same = v1;
    F512 vlarger{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    F512 vsmaller{0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f};

    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);

    ASSERT_TRUE(equal(v1 > v1same, allclear));
    ASSERT_TRUE(equal(v1 > vlarger, allclear));
    ASSERT_TRUE(equal(v1 > vsmaller, allset));
}

TYPED_TEST(T512Test, GreaterThanOrEqual) {
    using F512 = typename TestFixture::T512;

    F512 v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 v1same = v1;
    F512 vlarger{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    F512 vsmaller{0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f};

    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);

    ASSERT_TRUE(equal(v1 >= v1same, allset));
    ASSERT_TRUE(equal(v1 >= vlarger, allclear));
    ASSERT_TRUE(equal(v1 >= vsmaller, allset));
}

TYPED_TEST(T512Test, BitwiseAND) {
    using F512 = typename TestFixture::T512;
    F512 v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 zeros{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    ASSERT_TRUE(equal(v & allclear, zeros));

    // Per-lane bit-mask preserves only the lane the mask has set.
    for (std::size_t lane = 0; lane < 16; ++lane) {
        F512 mask = maskOneSet<F512>(lane);
        float expected[16] = {};
        expected[lane] = static_cast<float>(lane + 1);
        F512 e;
        e.unalignedLoad(expected);
        ASSERT_TRUE(equal(v & mask, e)) << "lane " << lane;
    }
}

TYPED_TEST(T512Test, BitwiseOR) {
    using F512 = typename TestFixture::T512;
    F512 v1{0.0f, 2.0f, 0.0f, 4.0f, 0.0f, 6.0f, 0.0f, 8.0f, 0.0f, 10.0f, 0.0f, 12.0f, 0.0f, 14.0f, 0.0f, 16.0f};
    F512 v2{1.0f, 0.0f, 3.0f, 0.0f, 5.0f, 0.0f, 7.0f, 0.0f, 9.0f, 0.0f, 11.0f, 0.0f, 13.0f, 0.0f, 15.0f, 0.0f};
    F512 expected{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    ASSERT_TRUE(equal(v1 | v2, expected));
}

TYPED_TEST(T512Test, BitwiseXOR) {
    using F512 = typename TestFixture::T512;
    F512 v1{0.0f, 2.0f, 0.0f, 4.0f, 0.0f, 6.0f, 0.0f, 8.0f, 0.0f, 10.0f, 0.0f, 12.0f, 0.0f, 14.0f, 0.0f, 16.0f};
    F512 v2{0.0f, 0.0f, 3.0f, 0.0f, 5.0f, 0.0f, 7.0f, 8.0f, 0.0f, 0.0f, 11.0f, 0.0f, 13.0f, 0.0f, 15.0f, 16.0f};
    F512 expected{0.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 0.0f, 0.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 0.0f};
    ASSERT_TRUE(equal(v1 ^ v2, expected));
}

TYPED_TEST(T512Test, BitwiseNOT) {
    using F512 = typename TestFixture::T512;
    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);

    ASSERT_TRUE(equal(~allclear, allset));
    ASSERT_TRUE(equal(~allset, allclear));
}

TYPED_TEST(T512Test, ConstructFromArgs) {
    typename TestFixture::T512
        v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512
        expected{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    ASSERT_TRUE(equal(v, expected));
}

TYPED_TEST(T512Test, ConstructFromSingleValue) {
    typename TestFixture::T512 v{42.0f};
    typename TestFixture::T512
        expected{42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f, 42.0f};
    ASSERT_TRUE(equal(v, expected));
}

TYPED_TEST(T512Test, FromAlignedSource) {
    alignas(TestFixture::T512::alignment()) const float expected[] =
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    auto v = TestFixture::T512::fromAlignedSource(expected);

    alignas(TestFixture::T512::alignment()) float result[TestFixture::T512::size()];
    v.alignedStore(result);

    ASSERT_ELEMENTS_EQ(result, expected, TestFixture::T512::size());
}

TYPED_TEST(T512Test, AlignedLoadStore) {
    alignas(TestFixture::T512::alignment()) const float expected[] =
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512 v;
    v.alignedLoad(expected);

    alignas(TestFixture::T512::alignment()) float result[TestFixture::T512::size()];
    v.alignedStore(result);

    ASSERT_ELEMENTS_EQ(result, expected, TestFixture::T512::size());
}

TYPED_TEST(T512Test, FromUnalignedSource) {
    const float expected[] =
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    auto v = TestFixture::T512::fromUnalignedSource(expected);

    float result[TestFixture::T512::size()];
    v.unalignedStore(result);

    ASSERT_ELEMENTS_EQ(result, expected, TestFixture::T512::size());
}

TYPED_TEST(T512Test, UnalignedLoadStore) {
    const float expected[] =
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512 v;
    v.unalignedLoad(expected);

    float result[TestFixture::T512::size()];
    v.unalignedStore(result);

    ASSERT_ELEMENTS_EQ(result, expected, TestFixture::T512::size());
}

TYPED_TEST(T512Test, LoadSingleValue) {
    const float source[] =
        {42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f, 57.0f};
    auto v = TestFixture::T512::loadSingleValue(source);
    typename TestFixture::T512
        expected{42.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    ASSERT_TRUE(equal(v, expected));
}

TYPED_TEST(T512Test, Sum) {
    typename TestFixture::T512
        v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    // 1 + 2 + ... + 16 = 136
    ASSERT_EQ(v.sum(), 136.0f);
}

TYPED_TEST(T512Test, CompoundAssignmentAdd) {
    typename TestFixture::T512
        v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512
        v2{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    typename TestFixture::T512
        expected{3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f, 17.0f, 19.0f, 21.0f, 23.0f, 25.0f, 27.0f, 29.0f, 31.0f, 33.0f};
    v1 += v2;
    ASSERT_TRUE(equal(v1, expected));
}

TYPED_TEST(T512Test, CompoundAssignmentSub) {
    typename TestFixture::T512
        v1{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    typename TestFixture::T512
        v2{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512
        expected{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    v1 -= v2;
    ASSERT_TRUE(equal(v1, expected));
}

TYPED_TEST(T512Test, CompoundAssignmentMul) {
    typename TestFixture::T512
        v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512 v2{2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    typename TestFixture::T512
        expected{2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f};
    v1 *= v2;
    ASSERT_TRUE(equal(v1, expected));
}

TYPED_TEST(T512Test, CompoundAssignmentDiv) {
    typename TestFixture::T512
        v1{4.0f, 6.0f, 9.0f, 12.0f, 4.0f, 6.0f, 9.0f, 12.0f, 4.0f, 6.0f, 9.0f, 12.0f, 4.0f, 6.0f, 9.0f, 12.0f};
    typename TestFixture::T512 v2{1.0f, 2.0f, 3.0f, 3.0f, 1.0f, 2.0f, 3.0f, 3.0f, 1.0f, 2.0f, 3.0f, 3.0f, 1.0f, 2.0f, 3.0f, 3.0f};
    float expected[TestFixture::T512::size()] =
        {4.0f, 3.0f, 3.0f, 4.0f, 4.0f, 3.0f, 3.0f, 4.0f, 4.0f, 3.0f, 3.0f, 4.0f, 4.0f, 3.0f, 3.0f, 4.0f};
    v1 /= v2;

    float result[TestFixture::T512::size()];
    v1.unalignedStore(result);
    ASSERT_ELEMENTS_NEAR(result, expected, TestFixture::T512::size(), 0.0001f);
}

TYPED_TEST(T512Test, OperatorAdd) {
    typename TestFixture::T512
        v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512
        v2{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    typename TestFixture::T512
        expected{3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f, 17.0f, 19.0f, 21.0f, 23.0f, 25.0f, 27.0f, 29.0f, 31.0f, 33.0f};
    auto v3 = v1 + v2;
    ASSERT_TRUE(equal(v3, expected));
}

TYPED_TEST(T512Test, OperatorSub) {
    typename TestFixture::T512
        v1{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f};
    typename TestFixture::T512
        v2{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512
        expected{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    auto v3 = v1 - v2;
    ASSERT_TRUE(equal(v3, expected));
}

TYPED_TEST(T512Test, OperatorMul) {
    typename TestFixture::T512
        v1{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    typename TestFixture::T512 v2{2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    typename TestFixture::T512
        expected{2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 32.0f};
    auto v3 = v1 * v2;
    ASSERT_TRUE(equal(v3, expected));
}

TYPED_TEST(T512Test, OperatorDiv) {
    typename TestFixture::T512
        v1{4.0f, 6.0f, 9.0f, 12.0f, 4.0f, 6.0f, 9.0f, 12.0f, 4.0f, 6.0f, 9.0f, 12.0f, 4.0f, 6.0f, 9.0f, 12.0f};
    typename TestFixture::T512 v2{1.0f, 2.0f, 3.0f, 3.0f, 1.0f, 2.0f, 3.0f, 3.0f, 1.0f, 2.0f, 3.0f, 3.0f, 1.0f, 2.0f, 3.0f, 3.0f};
    float expected[TestFixture::T512::size()] =
        {4.0f, 3.0f, 3.0f, 4.0f, 4.0f, 3.0f, 3.0f, 4.0f, 4.0f, 3.0f, 3.0f, 4.0f, 4.0f, 3.0f, 3.0f, 4.0f};
    auto v3 = v1 / v2;

    float result[TestFixture::T512::size()];
    v3.unalignedStore(result);
    ASSERT_ELEMENTS_NEAR(result, expected, TestFixture::T512::size(), 0.0001f);
}

// Free-function arms (abs, andnot, rsqrt, fma) live in trimd::avx512:: for the
// native arm and trimd::fallback:: for everything else. The top-level F512
// alias pulls one set in via TRiMD.h; here we exercise both flavors explicitly
// where they're reachable so each codegen path is covered.

TEST(T512Test, AbsFallbackScalar) {
    using F512 = trimd::fallback::T512<trimd::scalar::F256>;
    F512 v{-1.0f, 2.0f, -3.0f, 0.0f, -1.0f, 2.0f, -3.0f, 0.0f, -1.0f, 2.0f, -3.0f, 0.0f, -1.0f, 2.0f, -3.0f, 0.0f};
    v = trimd::fallback::abs(v);
    F512 e{1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f, 0.0f};
    ASSERT_TRUE(equal(v, e));
}

TEST(T512Test, AndNotFallbackScalar) {
    using F512 = trimd::fallback::T512<trimd::scalar::F256>;
    F512 v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);
    F512 zeros{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    F512 same = v;
    ASSERT_TRUE(equal(trimd::fallback::andnot(allclear, v), same));
    ASSERT_TRUE(equal(trimd::fallback::andnot(allset, v), zeros));
}

TEST(T512Test, RsqrtFallbackScalar) {
    using F512 = trimd::fallback::T512<trimd::scalar::F256>;
    F512 v{1.0f, 2.0f, 3.0f, 9.0f, 9.0f, 3.0f, 2.0f, 1.0f, 1.0f, 2.0f, 3.0f, 9.0f, 9.0f, 3.0f, 2.0f, 1.0f};
    v = trimd::fallback::rsqrt(v);
    F512 e{1.0f,
           0.70710678f,
           0.57735026f,
           0.33333333f,
           0.33333333f,
           0.57735026f,
           0.70710678f,
           1.0f,
           1.0f,
           0.70710678f,
           0.57735026f,
           0.33333333f,
           0.33333333f,
           0.57735026f,
           0.70710678f,
           1.0f};
#ifdef TRIMD_ENABLE_FAST_INVERSE_SQRT
    static constexpr float threshold = 0.0004f;
#else
    static constexpr float threshold = 0.0002f;
#endif  // TRIMD_ENABLE_FAST_INVERSE_SQRT
    float a[F512::size()];
    float b[F512::size()];
    v.unalignedStore(a);
    e.unalignedStore(b);
    ASSERT_ELEMENTS_NEAR(a, b, F512::size(), threshold);
}

#ifdef TRIMD_ENABLE_AVX512F
TEST(T512Test, AbsAVX512) {
    if (!trimd::getCPUFeatures().AVX512F) {
        GTEST_SKIP() << "AVX-512F not supported on this CPU";
    }
    trimd::avx512::F512 v{-1.0f, 2.0f, -3.0f, 0.0f, -1.0f, 2.0f, -3.0f, 0.0f, -1.0f, 2.0f, -3.0f, 0.0f, -1.0f, 2.0f, -3.0f, 0.0f};
    v = trimd::avx512::abs(v);
    trimd::avx512::F512 e{1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f, 0.0f, 1.0f, 2.0f, 3.0f, 0.0f};
    ASSERT_TRUE(equal(v, e));
}

TEST(T512Test, AndNotAVX512) {
    if (!trimd::getCPUFeatures().AVX512F) {
        GTEST_SKIP() << "AVX-512F not supported on this CPU";
    }
    using F512 = trimd::avx512::F512;
    F512 v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    F512 allclear = frombits<F512>(0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    F512 allset = frombits<F512>(0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);
    F512 zeros{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    F512 same = v;
    ASSERT_TRUE(equal(trimd::avx512::andnot(allclear, v), same));
    ASSERT_TRUE(equal(trimd::avx512::andnot(allset, v), zeros));
}

TEST(T512Test, RsqrtAVX512) {
    if (!trimd::getCPUFeatures().AVX512F) {
        GTEST_SKIP() << "AVX-512F not supported on this CPU";
    }
    trimd::avx512::F512 v{1.0f, 2.0f, 3.0f, 9.0f, 9.0f, 3.0f, 2.0f, 1.0f, 1.0f, 2.0f, 3.0f, 9.0f, 9.0f, 3.0f, 2.0f, 1.0f};
    v = trimd::avx512::rsqrt(v);
    trimd::avx512::F512 e{1.0f,
                          0.70710678f,
                          0.57735026f,
                          0.33333333f,
                          0.33333333f,
                          0.57735026f,
                          0.70710678f,
                          1.0f,
                          1.0f,
                          0.70710678f,
                          0.57735026f,
                          0.33333333f,
                          0.33333333f,
                          0.57735026f,
                          0.70710678f,
                          1.0f};
    // rsqrt14 gives ~14-bit relative precision; widen threshold accordingly.
    #ifdef TRIMD_ENABLE_FAST_INVERSE_SQRT
    static constexpr float threshold = 0.0004f;
    #else
    static constexpr float threshold = 0.0003f;
    #endif  // TRIMD_ENABLE_FAST_INVERSE_SQRT
    float a[trimd::avx512::F512::size()];
    float b[trimd::avx512::F512::size()];
    v.unalignedStore(a);
    e.unalignedStore(b);
    ASSERT_ELEMENTS_NEAR(a, b, trimd::avx512::F512::size(), threshold);
}

TEST(T512Test, FmaAVX512) {
    if (!trimd::getCPUFeatures().AVX512F) {
        GTEST_SKIP() << "AVX-512F not supported on this CPU";
    }
    trimd::avx512::F512 a{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    trimd::avx512::F512 b{2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    trimd::avx512::F512 c{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    trimd::avx512::F512
        expected{3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f, 17.0f, 19.0f, 21.0f, 23.0f, 25.0f, 27.0f, 29.0f, 31.0f, 33.0f};
    ASSERT_TRUE(equal(trimd::avx512::fma(a, b, c), expected));
}
#endif  // TRIMD_ENABLE_AVX512F
