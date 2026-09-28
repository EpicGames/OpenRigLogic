// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

namespace rl4 {

struct Configuration;
struct RigMetadata;

// Non-serialized context passed via the archive's user-data pointer during dump()/restore(), read by each load():
// config selects the runtime code path (SIMD/float type); metadata carries the rig dimensions load() validates
// its offsets/sizes against.
struct SerializationContext {
    const Configuration* config;
    const RigMetadata* metadata;
};

}  // namespace rl4
