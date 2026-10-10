// Copyright Epic Games, Inc. All Rights Reserved.

#include "dna/BinaryStreamReader.h"
#include "dna/Reader.h"
#include "trio/streams/FileStream.h"
#include "status/Status.h"

#include <iostream>
#include <iomanip>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "OpenRigLogic DNA Inspection Tool\n"
                  << "Usage: " << argv[0] << " <path_to_dna_file>\n";
        return 1;
    }

    const char* filePath = argv[1];
    auto stream = trio::makeScoped<trio::FileStream>(
        filePath, trio::FileStream::AccessMode::Read, trio::FileStream::OpenMode::Binary);

    if (!stream) {
        std::cerr << "Error: Unable to open file: " << filePath << "\n";
        return 1;
    }

    auto reader = dna::makeScoped<dna::BinaryStreamReader>(stream.get(), dna::DataLayer::All);
    reader->read();

    if (!dna::Status::isOk()) {
        std::cerr << "Error: Failed to parse DNA file (" 
                  << dna::Status::get().message << ")\n";
        return 1;
    }

    std::cout << "========================================\n"
              << " MetaHuman DNA File Summary\n"
              << "========================================\n"
              << "Rig Name:        " << reader->getName() << "\n"
              << "LOD Count:       " << reader->getLODCount() << "\n"
              << "Joints:          " << reader->getJointCount() << "\n"
              << "Meshes:          " << reader->getMeshCount() << "\n"
              << "BlendShapes:     " << reader->getBlendShapeChannelCount() << "\n"
              << "Animated Maps:   " << reader->getAnimatedMapCount() << "\n"
              << "RBF Solvers:     " << reader->getRBFSolverCount() << "\n"
              << "ML Controllers:  " << reader->getMLControlCount() << "\n"
              << "========================================\n";

    if (reader->getJointCount() > 0) {
        std::cout << "First 5 Joints:\n";
        std::uint16_t sampleLimit = std::min(reader->getJointCount(), static_cast<std::uint16_t>(5));
        for (std::uint16_t i = 0; i < sampleLimit; ++i) {
            std::cout << "  [" << i << "] " << reader->getJointName(i) << "\n";
        }
    }

    return 0;
}
