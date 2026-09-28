// Copyright Epic Games, Inc. All Rights Reserved.

#include <riglogic/RigLogic.h>

#include <cmath>
#include <iostream>

const float expectedJointDeltas[] = {
    0.699999988f,      // spine_04.translation.x
    0.00000000f,       // spine_04.translation.y
    0.200000003f,      // spine_04.translation.z
    -1.02342628e-05f,  // spine_04.rotation.qx
    0.00488569820f,    // spine_04.rotation.qy
    0.00209470955f,    // spine_04.rotation.qz
    0.999985874f,      // spine_04.rotation.qw
    0.00000000f,       // spine_04.scale.x
    0.00000000f,       // spine_04.scale.y
    0.330053717f,      // spine_04.scale.z
    0.00000000f,       // spine_05.translation.x
    0.256729037f,      // spine_05.translation.y
    0.114503264f,      // spine_05.translation.z
    0.00446887268f,    // spine_05.rotation.qx
    0.00374362129f,    // spine_05.rotation.qy
    0.00196823012f,    // spine_05.rotation.qz
    0.999754012f,      // spine_05.rotation.qw
    0.205333084f,      // spine_05.scale.x
    0.0915802494f,     // spine_05.scale.y
    0.00000000f        // spine_05.scale.z
};
const float expectedBlendShapeChannels[] = {
    0.699999988f,  // brow_down_L
    0.699999988f,  // brow_down_R
    0.400000006f,  // brow_lateral_L
    0.400000006f   // brow_lateral_R
};
const float expectedAnimatedMaps[] = {
    0.699999988f,  // head_cm2_color.head_wm2_browsDown_L
    0.625000000f,  // head_cm2_color.head_wm2_browsDown_R
    0.600000024f,  // head_cm2_color.head_wm2_browsLateral_L
    1.00000000f    // head_cm2_color.head_wm2_browsLateral_R
};

#define ASSERT_NEAR(a, b, threshold) assert(std::fabs(a - b) <= threshold);

template<typename T, std::size_t N>
std::size_t arrayLength(const T (&)[N]) {
    return N;
}

int main(int argc, char** argv) {
    auto stream =
        rl4::makeScoped<rl4::FileStream>("Sample.dna", rl4::FileStream::AccessMode::Read, rl4::FileStream::OpenMode::Binary);
    auto reader = rl4::makeScoped<rl4::BinaryStreamReader>(stream.get());
    reader->read();
    if (!rl4::Status::isOk()) {
        auto status = rl4::Status::get();
        std::cout << status.message << std::endl;
        return -1;
    }

    rl4::Configuration rlConfig;
    rlConfig.rotationType = rl4::RotationType::Quaternions;
    auto rigLogic = rl4::makeScoped<rl4::RigLogic>(reader.get(), rlConfig);
    auto rigInstance = rl4::makeScoped<rl4::RigInstance>(rigLogic.get());

    auto rawCtrls = rigInstance->getRawControlValues();
    rawCtrls[0] = 0.7f;
    rawCtrls[1] = 0.4f;

    rigLogic->calculate(rigInstance.get());

    // The tables mirror the shipped Sample.dna; if the asset changed, fail rather than compare past a table's end.
    const auto jointDeltas = rigInstance->getJointOutputs();
    const auto blendShapeChannels = rigInstance->getBlendShapeOutputs();
    const auto animatedMaps = rigInstance->getAnimatedMapOutputs();
    if ((jointDeltas.size() != arrayLength(expectedJointDeltas)) ||
        (blendShapeChannels.size() != arrayLength(expectedBlendShapeChannels)) ||
        (animatedMaps.size() != arrayLength(expectedAnimatedMaps))) {
        std::cout << "Output counts do not match the expected-value tables; regenerate them for this Sample.dna" << std::endl;
        return -1;
    }

    for (std::size_t i = {}; i < jointDeltas.size(); ++i) {
        ASSERT_NEAR(jointDeltas[i], expectedJointDeltas[i], 0.001f);
    }

    for (std::size_t i = {}; i < blendShapeChannels.size(); ++i) {
        ASSERT_NEAR(blendShapeChannels[i], expectedBlendShapeChannels[i], 0.001f);
    }

    for (std::size_t i = {}; i < animatedMaps.size(); ++i) {
        ASSERT_NEAR(animatedMaps[i], expectedAnimatedMaps[i], 0.001f);
    }

    return 0;
}
