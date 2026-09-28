// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <arrayview/ArrayView.h>
#include <arrayview/StringView.h>
#include <pma/MemoryResource.h>
#include <pma/ScopedPtr.h>
#include <status/Status.h>
#include <status/StatusCode.h>
#include <tdm/TDM.h>
#include <trio/Stream.h>
#include <trio/streams/FileStream.h>
#include <trio/streams/MemoryMappedFileStream.h>
#include <trio/streams/MemoryStream.h>

namespace dna {

using sc::Status;
using trio::BoundedIOStream;
using trio::FileStream;
using trio::MemoryMappedFileStream;
using trio::MemoryStream;

using av::ArrayView;
using av::ConstArrayView;
using av::StringView;

using pma::DefaultInstanceCreator;
using pma::DefaultInstanceDestroyer;
using pma::Delete;
using pma::FactoryCreate;
using pma::FactoryDestroy;
using pma::makeScoped;
using pma::MemoryResource;
using pma::New;
using pma::ScopedPtr;

}  // namespace dna
