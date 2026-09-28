// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "trio/streams/FileStream.h"
#include "trio/streams/StreamStatus.h"
#include "trio/types/Aliases.h"
#include "trio/utils/NativeString.h"

#include <pma/TypeDefs.h>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4365 4987)
#endif
#include <cstddef>
#include <cstdint>
#include <cstdio>
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

namespace trio {

class FileStreamImpl : public FileStream {
public:
    FileStreamImpl(const char* path_, AccessMode accessMode_, OpenMode openMode_, MemoryResource* memRes_);
    ~FileStreamImpl() override;

    FileStreamImpl(const FileStreamImpl&) = delete;
    FileStreamImpl& operator=(const FileStreamImpl&) = delete;
    FileStreamImpl(FileStreamImpl&&) = delete;
    FileStreamImpl& operator=(FileStreamImpl&&) = delete;

    void open() override;
    void close() override;
    void flush() override;
    std::uint64_t tell() override;
    void seek(std::uint64_t position) override;
    std::uint64_t size() override;
    std::size_t read(char* destination, std::size_t size) override;
    std::size_t read(Writable* destination, std::size_t size) override;
    std::size_t write(const char* source, std::size_t size) override;
    std::size_t write(Readable* source, std::size_t size) override;

    MemoryResource* getMemoryResource();

private:
    bool reposition(AccessMode direction);

private:
    std::FILE* file;
    NativeString filePath;
    AccessMode fileAccessMode;
    std::uint64_t fileSize;
    std::uint64_t filePos;
    // Directions the C stream may take next without a positioning call in between
    AccessMode positionedFor;
    MemoryResource* memRes;
    StreamStatus status;
};

}  // namespace trio
