// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/joints/bpcm/BPCMFixturesBlock4.h"

#include "riglogic/RigLogic.h"
#include "riglogic/TypeDefs.h"
#include "riglogic/controls/ControlsFactory.h"
#include "riglogic/joints/JointsFactory.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <tdm/Quat.h>

#include <algorithm>
#include <vector>

TEST(ScalarJointsFactoryTest, NeutralJointsAreCopied) {
    pma::AlignedMemoryResource memRes;
    block4::CanonicalReader reader;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::Scalar;
    auto meta = rl4::RigMetadata::create(config, &reader, &memRes);
    auto controls = rl4::ControlsFactory::create(config, meta.get(), &reader, &memRes);
    auto joints = rl4::JointsFactory::create(config, meta.get(), &reader, controls.get(), &memRes);
    const float expected[] = {0.0f,  1.0f, 2.0f, 3.0f, 4.0f,  5.0f,  1.0f,  1.0f,  1.0f,  6.0f,  7.0f, 8.0f, 9.0f, 10.0f,
                              11.0f, 1.0f, 1.0f, 1.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 1.0f, 1.0f, 1.0f};
    ASSERT_ELEMENTS_EQ(joints->getNeutralValues(), expected, 27ul);
}

namespace {

// Each reader perturbs one DNA field of the canonical block4 fixture so a downstream consumer would remap or narrow it
// out of the joint domain; the factory must reject the rig before any builder retains the data.

// A joint group with more input columns than the filter's uint16 column count can carry.
class WideColumnGroupReader : public block4::CanonicalReader {
public:
    WideColumnGroupReader() :
        columns(65536u, static_cast<std::uint16_t>(0u)) {
    }

    ~WideColumnGroupReader();

    rl4::ConstArrayView<std::uint16_t> getJointGroupInputIndices(std::uint16_t jointGroupIndex) const override {
        if (jointGroupIndex == 0u) {
            return {columns.data(), columns.size()};
        }
        return CanonicalReader::getJointGroupInputIndices(jointGroupIndex);
    }

private:
    std::vector<std::uint16_t> columns;
};

WideColumnGroupReader::~WideColumnGroupReader() = default;

// Group 0's first output attribute index replaced with an attribute of a joint the DNA does not declare.
class OutOfDomainOutputIndexReader : public block4::CanonicalReader {
public:
    explicit OutOfDomainOutputIndexReader(std::uint16_t outputIndex) {
        const auto canonical = CanonicalReader::getJointGroupOutputIndices(0u);
        outputs.assign(canonical.begin(), canonical.end());
        outputs[0] = outputIndex;
    }

    ~OutOfDomainOutputIndexReader();

    rl4::ConstArrayView<std::uint16_t> getJointGroupOutputIndices(std::uint16_t jointGroupIndex) const override {
        if (jointGroupIndex == 0u) {
            return {outputs.data(), outputs.size()};
        }
        return CanonicalReader::getJointGroupOutputIndices(jointGroupIndex);
    }

private:
    std::vector<std::uint16_t> outputs;
};

OutOfDomainOutputIndexReader::~OutOfDomainOutputIndexReader() = default;

// Joint group 2's rows replaced so joint 3's rows straddle a joint 4 row: [J3.tx, J4.tx, J3.ty]. Interleaved joint
// rows are supported input (the canonical fixture's groups 11 and 12 interleave too); the builders must not depend
// on one run per joint.
class InterleavedJointRowsReader : public block4::CanonicalReader {
public:
    explicit InterleavedJointRowsReader(bool interleaved) :
        outputs(interleaved ? std::vector<std::uint16_t>{27u, 36u, 28u} : std::vector<std::uint16_t>{27u, 28u, 36u}) {
    }

    ~InterleavedJointRowsReader();

    rl4::ConstArrayView<std::uint16_t> getJointGroupOutputIndices(std::uint16_t jointGroupIndex) const override {
        if (jointGroupIndex == 2u) {
            return {outputs.data(), outputs.size()};
        }
        return CanonicalReader::getJointGroupOutputIndices(jointGroupIndex);
    }

private:
    std::vector<std::uint16_t> outputs;
};

InterleavedJointRowsReader::~InterleavedJointRowsReader() = default;

// One twist setup driving a single output joint.
class TwistOutputJointReader : public block4::CanonicalReader {
public:
    explicit TwistOutputJointReader(std::uint16_t jointIndex) :
        outputJoint{jointIndex} {
    }

    ~TwistOutputJointReader();

    std::uint16_t getTwistCount() const override {
        return 1u;
    }

    rl4::ConstArrayView<std::uint16_t> getTwistInputControlIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t quad[] = {0u, 1u, 2u, 3u};
        return {quad, 4ul};
    }

    rl4::ConstArrayView<std::uint16_t> getTwistOutputJointIndices(std::uint16_t /*unused*/) const override {
        return {&outputJoint, 1ul};
    }

    rl4::ConstArrayView<float> getTwistBlendWeights(std::uint16_t /*unused*/) const override {
        static const float weights[] = {1.0f};
        return {weights, 1ul};
    }

private:
    std::uint16_t outputJoint;
};

TwistOutputJointReader::~TwistOutputJointReader() = default;

// The per-LOD joint list names a joint the DNA does not declare.
class LODJointIndexReader : public block4::CanonicalReader {
public:
    explicit LODJointIndexReader(std::uint16_t jointIndex) :
        joint{jointIndex} {
    }

    ~LODJointIndexReader();

    rl4::ConstArrayView<std::uint16_t> getJointIndicesForLOD(std::uint16_t /*unused*/) const override {
        return {&joint, 1ul};
    }

private:
    std::uint16_t joint;
};

LODJointIndexReader::~LODJointIndexReader() = default;

// One Euler joint whose two rotation rows sit in one group with INCREASING per-LOD row counts {1, 2}. In the
// quaternion configuration the rotation-row array is sized from LOD 0, so LOD 1's boundary must not index past it.
class IncreasingRotationLODsReader : public dna::FakeReader {
public:
    ~IncreasingRotationLODsReader();

    std::uint16_t getLODCount() const override {
        return 2u;
    }

    std::uint16_t getRawControlCount() const override {
        return 2u;
    }

    std::uint16_t getJointCount() const override {
        return 2u;
    }

    std::uint16_t getJointRowCount() const override {
        return 2u;
    }

    std::uint16_t getJointColumnCount() const override {
        return 1u;
    }

    std::uint16_t getJointGroupCount() const override {
        return 1u;
    }

    rl4::ConstArrayView<std::uint16_t> getJointGroupLODs(std::uint16_t /*unused*/) const override {
        static const std::uint16_t lods[] = {1u, 2u};
        return {lods, 2ul};
    }

    rl4::ConstArrayView<std::uint16_t> getJointGroupInputIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t inputs[] = {0u};
        return {inputs, 1ul};
    }

    rl4::ConstArrayView<std::uint16_t> getJointGroupOutputIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t outputs[] = {3u, 4u};  // rx, ry of joint 0
        return {outputs, 2ul};
    }

    rl4::ConstArrayView<float> getJointGroupValues(std::uint16_t /*unused*/) const override {
        static const float values[] = {0.5f, 0.25f};
        return {values, 2ul};
    }
};

IncreasingRotationLODsReader::~IncreasingRotationLODsReader() = default;

// Two Euler joints in one group, joint 0's rotation rows straddling joint 1's: [J0.rx, J1.rx, J0.ry] = {90, 90, 30}
// degrees, one control. LOD 1 keeps the first two rows, so it drives BOTH joints' rotations and must convert both.
class StraddlingRotationRowsReader : public dna::FakeReader {
public:
    ~StraddlingRotationRowsReader();

    // FakeReader zero-initializes the rotation sign to an invalid direction, which the quaternion adapter multiplies
    // into every Euler angle; a real rig always carries valid signs.
    dna::RotationSign getRotationSign() const override {
        return {tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
    }

    std::uint16_t getLODCount() const override {
        return 2u;
    }

    std::uint16_t getRawControlCount() const override {
        return 1u;
    }

    std::uint16_t getJointCount() const override {
        return 2u;
    }

    std::uint16_t getJointRowCount() const override {
        return 3u;
    }

    std::uint16_t getJointColumnCount() const override {
        return 1u;
    }

    std::uint16_t getJointGroupCount() const override {
        return 1u;
    }

    rl4::ConstArrayView<std::uint16_t> getJointGroupLODs(std::uint16_t /*unused*/) const override {
        static const std::uint16_t lods[] = {3u, 2u};
        return {lods, 2ul};
    }

    rl4::ConstArrayView<std::uint16_t> getJointGroupInputIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t inputs[] = {0u};
        return {inputs, 1ul};
    }

    rl4::ConstArrayView<std::uint16_t> getJointGroupOutputIndices(std::uint16_t /*unused*/) const override {
        static const std::uint16_t outputs[] = {3u, 12u, 4u};  // J0.rx, J1.rx, J0.ry
        return {outputs, 3ul};
    }

    rl4::ConstArrayView<float> getJointGroupValues(std::uint16_t /*unused*/) const override {
        static const float values[] = {90.0f, 90.0f, 30.0f};  // all non-zero so no row is pruned away
        return {values, 3ul};
    }
};

StraddlingRotationRowsReader::~StraddlingRotationRowsReader() = default;

// Joints keeps a raw pointer to the metadata it was built from, so the helper hands back the owners together with it.
struct BuiltJoints {
    rl4::RigMetadata::Pointer meta;
    rl4::Controls::Pointer controls;
    rl4::Joints::Pointer joints;
};

BuiltJoints createJoints(const dna::Reader& reader, rl4::RotationType rotationType, pma::AlignedMemoryResource& memRes) {
    BuiltJoints built;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::Scalar;
    config.rotationType = rotationType;
    built.meta = rl4::RigMetadata::create(config, &reader, &memRes);
    if (!built.meta) {
        return built;
    }
    built.controls = rl4::ControlsFactory::create(config, built.meta.get(), &reader, &memRes);
    built.joints = rl4::JointsFactory::create(config, built.meta.get(), &reader, built.controls.get(), &memRes);
    return built;
}

}  // namespace

TEST(ScalarJointsFactoryTest, RejectsJointGroupWithColumnCountBeyondUint16) {
    pma::AlignedMemoryResource memRes;
    WideColumnGroupReader reader;
    ASSERT_EQ(createJoints(reader, rl4::RotationType::EulerAngles, memRes).joints, nullptr);
}

TEST(ScalarJointsFactoryTest, RejectsJointGroupOutputIndexBeyondJointDomain) {
    pma::AlignedMemoryResource memRes;
    // tx of joint 24 in a 24-joint rig: one past the domain.
    OutOfDomainOutputIndexReader firstBeyond{static_cast<std::uint16_t>(24u * 9u)};
    ASSERT_EQ(createJoints(firstBeyond, rl4::RotationType::EulerAngles, memRes).joints, nullptr);
    ASSERT_EQ(createJoints(firstBeyond, rl4::RotationType::Quaternions, memRes).joints, nullptr);
    // tx of joint 6554: a valid uint16 DNA index whose 10-attribute remap (65540) wraps to 4 - inside the 240-float
    // quaternion output buffer, so only the pre-remap domain check can catch it.
    OutOfDomainOutputIndexReader wrapping{static_cast<std::uint16_t>(6554u * 9u)};
    ASSERT_EQ(createJoints(wrapping, rl4::RotationType::Quaternions, memRes).joints, nullptr);
}

TEST(ScalarJointsFactoryTest, RejectsTwistOutputJointBeyondJointCount) {
    pma::AlignedMemoryResource memRes;
    TwistOutputJointReader lastJoint{23u};
    ASSERT_NE(createJoints(lastJoint, rl4::RotationType::EulerAngles, memRes).joints, nullptr);
    ASSERT_NE(createJoints(lastJoint, rl4::RotationType::Quaternions, memRes).joints, nullptr);
    TwistOutputJointReader beyond{24u};
    ASSERT_EQ(createJoints(beyond, rl4::RotationType::EulerAngles, memRes).joints, nullptr);
    ASSERT_EQ(createJoints(beyond, rl4::RotationType::Quaternions, memRes).joints, nullptr);
}

TEST(ScalarJointsFactoryTest, RejectsLODJointIndexBeyondJointCount) {
    pma::AlignedMemoryResource memRes;
    LODJointIndexReader lastJoint{23u};
    ASSERT_NE(createJoints(lastJoint, rl4::RotationType::EulerAngles, memRes).joints, nullptr);
    LODJointIndexReader beyond{24u};
    ASSERT_EQ(createJoints(beyond, rl4::RotationType::EulerAngles, memRes).joints, nullptr);
}

TEST(ScalarJointsFactoryTest, IncreasingRotationLODRowCountsDoNotReadPastRotationRows) {
    pma::AlignedMemoryResource memRes;
    IncreasingRotationLODsReader reader;
    // Whether the rig is accepted is a policy question (the LOD prefix is clamped to the rows that exist); the contract
    // is that the builder never indexes past the LOD-0-sized rotation row array. ASan builds observe the read.
    auto built = createJoints(reader, rl4::RotationType::Quaternions, memRes);
    (void)built;
}

TEST(ScalarJointsFactoryTest, AcceptsJointGroupWithInterleavedJointRows) {
    pma::AlignedMemoryResource memRes;
    for (const bool interleaved : {false, true}) {
        InterleavedJointRowsReader reader{interleaved};
        ASSERT_NE(createJoints(reader, rl4::RotationType::EulerAngles, memRes).joints, nullptr) << "interleaved=" << interleaved;
        ASSERT_NE(createJoints(reader, rl4::RotationType::Quaternions, memRes).joints, nullptr) << "interleaved=" << interleaved;
    }
}

TEST(ScalarJointsFactoryTest, RotationLODCoversEveryJointDrivenByInterleavedRows) {
    pma::AlignedMemoryResource memRes;
    StraddlingRotationRowsReader reader;
    auto built = createJoints(reader, rl4::RotationType::Quaternions, memRes);
    ASSERT_NE(built.joints, nullptr);
    const auto* meta = built.meta.get();

    auto inputs = built.controls->createInstance(&memRes);
    inputs->getInputBuffer()[0] = 1.0f;
    auto outputs = built.joints->createInstance(&memRes);
    // Per-joint layout [tx ty tz qx qy qz qw sx sy sz]; LOD 0 drives all three rows, LOD 1 only the two rx rows.
    const auto expectQuaternion =
        [&meta](rl4::ConstArrayView<float> out, std::size_t base, float rx, float ry, const char* what) {
            const tdm::frad3 euler{tdm::frad{tdm::fdeg{rx}}, tdm::frad{tdm::fdeg{ry}}, tdm::frad{0.0f}};
            const tdm::fquat q{euler, meta->rotationSequence, meta->rotationSigns};
            ASSERT_NEAR(out[base + 3u], q.x, 1e-3f) << what;
            ASSERT_NEAR(out[base + 4u], q.y, 1e-3f) << what;
            ASSERT_NEAR(out[base + 5u], q.z, 1e-3f) << what;
            ASSERT_NEAR(out[base + 6u], q.w, 1e-3f) << what;  // unconverted rows leave qw at 0 (or the neutral 1)
        };
    for (std::uint16_t lod = 0u; lod < 2u; ++lod) {
        built.joints->calculate(inputs.get(), outputs.get(), lod);
        const auto out = outputs->getOutputBuffer();
        ASSERT_EQ(out.size(), 20u);
        // Fatal assertions inside the lambda only leave the lambda; stop the test at the first mismatch.
        expectQuaternion(out, 0u, 90.0f, (lod == 0u) ? 30.0f : 0.0f, (lod == 0u) ? "J0 at lod 0" : "J0 at lod 1");
        if (HasFatalFailure()) {
            return;
        }
        expectQuaternion(out, 10u, 90.0f, 0.0f, (lod == 0u) ? "J1 at lod 0" : "J1 at lod 1");
        if (HasFatalFailure()) {
            return;
        }
    }
}
