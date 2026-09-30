// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "dna/stream/StreamReaderStatus.h"

namespace dna {

struct DNA;

struct Validator : private StreamReaderStatus {
    static bool validate(const DNA& dna);
};

}  // namespace dna
