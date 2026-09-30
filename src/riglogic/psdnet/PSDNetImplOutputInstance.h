// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/psdnet/PSDNetOutputInstance.h"

#include <cstdint>

namespace rl4 {

class PSDNetImplOutputInstance : public PSDNetOutputInstance {
public:
    // Combined raw + PSD + ML + RBF count; the sum can exceed uint16.
    PSDNetImplOutputInstance(std::size_t controlCount, MemoryResource* memRes);
    void resetClampBuffer() override;
    ArrayView<float> getClampBuffer() override;

private:
    Vector<float> clampBuffer;
};

}  // namespace rl4
