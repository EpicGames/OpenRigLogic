// Copyright Epic Games, Inc. All Rights Reserved.

#include "dna/stream/Validator.h"

#include "dna/DNA.h"
#include "dna/StreamReader.h"
#include "dna/stream/StreamReaderStatus.h"

namespace dna {

bool Validator::validate(const DNA& dna) {
    // Every structural defect the reader detects is reported as InvalidDataError; the mesh/blend-shape-channel
    // mapping is a parallel-array mismatch like the joint-group checks below, not a distinct failure class.
    if (dna.definition.meshBlendShapeChannelMapping.sourceSize() != dna.definition.meshBlendShapeChannelMapping.targetSize()) {
        status.set(StreamReader::InvalidDataError);
        return false;
    }

    // Joint-group structure, checked before coordinate-system conversion rewrites the groups (the converter and the
    // cache population index these arrays by the declared dimensions): the coefficient matrix must cover rows x
    // columns, and the per-LOD row counts must carry one entry per LOD (or none, for a group that is never evaluated)
    // and each stay within the group's rows.
    const auto lodCount = static_cast<std::size_t>(dna.descriptor.lodCount);
    for (const auto& jointGroup : dna.behavior.joints.jointGroups) {
        const auto rowCount = jointGroup.outputIndices.size();
        const auto columnCount = jointGroup.inputIndices.size();
        if (jointGroup.values.size() < rowCount * columnCount) {
            status.set(StreamReader::InvalidDataError);
            return false;
        }
        if ((jointGroup.lods.size() != 0ul) && (jointGroup.lods.size() != lodCount)) {
            status.set(StreamReader::InvalidDataError);
            return false;
        }
        for (const auto lodRowCount : jointGroup.lods) {
            if (static_cast<std::size_t>(lodRowCount) > rowCount) {
                status.set(StreamReader::InvalidDataError);
                return false;
            }
        }
    }

    return true;
}

}  // namespace dna
