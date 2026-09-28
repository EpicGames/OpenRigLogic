// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/rbf/cpu/AdditiveRBFSolver.h"
#include "riglogic/rbf/cpu/InterpolativeRBFSolver.h"
#include "riglogic/rbf/cpu/RBFSolver.h"
#include "riglogic/types/BoundedInputArchive.h"

#include <terse/archives/binary/OutputArchive.h>

#include <cmath>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4324)
#endif

namespace {

class RBFSolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        recipe.normalizeMethod = rl4::RBFNormalizeMethod::AlwaysNormalize;
        recipe.isAutomaticRadius = true;
        recipe.radius = 0.0f;
        recipe.twistAxis = dna::TwistAxis::X;
        recipe.weightThreshold = 0.001f;
    }

protected:
    pma::AlignedMemoryResource memRes;
    rl4::RBFSolverRecipe recipe;
};

}  // namespace

TEST_F(RBFSolverTest, InterpolativeGaussianSwingAngle0) {
    recipe.solverType = rl4::RBFSolverType::Interpolative;
    recipe.distanceMethod = rl4::RBFDistanceMethod::SwingAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Gaussian;
    std::size_t targetCount = 4;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.rawControlCount = 4u;
    pma::Vector<float> targetValues{0.0f,
                                    0.0f,
                                    0.0f,
                                    1.0f,
                                    -0.0936718f,
                                    -0.12003f,
                                    0.663135f,
                                    0.732851f,
                                    0.123443f,
                                    0.0891258f,
                                    -0.199695f,
                                    0.967957f,
                                    -0.12003f,
                                    0.0936719f,
                                    -0.732851f,
                                    0.663135f};
    recipe.targetValues = targetValues;
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    pma::Vector<float> input{-0.0936718f, -0.12003f, 0.663135f, 0.732851f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};

    solver->solve(input, buffer, result);
    pma::Vector<float> expected{0.0f, 1.0f, 0.0f, 0.0f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
    input = {0.0f, 0.0f, -0.47362f, 0.880729f};
    solver->solve(input, buffer, result);
    expected = {0.0657254f, 0.0f, 0.453696f, 0.480578f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
}

TEST_F(RBFSolverTest, InterpolativeGaussianSwingAngle1) {
    recipe.solverType = rl4::RBFSolverType::Interpolative;
    recipe.distanceMethod = rl4::RBFDistanceMethod::SwingAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Gaussian;
    std::size_t targetCount = 12u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.rawControlCount = 4u;
    pma::Vector<float> targetValues{
        0.000000000000000f,  0.000000000000000f,  0.000000000000000f,  1.000000000000000f,  -0.003081271657720f,
        -0.118239738047123f, -0.009329595603049f, 0.992936491966248f,  -0.008705757558346f, 0.009779179468751f,
        -0.141530960798264f, 0.989847242832184f,  0.026532903313637f,  -0.811531901359558f, -0.024197027087212f,
        0.583203732967377f,  -0.000952127971686f, 0.013058164156973f,  0.076173260807991f,  0.997008621692657f,
        -0.044993601739407f, -0.664866507053375f, 0.044108338654041f,  0.744300007820129f,  -0.005394733510911f,
        0.099454566836357f,  -0.012115634977818f, 0.994953811168671f,  0.009781738743186f,  0.008702844381332f,
        0.372627735137939f,  0.927888572216034f,  -0.009282855316997f, 0.312406390905380f,  -0.014897738583386f,
        0.949786365032196f,  -0.003883346682414f, -0.450696706771851f, -0.001544478582218f, 0.892667353153229f,
        -0.005706345662475f, -0.011783968657255f, -0.714682221412659f, 0.699326753616333f,  0.000949318520725f,
        -0.013058470562100f, -0.825225293636322f, 0.564651966094971f};

    recipe.targetValues = targetValues;
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    pma::Vector<float> input{-0.005706345662475f, -0.011783968657255f, -0.714682221412659f, 0.699326753616333f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};

    solver->solve(input, buffer, result);
    pma::Vector<float> expected{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);

    input = {0.620591878890991f, 0.382426619529724f, -0.683809995651245f, 0.031930625438690f};
    solver->solve(input, buffer, result);
    expected = {0.0f, 0.0f, 0.0f, 0.212102f, 0.0116427f, 0.0f, 0.0f, 0.279036f, 0.473921f, 0.0f, 0.0033996f, 0.0198993f};

    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
}

TEST_F(RBFSolverTest, AdditiveGaussianSwingAngle0) {
    recipe.solverType = rl4::RBFSolverType::Additive;
    recipe.distanceMethod = rl4::RBFDistanceMethod::SwingAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Gaussian;
    std::size_t targetCount = 4;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.rawControlCount = 4u;
    pma::Vector<float> targetValues{0.0f,
                                    0.0f,
                                    0.0f,
                                    1.0f,
                                    -0.0936718f,
                                    -0.12003f,
                                    0.663135f,
                                    0.732851f,
                                    0.123443f,
                                    0.0891258f,
                                    -0.199695f,
                                    0.967957f,
                                    -0.12003f,
                                    0.0936719f,
                                    -0.732851f,
                                    0.663135f};
    recipe.targetValues = targetValues;
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    pma::Vector<float> input{-0.0936718f, -0.12003f, 0.663135f, 0.732851f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};

    solver->solve(input, buffer, result);
    pma::Vector<float> expected{0.242209f, 0.41645f, 0.20938f, 0.131961f};

    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
}

// TwistAngle must match UE's RBFDistanceMetric::TwistAngle: |twistA - twistB| on unwound angles, range [0, 2pi].
// The pre-fix implementation took the arc length between twist quaternions, which wraps at pi (a 240-degree
// separation read as 120), so these pin the unwrapped metric. Radius is in radians (2pi == 360 degrees).

TEST_F(RBFSolverTest, AdditiveLinearTwistAngleDoesNotWrapAtPi) {
    recipe.solverType = rl4::RBFSolverType::Additive;
    recipe.distanceMethod = rl4::RBFDistanceMethod::TwistAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Linear;
    recipe.normalizeMethod = rl4::RBFNormalizeMethod::OnlyNormalizeAboveOne;
    recipe.twistAxis = dna::TwistAxis::Z;
    recipe.isAutomaticRadius = false;
    recipe.radius = 2.0f * 3.14159265f;
    std::size_t targetCount = 1u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.rawControlCount = 4u;
    // Target: -120 degrees about Z
    pma::Vector<float> targetValues{0.0f, 0.0f, -0.8660254f, 0.5f};
    recipe.targetValues = targetValues;
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    // Input: +120 degrees about Z -> twist distance 240 degrees, linear weight 1 - 240 / 360
    pma::Vector<float> input{0.0f, 0.0f, 0.8660254f, 0.5f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};

    solver->solve(input, buffer, result);
    pma::Vector<float> expected{0.3333333f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
}

TEST_F(RBFSolverTest, AdditiveLinearTwistAngleUnwindsNegatedQuaternionAndIgnoresSwing) {
    recipe.solverType = rl4::RBFSolverType::Additive;
    recipe.distanceMethod = rl4::RBFDistanceMethod::TwistAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Linear;
    recipe.normalizeMethod = rl4::RBFNormalizeMethod::OnlyNormalizeAboveOne;
    recipe.twistAxis = dna::TwistAxis::Z;
    recipe.isAutomaticRadius = false;
    recipe.radius = 2.0f * 3.14159265f;
    std::size_t targetCount = 1u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.rawControlCount = 4u;
    // Target: +90 degrees about Z
    pma::Vector<float> targetValues{0.0f, 0.0f, 0.7071068f, 0.7071068f};
    recipe.targetValues = targetValues;
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    // Input: -(swingX(60) * twistZ(90)); the negated representation yields 2 * atan2 == -270 degrees, which must
    // unwind to +90 (distance 0), and the swing component must not contribute.
    pma::Vector<float> input{-0.3535534f, 0.3535534f, -0.6123724f, -0.6123724f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};

    solver->solve(input, buffer, result);
    pma::Vector<float> expected{1.0f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
}

TEST_F(RBFSolverTest, InterpolativeLinearTwistAngleHalfJointSetup) {
    // The MetaHuman body finger "half" solvers: targets at 0 / +120 / -120 / -20 degrees about Z, Linear, radius 360.
    // With the unwrapped metric the net is a 1D piecewise-linear interpolation, so an input between two neighboring
    // targets weights exactly those two. The wrapped metric instead activated the -20 target for a +40 input.
    recipe.solverType = rl4::RBFSolverType::Interpolative;
    recipe.distanceMethod = rl4::RBFDistanceMethod::TwistAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Linear;
    recipe.normalizeMethod = rl4::RBFNormalizeMethod::AlwaysNormalize;
    recipe.twistAxis = dna::TwistAxis::Z;
    recipe.isAutomaticRadius = false;
    recipe.radius = 2.0f * 3.14159265f;
    std::size_t targetCount = 4u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.rawControlCount = 4u;
    pma::Vector<float> targetValues{0.0f,
                                    0.0f,
                                    0.0f,
                                    1.0f,  // default
                                    0.0f,
                                    0.0f,
                                    0.8660254f,
                                    0.5f,  // curl +120
                                    0.0f,
                                    0.0f,
                                    -0.8660254f,
                                    0.5f,  // push -120
                                    0.0f,
                                    0.0f,
                                    -0.1736482f,
                                    0.9848078f};  // caps -20
    recipe.targetValues = targetValues;
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};

    // +90 degrees: three quarters of the way from default to curl
    pma::Vector<float> input{0.0f, 0.0f, 0.7071068f, 0.7071068f};
    solver->solve(input, buffer, result);
    pma::Vector<float> expected{0.25f, 0.75f, 0.0f, 0.0f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.001f);

    // +40 degrees: one third of the way from default to curl
    input = {0.0f, 0.0f, 0.3420201f, 0.9396926f};
    solver->solve(input, buffer, result);
    expected = {0.6666667f, 0.3333333f, 0.0f, 0.0f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.001f);

    // -90 degrees: between caps and push
    input = {0.0f, 0.0f, -0.7071068f, 0.7071068f};
    solver->solve(input, buffer, result);
    expected = {0.0f, 0.0f, 0.7f, 0.3f};
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.001f);
}

namespace {

// The finger half-joint setup: targets at 0 / +120 / -120 / -20 degrees about Z, Linear, radius 360, Interpolative.
void setHalfJointRecipe(rl4::RBFSolverRecipe& recipe, rl4::RBFDistanceMethod distanceMethod) {
    recipe.solverType = rl4::RBFSolverType::Interpolative;
    recipe.distanceMethod = distanceMethod;
    recipe.weightFunction = rl4::RBFFunctionType::Linear;
    recipe.normalizeMethod = rl4::RBFNormalizeMethod::AlwaysNormalize;
    recipe.twistAxis = dna::TwistAxis::Z;
    recipe.isAutomaticRadius = false;
    recipe.radius = 2.0f * 3.14159265f;
    recipe.rawControlCount = 4u;
}

const float halfJointTargets[] =
    {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.8660254f, 0.5f, 0.0f, 0.0f, -0.8660254f, 0.5f, 0.0f, 0.0f, -0.1736482f, 0.9848078f};

}  // namespace

TEST_F(RBFSolverTest, InterpolativeTwistAngleRestoreMatchesFreshBuild) {
    setHalfJointRecipe(recipe, rl4::RBFDistanceMethod::TwistAngle);
    const std::size_t targetCount = 4u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.targetValues = rl4::ConstArrayView<float>{halfJointTargets, sizeof(halfJointTargets) / sizeof(float)};
    auto solver = rl4::RBFSolver::create(recipe, &memRes);

    auto stream = rl4::makeScoped<rl4::MemoryStream>(&memRes);
    terse::BinaryOutputArchive<rl4::BoundedIOStream> output{stream.get()};
    solver->save(output);
    stream->seek(0ul);
    rl4::BoundedInputArchive input{stream.get()};
    rl4::InterpolativeRBFSolver restored{&memRes};
    restored.load(input);
    ASSERT_TRUE(input.isOk());

    pma::Vector<float> query{0.0f, 0.0f, 0.7071068f, 0.7071068f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> expected{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};
    solver->solve(query, buffer, expected);
    query = {0.0f, 0.0f, 0.7071068f, 0.7071068f};
    restored.solve(query, buffer, result);
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
    EXPECT_NEAR(result[1], 0.75f, 0.001f);
}

TEST_F(RBFSolverTest, InterpolativeRestoreRejectsMismatchedCoefficientMatrix) {
    // Quaternion: no input conversion, so the raw targets serialized below are exactly what save() writes, and the
    // pure-Z targets stay distinct (SwingAngle about Z would collapse them all onto the identity swing).
    setHalfJointRecipe(recipe, rl4::RBFDistanceMethod::Quaternion);
    const std::size_t targetCount = 4u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.targetValues = rl4::ConstArrayView<float>{halfJointTargets, sizeof(halfJointTargets) / sizeof(float)};
    auto solver = rl4::RBFSolver::create(recipe, &memRes);
    const auto* interpolative = static_cast<const rl4::InterpolativeRBFSolver*>(solver.get());

    // Same state as save() - Quaternion solvers carry an empty distance-row block - with the first coefficientRows rows
    // of the matrix. The complete matrix must load, so a rejection of the truncated one cannot be a misaligned stream.
    auto loadWithCoefficientRows = [&](std::size_t coefficientRows) {
        rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
        targets.resize(targetCount);
        for (std::size_t ti = 0u; ti < targetCount; ++ti) {
            targets[ti].assign(halfJointTargets + ti * 4u, halfJointTargets + ti * 4u + 4u);
        }
        rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
        rl4::Vector<float> targetScale{scaleFactors.begin(), scaleFactors.end(), &memRes};
        float radius = recipe.radius;
        float weightThreshold = recipe.weightThreshold;
        auto distanceMethod = recipe.distanceMethod;
        auto weightFunction = recipe.weightFunction;
        auto normalizeMethod = recipe.normalizeMethod;
        auto twistAxis = recipe.twistAxis;
        rl4::Matrix<float> coefficients{&memRes};
        coefficients.resize(coefficientRows);
        for (std::size_t i = 0u; i < coefficientRows; ++i) {
            coefficients[i].assign(interpolative->getCoefficients()[i].begin(), interpolative->getCoefficients()[i].end());
        }

        auto stream = rl4::makeScoped<rl4::MemoryStream>(&memRes);
        terse::BinaryOutputArchive<rl4::BoundedIOStream> output{stream.get()};
        output(targets);
        output(distanceTargets);
        output(targetScale);
        output(radius);
        output(weightThreshold);
        output(distanceMethod);
        output(weightFunction);
        output(normalizeMethod);
        output(twistAxis);
        output(coefficients);
        stream->seek(0ul);

        rl4::BoundedInputArchive input{stream.get()};
        rl4::InterpolativeRBFSolver restored{&memRes};
        restored.load(input);
        return input.isOk();
    };
    ASSERT_TRUE(loadWithCoefficientRows(targetCount));
    EXPECT_FALSE(loadWithCoefficientRows(targetCount - 1u));
}

namespace {

// Serializes a solver state of the given type and distance method with the given target and distance rows (plus a
// well-formed targetCount x targetCount coefficient matrix for Interpolative), then loads it into that solver type.
// Returns the archive's isOk() after load.
bool loadSnapshot(pma::AlignedMemoryResource& memRes,
                  rl4::RBFSolverType solverType,
                  rl4::RBFDistanceMethod distanceMethod,
                  const rl4::Vector<rl4::AlignedVector<float>>& targets,
                  const rl4::Vector<rl4::AlignedVector<float>>& distanceTargets) {
    const std::size_t targetCount = targets.size();
    rl4::Vector<rl4::AlignedVector<float>> storedTargets{targets.begin(), targets.end(), &memRes};
    rl4::Vector<rl4::AlignedVector<float>> storedDistanceTargets{distanceTargets.begin(), distanceTargets.end(), &memRes};
    rl4::Vector<float> targetScale{targetCount, 1.0f, &memRes};
    float radius = 2.0f * 3.14159265f;
    float weightThreshold = 0.001f;
    auto weightFunction = rl4::RBFFunctionType::Linear;
    auto normalizeMethod = rl4::RBFNormalizeMethod::AlwaysNormalize;
    auto twistAxis = dna::TwistAxis::Z;
    rl4::Matrix<float> coefficients{&memRes};
    coefficients.resize(targetCount);
    for (auto& row : coefficients) {
        row.assign(targetCount, 0.0f);
    }
    auto stream = rl4::makeScoped<rl4::MemoryStream>(&memRes);
    terse::BinaryOutputArchive<rl4::BoundedIOStream> output{stream.get()};
    output(storedTargets);
    output(storedDistanceTargets);
    output(targetScale);
    output(radius);
    output(weightThreshold);
    output(distanceMethod);
    output(weightFunction);
    output(normalizeMethod);
    output(twistAxis);
    if (solverType == rl4::RBFSolverType::Interpolative) {
        output(coefficients);
    }
    stream->seek(0ul);
    rl4::BoundedInputArchive input{stream.get()};
    if (solverType == rl4::RBFSolverType::Interpolative) {
        rl4::InterpolativeRBFSolver restored{&memRes};
        restored.load(input);
    } else {
        rl4::AdditiveRBFSolver restored{&memRes};
        restored.load(input);
    }
    return input.isOk();
}

bool loadTwistAngleSnapshot(pma::AlignedMemoryResource& memRes,
                            const rl4::Vector<rl4::AlignedVector<float>>& targets,
                            const rl4::Vector<rl4::AlignedVector<float>>& distanceTargets) {
    return loadSnapshot(memRes, rl4::RBFSolverType::Interpolative, rl4::RBFDistanceMethod::TwistAngle, targets, distanceTargets);
}

}  // namespace

// The quaternion-family distance functors size their reads of every row by the width of the row they are handed, and
// RBFBehaviorValidator only checks target widths after load, so ragged target rows are an out-of-bounds read at solve
// unless rejected up front: the long-first-row case a heap-buffer-overflow, the short-row case a silent read of padding.
// The distance rows are well-formed here (count and width match the first target row), so only the target guard rejects.

TEST_F(RBFSolverTest, InterpolativeTwistAngleRestoreRejectsLongerTargetRow) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(3u);
    targets[0].assign(64u, 0.5f);
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    targets[2].assign({0.0f, 0.0f, -0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(3u);
    for (auto& row : distanceTargets) {
        row.assign(64u, 0.0f);
    }
    EXPECT_FALSE(loadTwistAngleSnapshot(memRes, targets, distanceTargets));
}

TEST_F(RBFSolverTest, InterpolativeTwistAngleRestoreRejectsShorterTargetRow) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(4u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    targets[2].assign({0.0f, 0.0f});
    targets[3].assign({0.0f, 0.0f, -0.1736482f, 0.9848078f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(4u);
    for (auto& row : distanceTargets) {
        row.assign(4u, 0.0f);
    }
    EXPECT_FALSE(loadTwistAngleSnapshot(memRes, targets, distanceTargets));
}

// solve() sizes its weight buffers from targets and reads the restored distance rows by input width, so distance rows
// that disagree with targets in count or width must be rejected; matching rows are the well-formed case and must load.

TEST_F(RBFSolverTest, InterpolativeTwistAngleRestoreAcceptsMatchingDistanceRows) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(2u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign({2.0943951f, 0.0f, 0.0f, 0.0f});
    EXPECT_TRUE(loadTwistAngleSnapshot(memRes, targets, distanceTargets));
}

TEST_F(RBFSolverTest, InterpolativeTwistAngleRestoreRejectsDistanceRowCountMismatch) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(3u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    targets[2].assign({0.0f, 0.0f, -0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign({2.0943951f, 0.0f, 0.0f, 0.0f});
    EXPECT_FALSE(loadTwistAngleSnapshot(memRes, targets, distanceTargets));
}

TEST_F(RBFSolverTest, InterpolativeTwistAngleRestoreRejectsDistanceRowWidthMismatch) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(2u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign(8u, 2.0943951f);
    EXPECT_FALSE(loadTwistAngleSnapshot(memRes, targets, distanceTargets));
}

// The guard lives in RBFSolver::load, so the Additive restore path rejects and accepts the same rows.

TEST_F(RBFSolverTest, AdditiveTwistAngleRestoreAcceptsMatchingDistanceRows) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(2u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign({2.0943951f, 0.0f, 0.0f, 0.0f});
    EXPECT_TRUE(loadSnapshot(memRes, rl4::RBFSolverType::Additive, rl4::RBFDistanceMethod::TwistAngle, targets, distanceTargets));
}

TEST_F(RBFSolverTest, AdditiveTwistAngleRestoreRejectsDistanceRowCountMismatch) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(3u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    targets[2].assign({0.0f, 0.0f, -0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign({2.0943951f, 0.0f, 0.0f, 0.0f});
    EXPECT_FALSE(
        loadSnapshot(memRes, rl4::RBFSolverType::Additive, rl4::RBFDistanceMethod::TwistAngle, targets, distanceTargets));
}

TEST_F(RBFSolverTest, AdditiveTwistAngleRestoreRejectsDistanceRowWidthMismatch) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(2u);
    targets[0].assign({0.0f, 0.0f, 0.0f, 1.0f});
    targets[1].assign({0.0f, 0.0f, 0.8660254f, 0.5f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign(8u, 2.0943951f);
    EXPECT_FALSE(
        loadSnapshot(memRes, rl4::RBFSolverType::Additive, rl4::RBFDistanceMethod::TwistAngle, targets, distanceTargets));
}

// buildDistanceTargets leaves the distance rows empty for every metric but TwistAngle, so a non-empty block in such a
// snapshot is not one save() writes; an empty block is the well-formed case.

TEST_F(RBFSolverTest, InterpolativeRestoreRejectsDistanceRowsForNonTwistAngleMethod) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(2u);
    targets[0].assign({0.0f, 0.0f, 0.0f, -1.0f});
    targets[1].assign({0.0f, 0.2588190f, 0.0f, -0.9659258f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    distanceTargets.resize(2u);
    distanceTargets[0].assign({0.0f, 0.0f, 0.0f, 0.0f});
    distanceTargets[1].assign({0.0f, 0.0f, 0.0f, 0.0f});
    EXPECT_FALSE(
        loadSnapshot(memRes, rl4::RBFSolverType::Interpolative, rl4::RBFDistanceMethod::SwingAngle, targets, distanceTargets));
}

TEST_F(RBFSolverTest, InterpolativeRestoreAcceptsEmptyDistanceRowsForNonTwistAngleMethod) {
    rl4::Vector<rl4::AlignedVector<float>> targets{&memRes};
    targets.resize(2u);
    targets[0].assign({0.0f, 0.0f, 0.0f, -1.0f});
    targets[1].assign({0.0f, 0.2588190f, 0.0f, -0.9659258f});
    rl4::Vector<rl4::AlignedVector<float>> distanceTargets{&memRes};
    EXPECT_TRUE(
        loadSnapshot(memRes, rl4::RBFSolverType::Interpolative, rl4::RBFDistanceMethod::SwingAngle, targets, distanceTargets));
}

// getTargets() returns the stored twist quaternions, not the angle form: the angle rows are serialized alongside them as
// distanceTargets and are consumed only by solve().
TEST_F(RBFSolverTest, TwistAngleStoredTargetsStayTwistQuaternions) {
    setHalfJointRecipe(recipe, rl4::RBFDistanceMethod::TwistAngle);
    const std::size_t targetCount = 4u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.targetValues = rl4::ConstArrayView<float>{halfJointTargets, sizeof(halfJointTargets) / sizeof(float)};
    auto solver = rl4::RBFSolver::create(recipe, &memRes);

    const auto stored = solver->getTargets();
    ASSERT_EQ(stored.size(), targetCount);
    // curl +120 about Z: (0, 0, sin 60, cos 60), not the angle form (2.0944, 0, 0, 0)
    ASSERT_EQ(stored[1].size(), 4u);
    EXPECT_FLOAT_EQ(stored[1][0], 0.0f);
    EXPECT_FLOAT_EQ(stored[1][1], 0.0f);
    EXPECT_NEAR(stored[1][2], 0.8660254f, 0.000001f);
    EXPECT_NEAR(stored[1][3], 0.5f, 0.000001f);
}

// Angle form is axis- and sign-agnostic: a twist about any axis, in either quaternion sign, against a +90 degree
// target gives the Linear weight 1 - |90 - theta| / 360 with the unwound (not shortest-arc) difference.
TEST_F(RBFSolverTest, AdditiveLinearTwistAngleMatchesUnwoundDifferenceOnEveryAxis) {
    recipe.solverType = rl4::RBFSolverType::Additive;
    recipe.distanceMethod = rl4::RBFDistanceMethod::TwistAngle;
    recipe.weightFunction = rl4::RBFFunctionType::Linear;
    recipe.normalizeMethod = rl4::RBFNormalizeMethod::OnlyNormalizeAboveOne;
    recipe.isAutomaticRadius = false;
    recipe.radius = 2.0f * 3.14159265f;
    recipe.rawControlCount = 4u;
    const std::size_t targetCount = 1u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;

    const float degToRad = 3.14159265f / 180.0f;
    const dna::TwistAxis axes[] = {dna::TwistAxis::X, dna::TwistAxis::Y, dna::TwistAxis::Z};
    const float inputDegrees[] = {-170.0f, -120.0f, -45.0f, 0.0f, 30.0f, 90.0f, 150.0f, 179.0f};
    const float signs[] = {1.0f, -1.0f};
    for (auto axis : axes) {
        recipe.twistAxis = axis;
        const auto slot = static_cast<std::size_t>(axis);
        pma::Vector<float> targetValues{0.0f, 0.0f, 0.0f, 0.0f};
        targetValues[slot] = std::sin(45.0f * degToRad);
        targetValues[3] = std::cos(45.0f * degToRad);
        recipe.targetValues = targetValues;
        auto solver = rl4::RBFSolver::create(recipe, &memRes);

        for (float degrees : inputDegrees) {
            for (float sign : signs) {
                pma::Vector<float> input{0.0f, 0.0f, 0.0f, 0.0f};
                input[slot] = sign * std::sin(0.5f * degrees * degToRad);
                input[3] = sign * std::cos(0.5f * degrees * degToRad);
                pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
                pma::Vector<float> result{targetCount, 0.0f, &memRes};
                solver->solve(input, buffer, result);
                const float expected = 1.0f - std::abs(90.0f - degrees) / 360.0f;
                EXPECT_NEAR(result[0], expected, 0.0005f) << "axis " << slot << " degrees " << degrees << " sign " << sign;
            }
        }
    }
}

TEST_F(RBFSolverTest, AdditiveTwistAngleRestoreMatchesFreshBuild) {
    setHalfJointRecipe(recipe, rl4::RBFDistanceMethod::TwistAngle);
    recipe.solverType = rl4::RBFSolverType::Additive;
    const std::size_t targetCount = 4u;
    pma::Vector<float> scaleFactors{targetCount, 1.0f, &memRes};
    recipe.targetScales = scaleFactors;
    recipe.targetValues = rl4::ConstArrayView<float>{halfJointTargets, sizeof(halfJointTargets) / sizeof(float)};
    auto solver = rl4::RBFSolver::create(recipe, &memRes);

    auto stream = rl4::makeScoped<rl4::MemoryStream>(&memRes);
    terse::BinaryOutputArchive<rl4::BoundedIOStream> output{stream.get()};
    solver->save(output);
    stream->seek(0ul);
    rl4::BoundedInputArchive input{stream.get()};
    rl4::AdditiveRBFSolver restored{&memRes};
    restored.load(input);
    ASSERT_TRUE(input.isOk());

    pma::Vector<float> query{0.0f, 0.0f, 0.7071068f, 0.7071068f};
    pma::Vector<float> buffer{targetCount, 0.0f, &memRes};
    pma::Vector<float> expected{targetCount, 0.0f, &memRes};
    pma::Vector<float> result{targetCount, 0.0f, &memRes};
    solver->solve(query, buffer, expected);
    query = {0.0f, 0.0f, 0.7071068f, 0.7071068f};
    restored.solve(query, buffer, result);
    EXPECT_ELEMENTS_NEAR(result, expected, targetCount, 0.0001f);
    EXPECT_GT(result[1], result[0]);
}
