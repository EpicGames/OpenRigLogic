// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <pma/MemoryResource.h>
#include <pma/ScopedPtr.h>
#include <status/Status.h>
#include <status/StatusCode.h>

namespace trio {

using sc::Status;

using pma::DefaultInstanceCreator;
using pma::DefaultInstanceDestroyer;
using pma::Delete;
using pma::FactoryCreate;
using pma::FactoryDestroy;
using pma::makeScoped;
using pma::MemoryResource;
using pma::New;
using pma::ScopedPtr;

}  // namespace trio
