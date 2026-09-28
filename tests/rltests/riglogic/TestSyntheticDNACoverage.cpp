// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/dna/SyntheticFullReader.h"

#include "riglogic/RigLogic.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/RigLogicImpl.h"
#include "riglogic/riglogic/RigMetadata.h"
#include <cmath>

namespace {

// Everything seeded from examples/Synthetic.dna (fuzz corpus, examples) reaches only the subsystems it activates, so
// these tests pin that to "all of them". The reader is round-tripped through the binary DNA format first because the
// contract must hold for the serialized asset, not the in-memory reader (the format drops what it cannot represent).
class SyntheticDNACoverageTest : public ::testing::Test {
protected:
    void SetUp() override {
        dna::SyntheticFullReader synthetic;
        stream = pma::makeScoped<trio::MemoryStream>();
        auto writer = pma::makeScoped<dna::BinaryStreamWriter>(stream.get());
        writer->setFrom(&synthetic);
        writer->write();
        ASSERT_TRUE(sc::Status::isOk());
        stream->seek(0ul);
        reader = pma::makeScoped<dna::BinaryStreamReader>(stream.get());
        reader->read();
        ASSERT_TRUE(sc::Status::isOk());
    }

    void createRig(rl4::RotationType rotationType) {
        rl4::Configuration config{};
        config.calculationType = rl4::CalculationType::AnyVector;
        config.floatingPointType = rl4::FloatingPointType::Float;
        config.rotationType = rotationType;
        rig = pma::makeScoped<rl4::RigLogic>(reader.get(), config, &memRes);
        ASSERT_NE(rig.get(), nullptr);
    }

    const rl4::RigMetadata* getMetadata() const {
        return static_cast<const rl4::RigLogicImpl*>(rig.get())->getRigMetadata();
    }

protected:
    pma::AlignedMemoryResource memRes;
    pma::ScopedPtr<trio::MemoryStream> stream;
    pma::ScopedPtr<dna::BinaryStreamReader> reader;
    pma::ScopedPtr<rl4::RigLogic> rig;
};

}  // namespace

TEST_F(SyntheticDNACoverageTest, SerializedFormKeepsAllSubsystemData) {
    // Counts that gate entire subsystems must survive DNA serialization.
    ASSERT_GE(reader->getLODCount(), 4u);
    ASSERT_GT(reader->getGUIControlCount(), 0u);
    ASSERT_GT(reader->getRawControlCount(), 0u);
    ASSERT_GT(reader->getPSDCount(), 0u);
    ASSERT_GT(reader->getJointCount(), 0u);
    ASSERT_GT(reader->getJointGroupCount(), 0u);
    ASSERT_GT(reader->getBlendShapeChannelCount(), 0u);
    ASSERT_GT(reader->getAnimatedMapCount(), 0u);
    ASSERT_GT(reader->getMLControlCount(), 0u);
    ASSERT_GT(reader->getMLTypeCount(), 0u);
    ASSERT_GT(reader->getNeuralNetworkCount(), 0u);
    ASSERT_GT(reader->getRBFSolverCount(), 0u);
    ASSERT_GT(reader->getRBFPoseCount(), 0u);
    ASSERT_GT(reader->getTwistCount(), 0u);
    ASSERT_GT(reader->getSwingCount(), 0u);
}

TEST_F(SyntheticDNACoverageTest, EulerConfigBuildsEverySubsystemConcrete) {
    createRig(rl4::RotationType::EulerAngles);
    const auto* meta = getMetadata();
    EXPECT_EQ(meta->evaluators.mlBehavior, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.rbfBehavior, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.psdNet, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.bpcmJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.quaternionJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.twistSwingJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.mlJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.blendShapes, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.animatedMaps, rl4::EvaluatorType::Concrete);
}

TEST_F(SyntheticDNACoverageTest, QuaternionConfigBuildsEverySubsystemConcrete) {
    createRig(rl4::RotationType::Quaternions);
    const auto* meta = getMetadata();
    EXPECT_EQ(meta->evaluators.mlBehavior, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.rbfBehavior, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.psdNet, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.bpcmJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.quaternionJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.twistSwingJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.mlJoints, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.blendShapes, rl4::EvaluatorType::Concrete);
    EXPECT_EQ(meta->evaluators.animatedMaps, rl4::EvaluatorType::Concrete);
}

namespace {

// The DNA's rotation unit persists into the snapshot's ML-joints discriminator, which
// RigMetadata::validate range-checks on the restore path only - so MLJointsBuilder must normalize an
// out-of-range unit at ingestion, or create() accepts a rig whose own dump restore() rejects.
class GarbageRotationUnitReader : public dna::SyntheticFullReader {
public:
    dna::RotationUnit getRotationUnit() const override {
        return static_cast<dna::RotationUnit>(42405);
    }
};

// ML-joints parameter keys and values are independent DNA arrays; every parameter scan (MLJointsBuilder's lambdas and
// the DNA library's own denormalization) must walk only the paired prefix, never past the values array.
class MismatchedMLJointsParamsReader : public dna::SyntheticFullReader {
public:
    dna::ConstArrayView<std::uint16_t> getMLJointsParameterValues() const override {
        static const std::uint16_t v[] = {0u, 1u};  // 2 values against SyntheticFullReader's 10 keys
        return {v, 2ul};
    }
};

// The well-formed twin of MismatchedMLJointsParamsReader (keys truncated to the same prefix); the two must be
// indistinguishable, since keys in the unpaired tail have no values and must not certify a coordinate system.
class PairedPrefixMLJointsParamsReader : public dna::SyntheticFullReader {
public:
    dna::ConstArrayView<std::uint16_t> getMLJointsParameterKeys() const override {
        static const std::uint16_t k[] = {0u, 1u};
        return {k, 2ul};
    }

    dna::ConstArrayView<std::uint16_t> getMLJointsParameterValues() const override {
        static const std::uint16_t v[] = {0u, 1u};
        return {v, 2ul};
    }
};

// A net with zero outputs (empty biases) next to a full Scatter list: defaultValues is sized by the net's output count
// while the scatter list is an independent DNA array, so getDefaultValues must walk only the span both cover.
class ZeroOutputNetReader : public dna::SyntheticFullReader {
public:
    dna::ConstArrayView<float> getNeuralNetworkLayerBiases(std::uint16_t neuralNetIndex,
                                                           std::uint16_t layerIndex) const override {
        if (neuralNetIndex == 1u) {
            return {};
        }
        return SyntheticFullReader::getNeuralNetworkLayerBiases(neuralNetIndex, layerIndex);
    }
};

// Joint group LOD row counts are raw DNA values nothing forces to be non-increasing; a count above the LOD-0 row count
// must not drive QuaternionJointsBuilder::setLODs past the filtered output-index array (the storage validator runs later).
class IncreasingJointGroupLODsReader : public dna::SyntheticFullReader {
public:
    dna::ConstArrayView<std::uint16_t> getJointGroupLODs(std::uint16_t jointGroupIndex) const override {
        static const std::uint16_t g1[] = {4u, 4u, 4u, 9u};  // LOD 3 claims more rows than the group has
        if (jointGroupIndex == 1u) {
            return {g1, 4ul};
        }
        return SyntheticFullReader::getJointGroupLODs(jointGroupIndex);
    }
};

}  // namespace

TEST(SyntheticDNAHostileVariants, IncreasingJointGroupLODRowCountsDoNotCrash) {
    IncreasingJointGroupLODsReader synthetic;
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::AnyVector;
    config.floatingPointType = rl4::FloatingPointType::Float;
    // Whether create() accepts or rejects this rig is a policy question; not crashing is the contract.
    auto rig = pma::makeScoped<rl4::RigLogic>(&synthetic, config, &memRes);
    if (rig.get() != nullptr) {
        auto instance = pma::makeScoped<rl4::RigInstance>(rig.get(), &memRes);
        rig->calculate(instance.get());
    }
}

TEST(SyntheticDNAHostileVariants, OutOfRangeJointGroupIndexIsIgnored) {
    dna::SyntheticFullReader synthetic;
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::AnyVector;
    config.floatingPointType = rl4::FloatingPointType::Float;
    auto rig = pma::makeScoped<rl4::RigLogic>(&synthetic, config, &memRes);
    ASSERT_NE(rig.get(), nullptr);
    auto instance = pma::makeScoped<rl4::RigInstance>(rig.get(), &memRes);
    // The documented contract bounds the index by getJointGroupCount(), but a hostile snapshot can make
    // that count exceed the deserialized containers, so compliant callers reach out-of-range indices too;
    // they must be no-ops, not out-of-bounds reads.
    rig->calculateJoints(instance.get(), rig->getJointGroupCount());
    rig->calculateJoints(instance.get(), static_cast<std::uint16_t>(65535u));
}

TEST(SyntheticDNAHostileVariants, ZeroOutputNetWithScatterListDoesNotCrash) {
    ZeroOutputNetReader synthetic;
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::AnyVector;
    config.floatingPointType = rl4::FloatingPointType::Float;
    // Whether create() accepts or rejects this rig is a policy question; not crashing is the contract.
    auto rig = pma::makeScoped<rl4::RigLogic>(&synthetic, config, &memRes);
    if (rig.get() != nullptr) {
        auto instance = pma::makeScoped<rl4::RigInstance>(rig.get(), &memRes);
        rig->calculate(instance.get());
    }
}

TEST(SyntheticDNAHostileVariants, UnpairedMLJointsParamKeysAreInert) {
    // Differential oracle: coordinate-system keys sitting in the unpaired tail of the keys array must
    // not change behavior relative to the same rig without them - certifying them would run the
    // ML-joints change-of-basis on the value readers' zero-initialized defaults.
    auto evaluate = [](const dna::Reader* reader, rl4::Vector<float>& outputs, pma::MemoryResource* memRes) {
        rl4::Configuration config{};
        config.calculationType = rl4::CalculationType::AnyVector;
        config.floatingPointType = rl4::FloatingPointType::Float;
        auto rig = pma::makeScoped<rl4::RigLogic>(reader, config, memRes);
        ASSERT_NE(rig.get(), nullptr);
        auto instance = pma::makeScoped<rl4::RigInstance>(rig.get(), memRes);
        for (std::uint16_t i = {}; i < instance->getRawControlCount(); ++i) {
            instance->setRawControl(i, 0.5f);
        }
        rig->calculate(instance.get());
        const auto jointOutputs = instance->getJointOutputs();
        outputs.assign(jointOutputs.begin(), jointOutputs.end());
    };

    pma::AlignedMemoryResource memRes;
    MismatchedMLJointsParamsReader unpairedTail;
    PairedPrefixMLJointsParamsReader pairedOnly;
    rl4::Vector<float> unpairedOutputs{&memRes};
    rl4::Vector<float> pairedOutputs{&memRes};
    evaluate(&unpairedTail, unpairedOutputs, &memRes);
    evaluate(&pairedOnly, pairedOutputs, &memRes);
    ASSERT_EQ(unpairedOutputs.size(), pairedOutputs.size());
    for (std::size_t i = {}; i < unpairedOutputs.size(); ++i) {
        ASSERT_EQ(unpairedOutputs[i], pairedOutputs[i]) << "at joint output " << i;
    }
}

TEST(SyntheticDNAHostileVariants, MismatchedMLJointsParamArraysDoNotCrash) {
    MismatchedMLJointsParamsReader synthetic;
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::AnyVector;
    config.floatingPointType = rl4::FloatingPointType::Float;
    auto rig = pma::makeScoped<rl4::RigLogic>(&synthetic, config, &memRes);
    ASSERT_NE(rig.get(), nullptr);

    auto snapshot = pma::makeScoped<trio::MemoryStream>();
    rig->dump(snapshot.get());
    snapshot->seek(0ul);
    auto restored = rl4::RigLogic::restore(snapshot.get(), &memRes);
    ASSERT_NE(restored, nullptr);
    rl4::RigLogic::destroy(restored);
}

TEST(SyntheticDNAHostileVariants, GarbageRotationUnitDumpStillRestores) {
    GarbageRotationUnitReader synthetic;
    pma::AlignedMemoryResource memRes;
    rl4::Configuration config{};
    config.calculationType = rl4::CalculationType::AnyVector;
    config.floatingPointType = rl4::FloatingPointType::Float;
    auto rig = pma::makeScoped<rl4::RigLogic>(&synthetic, config, &memRes);
    ASSERT_NE(rig.get(), nullptr);

    auto snapshot = pma::makeScoped<trio::MemoryStream>();
    rig->dump(snapshot.get());
    snapshot->seek(0ul);
    auto restored = rl4::RigLogic::restore(snapshot.get(), &memRes);
    ASSERT_NE(restored, nullptr);
    rl4::RigLogic::destroy(restored);
}

TEST_F(SyntheticDNACoverageTest, GUIControlSweepProducesFiniteOutputs) {
    // The rig's quaternion quad raw[0,4) must resolve to a valid quaternion at every GUI-driven pose,
    // including neutral - a zero quaternion normalizes to NaN, which the fuzz triage tooling (and any
    // consumer) would surface as non-finite outputs from clean input.
    createRig(rl4::RotationType::EulerAngles);
    auto instance = pma::makeScoped<rl4::RigInstance>(rig.get(), &memRes);
    auto allFinite = [](rl4::ConstArrayView<float> values) {
        for (const float v : values) {
            if (!std::isfinite(v)) {
                return false;
            }
        }
        return true;
    };
    for (std::uint16_t lod = {}; lod < rig->getLODCount(); ++lod) {
        instance->setLOD(lod);
        for (std::uint16_t pass = 0u; pass < 3u; ++pass) {
            const float value = (pass == 0u) ? 0.0f : ((pass == 1u) ? 1.0f : -1.0f);
            for (std::uint16_t i = {}; i < instance->getGUIControlCount(); ++i) {
                instance->setGUIControl(i, value);
            }
            rig->mapGUIToRawControls(instance.get());
            rig->calculate(instance.get());
            ASSERT_TRUE(allFinite(instance->getJointOutputs())) << "lod " << lod << " pass " << pass;
            ASSERT_TRUE(allFinite(instance->getBlendShapeOutputs())) << "lod " << lod << " pass " << pass;
            ASSERT_TRUE(allFinite(instance->getAnimatedMapOutputs())) << "lod " << lod << " pass " << pass;
        }
    }
}

TEST_F(SyntheticDNACoverageTest, EvaluatesAndRoundTripsThroughSnapshot) {
    createRig(rl4::RotationType::EulerAngles);
    auto instance = pma::makeScoped<rl4::RigInstance>(rig.get(), &memRes);
    for (std::uint16_t lod = {}; lod < rig->getLODCount(); ++lod) {
        instance->setLOD(lod);
        for (std::uint16_t i = {}; i < instance->getRawControlCount(); ++i) {
            instance->setRawControl(i, 0.5f);
        }
        rig->calculate(instance.get());
    }

    auto snapshot = pma::makeScoped<trio::MemoryStream>();
    rig->dump(snapshot.get());
    snapshot->seek(0ul);
    auto restored = rl4::RigLogic::restore(snapshot.get(), &memRes);
    ASSERT_NE(restored, nullptr);
    auto restoredInstance = rl4::RigInstance::create(restored, &memRes);
    restored->calculate(restoredInstance);
    rl4::RigInstance::destroy(restoredInstance);
    rl4::RigLogic::destroy(restored);
}
