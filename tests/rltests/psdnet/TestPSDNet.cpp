// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"
#include "rltests/controls/ControlFixtures.h"
#include "rltests/dna/FakeReader.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/controls/ControlsFactory.h"
#include "riglogic/psdnet/PSDNetFactory.h"
#include "riglogic/psdnet/PSDNetImpl.h"
#include "riglogic/psdnet/PSDNetImplOutputInstance.h"
#include "riglogic/riglogic/Configuration.h"
#include "riglogic/riglogic/RigMetadata.h"

TEST(PSDNetTest, PSDsAppendToOutput) {
    pma::AlignedMemoryResource amr;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0u, 2u, 2u, 0u, 0u);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &amr);
    auto inputs = inputInstance->getInputBuffer();
    inputs[0] = 0.1f;
    inputs[1] = 0.2f;

    rl4::PSDNetImplOutputInstance outputInstance{4u, &amr};

    rl4::Matrix<std::uint16_t> inputLODs{{0u, 1u}};
    rl4::Matrix<std::uint16_t> outputLODs{{2u, 3u}};
    rl4::Vector<std::uint16_t> cols{0u, 1u, 0u, 1u};
    // PSD weights {4.0f, 3.0f, 0.5f, 2.0f}
    rl4::Vector<rl4::PSD> psds{{0u, 2u, 12.0f}, {2u, 2u, 1.0f}};
    rl4::PSDNetImpl psdNet{std::move(inputLODs), std::move(outputLODs), std::move(cols), std::move(psds), 2u, 3u, nullptr};

    const float expected[] = {0.1f, 0.2f, 0.24f, 0.02f};
    psdNet.calculate(inputInstance.get(), &outputInstance, 0u);
    ASSERT_ELEMENTS_NEAR(inputs, expected, 4ul, 0.0001f);
}

TEST(PSDNetTest, OutputsAreClamped) {
    pma::AlignedMemoryResource amr;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0u, 1u, 1u, 0u, 0u);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &amr);
    auto inputs = inputInstance->getInputBuffer();
    inputs[0] = 0.1f;

    rl4::PSDNetImplOutputInstance outputInstance{2u, &amr};

    rl4::Matrix<std::uint16_t> inputLODs{{0u}};
    rl4::Matrix<std::uint16_t> outputLODs{{1u}};
    rl4::Vector<std::uint16_t> cols{0u};
    // PSD weights {100.0f}
    rl4::Vector<rl4::PSD> psds{{0u, 1u, 100.0f}};
    rl4::PSDNetImpl psdNet{std::move(inputLODs), std::move(outputLODs), std::move(cols), std::move(psds), 1u, 1u, nullptr};

    const float expected[] = {0.1f, 1.0f};
    psdNet.calculate(inputInstance.get(), &outputInstance, 0u);
    ASSERT_ELEMENTS_EQ(inputs, expected, 2);
}

TEST(PSDNetTest, OutputsKeepExistingProduct) {
    pma::AlignedMemoryResource amr;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0u, 2u, 1u, 0u, 0u);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &amr);
    auto inputs = inputInstance->getInputBuffer();
    inputs[0] = 0.1f;
    inputs[1] = 0.2f;

    rl4::PSDNetImplOutputInstance outputInstance{3u, &amr};

    rl4::Matrix<std::uint16_t> inputLODs{{0u, 1u}};
    rl4::Matrix<std::uint16_t> outputLODs{{2u}};
    rl4::Vector<std::uint16_t> cols{0u, 1u};
    // PSD weights {4.0f, 10.0f}
    rl4::Vector<rl4::PSD> psds{{0u, 2u, 40.0f}};
    rl4::PSDNetImpl psdNet{std::move(inputLODs), std::move(outputLODs), std::move(cols), std::move(psds), 2u, 2u, nullptr};

    const float expected[] = {0.1f, 0.2f, 0.8f};
    psdNet.calculate(inputInstance.get(), &outputInstance, 0u);
    ASSERT_ELEMENTS_EQ(inputs, expected, 3ul);
}

TEST(PSDNetTest, RowsSpecifyDestinationIndex) {
    pma::AlignedMemoryResource amr;

    auto inputInstanceFactory = ControlsFactory::getInstanceFactory(0u, 2u, 2u, 0u, 0u);
    rl4::Vector<rl4::ControlInitializer> initialValues;
    auto inputInstance = inputInstanceFactory(initialValues, &amr);
    auto inputs = inputInstance->getInputBuffer();
    inputs[0] = 0.1f;
    inputs[1] = 0.2f;

    rl4::PSDNetImplOutputInstance outputInstance{4u, &amr};

    rl4::Matrix<std::uint16_t> inputLODs{{0u, 1u}};
    rl4::Matrix<std::uint16_t> outputLODs{{2u, 3u}};
    rl4::Vector<std::uint16_t> cols{0u, 1u};
    // PSD weights {4.0f, 3.0f}
    rl4::Vector<rl4::PSD> psds{{0u, 1u, 4.0f}, {1u, 1u, 3.0f}};
    rl4::PSDNetImpl psdNet{std::move(inputLODs), std::move(outputLODs), std::move(cols), std::move(psds), 2u, 2u, nullptr};

    const float expected[] = {0.1f, 0.2f, 0.4f, 0.6f};
    psdNet.calculate(inputInstance.get(), &outputInstance, 0u);
    ASSERT_ELEMENTS_EQ(inputs, expected, 4ul);
}

// The clamp buffer mirrors the combined raw + PSD + ML + RBF control buffer; those are four independent uint16 DNA
// scalars, so their sum can exceed uint16 and must not be narrowed on the way into this allocation.
TEST(PSDNetTest, ClampBufferKeepsCombinedControlCountAboveUint16) {
    pma::AlignedMemoryResource amr;
    static constexpr std::size_t controlCount = 65000u + 500u + 40u;  // raw + psd + ml
    rl4::PSDNetImplOutputInstance outputInstance{controlCount, &amr};
    ASSERT_EQ(outputInstance.getClampBuffer().size(), controlCount);
    // Every uint16 column index a validated PSD can carry addresses a slot in this buffer.
    outputInstance.getClampBuffer()[65535u] = 1.0f;
    ASSERT_EQ(outputInstance.getClampBuffer()[65535u], 1.0f);
}

namespace {

// 65000 raw + 500 PSD + 40 ML controls (65540 combined, past uint16) with one PSD: control 65000 = 1.0 x control 1000.
// The highest PSD index (65499) passes the factory's own uint16 guard, so only the combined count is oversized.
class WideControlSpaceReader : public dna::FakeReader {
public:
    ~WideControlSpaceReader();

    std::uint16_t getLODCount() const override {
        return 1u;
    }

    std::uint16_t getRawControlCount() const override {
        return 65000u;
    }

    std::uint16_t getPSDCount() const override {
        return 500u;
    }

    std::uint16_t getMLControlCount() const override {
        return 40u;
    }

    rl4::ConstArrayView<std::uint16_t> getPSDRowIndices() const override {
        static const std::uint16_t rows[] = {65000u};
        return {rows, 1ul};
    }

    rl4::ConstArrayView<std::uint16_t> getPSDColumnIndices() const override {
        static const std::uint16_t cols[] = {1000u};
        return {cols, 1ul};
    }

    rl4::ConstArrayView<float> getPSDValues() const override {
        static const float values[] = {1.0f};
        return {values, 1ul};
    }
};

WideControlSpaceReader::~WideControlSpaceReader() = default;

}  // namespace

// The reader-driven factory path: the combined count reaches the clamp buffer unnarrowed, so the instance the factory
// hands out has one clamp slot per control and evaluation writes the PSD product in place.
TEST(PSDNetTest, FactoryKeepsCombinedControlCountAboveUint16) {
    pma::AlignedMemoryResource amr;
    WideControlSpaceReader reader;
    rl4::Configuration config{};
    auto meta = rl4::RigMetadata::create(config, &reader, &amr);
    ASSERT_NE(meta, nullptr);
    auto controls = rl4::ControlsFactory::create(config, meta.get(), &reader, &amr);
    ASSERT_NE(controls, nullptr);
    // A PSD is evaluated at a LOD only when some consumer registers its control there (the joint builders do this).
    static const std::uint16_t consumed[] = {65000u};
    controls->registerControls(0u, rl4::ConstArrayView<std::uint16_t>{consumed, 1ul});
    auto psdNet = rl4::PSDNetFactory::create(config, meta.get(), &reader, controls.get(), &amr);
    ASSERT_NE(psdNet, nullptr);

    auto outputInstance = psdNet->createInstance(&amr);
    ASSERT_EQ(outputInstance->getClampBuffer().size(), 65540u);

    auto inputInstance = controls->createInstance(&amr);
    auto inputs = inputInstance->getInputBuffer();
    ASSERT_EQ(inputs.size(), 65540u);
    inputs[1000u] = 0.5f;
    psdNet->calculate(inputInstance.get(), outputInstance.get(), 0u);
    ASSERT_NEAR(inputs[65000u], 0.5f, 1e-6f);
}
