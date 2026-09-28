// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "riglogic/TypeDefs.h"
#include "riglogic/riglogic/RigMetadata.h"

#include <dna/Reader.h>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace rl4 {

// The reader overload guards the raw DNA fields JointsFactory::create() hands to the builders; the other guards the
// data both Joints::load() and create() retain. jointIndices values are identifiers handed back to the caller, never
// dereferenced internally, so the data overload carries no bound for them (RigMetadata has no jointCount).
struct JointsValidator {
    // Joint and column indices the builders would otherwise remap or narrow out of range.
    static bool validate(const dna::Reader* reader) {
        constexpr std::size_t dnaAttrsPerJoint = 9ul;
        constexpr std::size_t maxColumnCount = static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max());
        const std::size_t jointCount = static_cast<std::size_t>(reader->getJointCount());
        const std::size_t attributeDomain = jointCount * dnaAttrsPerJoint;

        for (std::uint16_t jgi = {}; jgi < reader->getJointGroupCount(); ++jgi) {
            if (reader->getJointGroupInputIndices(jgi).size() > maxColumnCount) {
                return false;
            }
            for (const auto outputIndex : reader->getJointGroupOutputIndices(jgi)) {
                if (static_cast<std::size_t>(outputIndex) >= attributeDomain) {
                    return false;
                }
            }
        }
        for (std::uint16_t lod = {}; lod < reader->getLODCount(); ++lod) {
            for (const auto jointIndex : reader->getJointIndicesForLOD(lod)) {
                if (static_cast<std::size_t>(jointIndex) >= jointCount) {
                    return false;
                }
            }
        }
        for (std::uint16_t si = {}; si < reader->getSwingCount(); ++si) {
            for (const auto jointIndex : reader->getSwingOutputJointIndices(si)) {
                if (static_cast<std::size_t>(jointIndex) >= jointCount) {
                    return false;
                }
            }
        }
        for (std::uint16_t ti = {}; ti < reader->getTwistCount(); ++ti) {
            for (const auto jointIndex : reader->getTwistOutputJointIndices(ti)) {
                if (static_cast<std::size_t>(jointIndex) >= jointCount) {
                    return false;
                }
            }
        }
        return true;
    }

    static bool validate(const Vector<float>& neutralValues,
                         const Matrix<std::uint16_t>& variableAttributeIndices,
                         const Matrix<std::uint16_t>& jointIndices,
                         const RigMetadata& metadata,
                         bool loadJoints) {
        const std::size_t lodCount = static_cast<std::size_t>(metadata.lodCount);
        const std::size_t jointAttributeCount = static_cast<std::size_t>(metadata.jointAttributeCount);

        // A floor, not just a ceiling: lodCount is deserialized independently of these matrices and RigInstance clamps the
        // runtime lod against lodCount only, so a short matrix would leave [lod] out of bounds.
        if ((variableAttributeIndices.size() != lodCount) || (jointIndices.size() != lodCount)) {
            return false;
        }

        // Floor, not equality: jointAttributeCount is a uint16 narrowing of the product neutralValues is sized from. Gated on
        // loadJoints: the Null shell for loadJoints=false on a DNA with joints dumps empty neutralValues under the real count.
        if (loadJoints && (jointAttributeCount != 0ul) && (lodCount != 0ul) && (neutralValues.size() < jointAttributeCount)) {
            return false;
        }

        // Attribute indices address the joint-attribute buffer.
        for (const auto& indicesForLOD : variableAttributeIndices) {
            for (const auto attributeIndex : indicesForLOD) {
                if (static_cast<std::size_t>(attributeIndex) >= jointAttributeCount) {
                    return false;
                }
            }
        }

        return true;
    }
};

}  // namespace rl4
