// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

namespace trio {

enum class AccessMode {
    Read = 1,
    Write = 2,
    ReadWrite = 3
};

enum class OpenMode {
    Binary = 4,
    // Deprecated: files are always opened in binary mode, so this behaves as Binary. Kept for source compatibility.
    Text = 8
};

}  // namespace trio
