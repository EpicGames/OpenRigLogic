// Copyright Epic Games, Inc. All Rights Reserved.

#include "dna/StreamReader.h"

#include "dna/stream/StreamReaderStatus.h"

#include <status/Provider.h>

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

namespace dna {

StreamReader::~StreamReader() = default;

const sc::StatusCode StreamReader::SignatureMismatchError{200, "DNA signature mismatched, expected %.3s, got %.3s"};
const sc::StatusCode StreamReader::VersionMismatchError{201, "DNA version mismatched, got %hu.%hu"};
const sc::StatusCode StreamReader::InvalidDataError{202, "Invalid data in DNA"};
const sc::StatusCode StreamReader::InvalidConfigError{
    203,
    "The coordinate system (x=%s, y=%s, z=%s) set in configuration is not valid."};
sc::StatusProvider StreamReaderStatus::status{StreamReader::SignatureMismatchError,
                                              StreamReader::VersionMismatchError,
                                              StreamReader::InvalidDataError,
                                              StreamReader::InvalidConfigError};

}  // namespace dna

#ifdef __clang__
    #pragma clang diagnostic pop
#endif
