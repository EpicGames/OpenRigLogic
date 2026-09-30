// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <dna/BinaryStreamReader.h>
#include <dna/BinaryStreamWriter.h>
#include <dna/Configuration.h>
#include <dna/types/Aliases.h>
#include <dna/types/Vector3.h>
#include <pma/MemoryResource.h>
#include <pma/ScopedPtr.h>
#include <pma/resources/AlignedMemoryResource.h>
#include <pma/resources/ArenaMemoryResource.h>
#include <pma/resources/DefaultMemoryResource.h>
#include <status/Status.h>
#include <tdm/TDM.h>
#include <trio/Stream.h>
#include <trio/streams/FileStream.h>
#include <trio/streams/MemoryMappedFileStream.h>
#include <trio/streams/MemoryStream.h>

namespace rl4 {

using dna::ArrayView;
using dna::BinaryStreamReader;
using dna::BinaryStreamWriter;
using dna::ConstArrayView;
using dna::DataLayer;
using dna::StringView;
using dna::UnknownLayerPolicy;
using dna::Vector3;
using sc::Status;
using tdm::fquat;
using tdm::fvec3;
using trio::BoundedIOStream;
using trio::FileStream;
using trio::MemoryMappedFileStream;
using trio::MemoryStream;

using pma::AlignedMemoryResource;
using pma::ArenaMemoryResource;
using pma::DefaultInstanceCreator;
using pma::DefaultInstanceDestroyer;
using pma::DefaultMemoryResource;
using pma::Delete;
using pma::FactoryCreate;
using pma::FactoryDestroy;
using pma::makeScoped;
using pma::MemoryResource;
using pma::New;
using pma::ScopedPtr;

}  // namespace rl4
