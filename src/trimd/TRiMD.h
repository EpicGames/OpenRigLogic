// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trimd/Platform.h"
// Includes that go after platform detection macros
#include "trimd/AVX.h"
#include "trimd/AVX512.h"
#include "trimd/NEON.h"
#include "trimd/SSE.h"
#include "trimd/Scalar.h"

namespace trimd {

#if defined(TRIMD_ENABLE_AVX512F)
using F512 = avx512::F512;
using F512Fast = avx512::F512Fast;
using avx512::abs;
using avx512::andnot;
using avx512::fma;
using avx512::rsqrt;
#elif defined(TRIMD_ENABLE_AVX)
using F512 = fallback::T512<avx::F256>;
using F512Fast = fallback::T512<avx::F256Fast>;
using fallback::abs;
using fallback::andnot;
using fallback::fma;
using fallback::rsqrt;
#elif defined(TRIMD_ENABLE_SSE)
using F512 = fallback::T512<sse::F256>;
using F512Fast = fallback::T512<sse::F256Fast>;
#elif defined(TRIMD_ENABLE_NEON)
using F512 = fallback::T512<neon::F256>;
using F512Fast = fallback::T512<neon::F256Fast>;
#else
using F512 = fallback::T512<scalar::F256>;
using F512Fast = fallback::T512<scalar::F256Fast>;
#endif  // TRIMD_ENABLE_AVX512F

#if defined(TRIMD_ENABLE_AVX)
using F256 = avx::F256;
using F256Fast = avx::F256Fast;
using avx::abs;
using avx::andnot;
using avx::fma;
using avx::rsqrt;
using avx::transpose;
#elif defined(TRIMD_ENABLE_SSE)
using F256 = sse::F256;
using F256Fast = sse::F256Fast;
#elif defined(TRIMD_ENABLE_NEON)
using F256 = neon::F256;
using F256Fast = neon::F256Fast;
#else
using F256 = scalar::F256;
using F256Fast = scalar::F256Fast;
#endif  // TRIMD_ENABLE_AVX

#if defined(TRIMD_ENABLE_SSE)
using F128 = sse::F128;
using F128Fast = sse::F128Fast;
using sse::abs;
using sse::andnot;
using sse::fma;
using sse::rsqrt;
using sse::transpose;
#elif defined(TRIMD_ENABLE_NEON)
using F128 = neon::F128;
using F128Fast = neon::F128Fast;
using neon::abs;
using neon::andnot;
using neon::fma;
using neon::rsqrt;
using neon::transpose;
#else
using F128 = scalar::F128;
using F128Fast = scalar::F128Fast;
#endif  // TRIMD_ENABLE_SSE

using scalar::abs;
using scalar::andnot;
using scalar::fma;
using scalar::rsqrt;
using scalar::transpose;

}  // namespace trimd
