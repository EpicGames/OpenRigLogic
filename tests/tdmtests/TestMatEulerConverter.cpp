// Copyright Epic Games, Inc. All Rights Reserved.

#include "tdmtests/Defs.h"
#include "tdmtests/Helpers.h"

#include "tdm/TDM.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

// Tests that mat2euler is an exact inverse of euler2mat for every rotation sequence and every
// per-axis sign convention. The check is matrix round-trip recovery:
//     M = euler2mat(E, seq, signs);  E' = mat2euler(M, seq, signs);  require euler2mat(E') == M
// Euler angles are not unique (especially at gimbal lock), so E and E' may differ; the rotations
// they encode must not, which keeps the test independent of which branch the extractor picks.
// Each sequence is a separate specialization with its own normal and two gimbal-lock (+-90 middle
// axis) branches; all are exercised by the value grids below.

namespace tdmtests {

constexpr float tolerance = 1e-4f;

const std::array<tdm::rot_seq, 6> allSequences{tdm::rot_seq::xyz,
                                               tdm::rot_seq::xzy,
                                               tdm::rot_seq::yxz,
                                               tdm::rot_seq::yzx,
                                               tdm::rot_seq::zxy,
                                               tdm::rot_seq::zyx};

// All 8 combinations of per-axis rotation direction.
const std::array<tdm::rot_sign, 8> allSigns{
    tdm::rot_sign{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive},
    tdm::rot_sign{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::negative},
    tdm::rot_sign{tdm::rot_dir::positive, tdm::rot_dir::negative, tdm::rot_dir::positive},
    tdm::rot_sign{tdm::rot_dir::positive, tdm::rot_dir::negative, tdm::rot_dir::negative},
    tdm::rot_sign{tdm::rot_dir::negative, tdm::rot_dir::positive, tdm::rot_dir::positive},
    tdm::rot_sign{tdm::rot_dir::negative, tdm::rot_dir::positive, tdm::rot_dir::negative},
    tdm::rot_sign{tdm::rot_dir::negative, tdm::rot_dir::negative, tdm::rot_dir::positive},
    tdm::rot_sign{tdm::rot_dir::negative, tdm::rot_dir::negative, tdm::rot_dir::negative}};

const char* sequenceName(tdm::rot_seq s) {
    switch (s) {
    case tdm::rot_seq::xyz:
        return "xyz";
    case tdm::rot_seq::xzy:
        return "xzy";
    case tdm::rot_seq::yxz:
        return "yxz";
    case tdm::rot_seq::yzx:
        return "yzx";
    case tdm::rot_seq::zxy:
        return "zxy";
    case tdm::rot_seq::zyx:
        return "zyx";
    }
    return "?";
}

// The euler component index (0=x,1=y,2=z) of the middle (second-applied) rotation, which is
// the axis whose +-90 degree value induces gimbal lock for the given sequence.
tdm::dim_t middleAxisIndex(tdm::rot_seq s) {
    switch (s) {
    case tdm::rot_seq::xyz:
        return 1;  // Rx*Ry*Rz -> middle Ry -> y
    case tdm::rot_seq::xzy:
        return 2;  // Rx*Rz*Ry -> middle Rz -> z
    case tdm::rot_seq::yxz:
        return 0;  // Ry*Rx*Rz -> middle Rx -> x
    case tdm::rot_seq::yzx:
        return 2;  // Ry*Rz*Rx -> middle Rz -> z
    case tdm::rot_seq::zxy:
        return 0;  // Rz*Rx*Ry -> middle Rx -> x
    case tdm::rot_seq::zyx:
        return 1;  // Rz*Ry*Rx -> middle Ry -> y
    }
    return 1;
}

std::string signString(tdm::rot_sign s) {
    auto c = [](tdm::rot_dir d) { return d == tdm::rot_dir::negative ? '-' : '+'; };
    return std::string{c(s.x), c(s.y), c(s.z)};
}

float maxMatrixDiff(const tdm::mat3<float>& a, const tdm::mat3<float>& b) {
    float worst = 0.0f;
    for (tdm::dim_t r = 0; r < 3; ++r) {
        for (tdm::dim_t c = 0; c < 3; ++c) {
            worst = std::max(worst, std::abs(a(r, c) - b(r, c)));
        }
    }
    return worst;
}

// euler2mat -> mat2euler -> euler2mat, returning the matrix-recovery error.
float roundTripError(tdm::rot_seq seq, tdm::rot_sign signs, const tdm::frad3& euler) {
    const tdm::mat3<float> m = tdm::impl::euler2mat<float>(euler, seq, signs);
    const tdm::frad3 extracted = tdm::impl::mat2euler<float>(m, seq, signs);
    const tdm::mat3<float> reconstructed = tdm::impl::euler2mat<float>(extracted, seq, signs);
    return maxMatrixDiff(m, reconstructed);
}

}  // namespace tdmtests

// Full-cube recovery sweep: every sequence x every sign combination x every euler triple drawn
// from a value set that hits all categories independently on all three axes (gimbal lock can be
// triggered only by the middle axis, but folding it combines the outer angles, so every axis is
// swept over the full set). The value set, by category:
//   - zero:            0
//   - small / normal:  +-10, +-45, +-135
//   - fractional:      +-123.456 (so recovery never relies on lucky exact arithmetic)
//   - exact gimbal:    +-90, and +-270 which reaches the singularity from the far side
//   - half/full turn:  +-180, +-360
//   - beyond 180:      +-225, +-270, +-315 (multi-wrap inputs outside the canonical range)
// The near-gimbal (+-89.9 / +-90.1) and additional fractional categories are covered by the
// dedicated test below.
TEST(TestMatEulerConverter, MatrixRecoveryAllSequencesAllSigns) {
    using namespace tdmtests;
    const std::array<float, 21> angles{-360.0f,  -315.0f, -270.0f, -225.0f, -180.0f, -135.0f, -123.456f,
                                       -90.0f,   -45.0f,  -10.0f,  0.0f,    10.0f,   45.0f,   90.0f,
                                       123.456f, 135.0f,  180.0f,  225.0f,  270.0f,  315.0f,  360.0f};
    for (tdm::rot_seq seq : allSequences) {
        for (tdm::rot_sign signs : allSigns) {
            float worst = 0.0f;
            tdm::frad3 worstInput{};
            for (float ax : angles) {
                for (float ay : angles) {
                    for (float az : angles) {
                        const tdm::frad3 e{tdm::frad{tdm::fdeg{ax}}, tdm::frad{tdm::fdeg{ay}}, tdm::frad{tdm::fdeg{az}}};
                        const float err = roundTripError(seq, signs, e);
                        if (err > worst) {
                            worst = err;
                            worstInput = e;
                        }
                    }
                }
            }
            if (worst > tolerance) {
                std::printf("FAIL seq=%s signs=%s worstErr=%.6f at euler(deg)=(%.3f,%.3f,%.3f)\n",
                            sequenceName(seq),
                            signString(signs).c_str(),
                            worst,
                            tdm::fdeg{worstInput[0]}.value,
                            tdm::fdeg{worstInput[1]}.value,
                            tdm::fdeg{worstInput[2]}.value);
            }
            ASSERT_LE(worst, tolerance) << "sequence " << sequenceName(seq) << " signs " << signString(signs);
        }
    }
}

// Covers the two categories that the full-cube sweep above intentionally skips, for every
// sequence and sign combination:
//   - near-gimbal: middle axis at +-89.9 / +-90.1, which sit just off the singularity and take
//     the normal branch, verifying numerical stability right next to gimbal lock; plus the exact
//     +-90 singularity itself, which the full-cube sweep only reaches with round outer angles
//     while here it is folded against fractional outers; and
//   - fractional / non-round angles, so recovery never relies on lucky exact arithmetic
//     (e.g. sin(90 deg) being exactly representable).
// The middle axis (the one whose value triggers gimbal lock) is placed per sequence, and the
// outer angles use fractional values so the gimbal fold is exercised with non-round numbers.
TEST(TestMatEulerConverter, NearGimbalAndFractionalAnglesAllSequencesAllSigns) {
    using namespace tdmtests;
    const std::array<float, 6> midValues{-90.1f, -90.0f, -89.9f, 89.9f, 90.0f, 90.1f};
    const std::array<float, 4> fractionalOuter{-123.456f, -37.77f, 11.3f, 158.9f};
    for (tdm::rot_seq seq : allSequences) {
        const tdm::dim_t mid = middleAxisIndex(seq);
        for (tdm::rot_sign signs : allSigns) {
            for (float midValue : midValues) {
                for (float a : fractionalOuter) {
                    for (float b : fractionalOuter) {
                        float v[3];
                        tdm::dim_t oi = 0;
                        const float outers[2] = {a, b};
                        for (tdm::dim_t i = 0; i < 3; ++i) {
                            v[i] = (i == mid) ? midValue : outers[oi++];
                        }
                        const tdm::frad3 e{tdm::frad{tdm::fdeg{v[0]}}, tdm::frad{tdm::fdeg{v[1]}}, tdm::frad{tdm::fdeg{v[2]}}};
                        ASSERT_LE(roundTripError(seq, signs, e), tolerance)
                            << "sequence " << sequenceName(seq) << " signs " << signString(signs) << " mid " << midValue;
                    }
                }
            }
        }
    }
}
