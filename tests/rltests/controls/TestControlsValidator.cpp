// Copyright Epic Games, Inc. All Rights Reserved.

#include "rltests/Defs.h"

#include "riglogic/TypeDefs.h"
#include "riglogic/conditionaltable/ConditionalTable.h"
#include "riglogic/controls/ControlsInputInstance.h"
#include "riglogic/controls/ControlsValidator.h"
#include "riglogic/riglogic/RigMetadata.h"

namespace controlsvalidatortest {

rl4::RigMetadata makeMetadata(rl4::MemoryResource* memRes, std::uint16_t guiControlCount, std::uint16_t rawControlCount) {
    rl4::RigMetadata meta{memRes};
    meta.guiControlCount = guiControlCount;
    meta.rawControlCount = rawControlCount;  // controlInputCount = raw + psd + ml + rbf; the rest stay zero
    return meta;
}

template<typename T, std::size_t N>
rl4::Vector<T> makeVector(const T (&values)[N], rl4::MemoryResource* memRes) {
    return rl4::Vector<T>{values, values + N, memRes};
}

rl4::ConditionalTable makeMapping(std::uint16_t inputIndex,
                                  std::uint16_t outputIndex,
                                  std::uint16_t inputCount,
                                  std::uint16_t outputCount,
                                  rl4::MemoryResource* memRes) {
    const std::uint16_t inputIndices[] = {inputIndex};
    const std::uint16_t outputIndices[] = {outputIndex};
    const float fromValues[] = {0.0f};
    const float toValues[] = {1.0f};
    const float slopeValues[] = {1.0f};
    const float cutValues[] = {0.0f};
    return rl4::ConditionalTable{makeVector(inputIndices, memRes),
                                 makeVector(outputIndices, memRes),
                                 makeVector(fromValues, memRes),
                                 makeVector(toValues, memRes),
                                 makeVector(slopeValues, memRes),
                                 makeVector(cutValues, memRes),
                                 inputCount,
                                 outputCount,
                                 memRes};
}

}  // namespace controlsvalidatortest

TEST(ControlsValidatorTest, AcceptsWellFormedData) {
    pma::AlignedMemoryResource memRes;
    auto mapping = controlsvalidatortest::makeMapping(1u,
                                                      2u,
                                                      3u,
                                                      4u,
                                                      &memRes);  // input < guiControlCount (3), output < controlInputCount (4)
    const rl4::ControlInitializer initValues[] = {{3u, 1.0f}};   // index < controlInputCount (4)
    auto initialValues = controlsvalidatortest::makeVector(initValues, &memRes);
    auto meta = controlsvalidatortest::makeMetadata(&memRes, 3u, 4u);
    ASSERT_TRUE(rl4::ControlsValidator::validate(mapping, initialValues, meta));
}

TEST(ControlsValidatorTest, RejectsInputIndexBeyondGUIControlCount) {
    pma::AlignedMemoryResource memRes;
    auto mapping = controlsvalidatortest::makeMapping(5u, 2u, 3u, 4u, &memRes);  // input index 5 >= guiControlCount (3)
    rl4::Vector<rl4::ControlInitializer> initialValues{&memRes};
    auto meta = controlsvalidatortest::makeMetadata(&memRes, 3u, 4u);
    ASSERT_FALSE(rl4::ControlsValidator::validate(mapping, initialValues, meta));
}

TEST(ControlsValidatorTest, RejectsOutputIndexBeyondControlInputCount) {
    pma::AlignedMemoryResource memRes;
    auto mapping = controlsvalidatortest::makeMapping(1u, 9u, 3u, 4u, &memRes);  // output index 9 >= controlInputCount (4)
    rl4::Vector<rl4::ControlInitializer> initialValues{&memRes};
    auto meta = controlsvalidatortest::makeMetadata(&memRes, 3u, 4u);
    ASSERT_FALSE(rl4::ControlsValidator::validate(mapping, initialValues, meta));
}

TEST(ControlsValidatorTest, RejectsInputCountBeyondGUIControlCount) {
    pma::AlignedMemoryResource memRes;
    auto mapping =
        controlsvalidatortest::makeMapping(1u,
                                           2u,
                                           9u,
                                           4u,
                                           &memRes);  // fill_n(inputs, inputCount) with inputCount 9 > guiControlCount (3)
    rl4::Vector<rl4::ControlInitializer> initialValues{&memRes};
    auto meta = controlsvalidatortest::makeMetadata(&memRes, 3u, 4u);
    ASSERT_FALSE(rl4::ControlsValidator::validate(mapping, initialValues, meta));
}

TEST(ControlsValidatorTest, RejectsOutputCountBeyondControlInputCount) {
    pma::AlignedMemoryResource memRes;
    auto mapping =
        controlsvalidatortest::makeMapping(1u,
                                           2u,
                                           3u,
                                           9u,
                                           &memRes);  // fill_n(outputs, outputCount) with outputCount 9 > controlInputCount (4)
    rl4::Vector<rl4::ControlInitializer> initialValues{&memRes};
    auto meta = controlsvalidatortest::makeMetadata(&memRes, 3u, 4u);
    ASSERT_FALSE(rl4::ControlsValidator::validate(mapping, initialValues, meta));
}

TEST(ControlsValidatorTest, RejectsInitialValueIndexBeyondControlInputCount) {
    pma::AlignedMemoryResource memRes;
    auto mapping = controlsvalidatortest::makeMapping(1u, 2u, 3u, 4u, &memRes);
    const rl4::ControlInitializer initValues[] = {{4u, 1.0f}};  // index 4 >= controlInputCount (4); resetInputBuffer OOB
    auto initialValues = controlsvalidatortest::makeVector(initValues, &memRes);
    auto meta = controlsvalidatortest::makeMetadata(&memRes, 3u, 4u);
    ASSERT_FALSE(rl4::ControlsValidator::validate(mapping, initialValues, meta));
}
