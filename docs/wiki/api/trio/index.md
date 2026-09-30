# API Reference — `trio`

_24 entries across 7 modules._

## By Task

### Stream implementations

Concrete stream types for files, memory, and memory-mapped files with full I/O capabilities

| API | Module | Summary |
|-----|--------|---------|
| [BoundedIOStream](Stream.md) | Stream | Base interface for a fully-featured, finite-size I/O stream — one type that can be opened/closed, read, written, seeked, and sized. |
| [FileStream](streams/FileStream.md) | FileStream | Standard file stream implementation — read from or write to a file on disk through the `BoundedIOStream` interface. |
| [MemoryMappedFileStream](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Memory mapped file stream. Use this when you need direct memory access to file contents instead of buffered read/write calls. |
| [MemoryStream](streams/MemoryStream.md) | MemoryStream | In-memory stream. Use this when you need a `BoundedIOStream`-compatible buffer that isn't backed by a file, for example to stage serialized data before writing it out. |

### Stream capability interfaces

Abstract interfaces defining individual read, write, seek, and size operations on streams

| API | Module | Summary |
|-----|--------|---------|
| [Bounded](Concepts.md) | Concepts | Abstract interface for streams that have a known, finite size in bytes. |
| [Readable](Concepts.md) | Concepts | Abstract interface for anything bytes can be read from — into a buffer or into another stream. |
| [Resizable](Concepts.md) | Concepts | Abstract interface for streams whose underlying storage can be resized to an exact byte length. |
| [Seekable](Concepts.md) | Concepts | Abstract interface for streams that support random access via a position cursor. |
| [Writable](Concepts.md) | Concepts | Abstract interface for anything bytes can be written to — a byte buffer or another stream. |

### Stream lifecycle and control

Interfaces and utilities for opening, closing, and managing stream resource lifecycles

| API | Module | Summary |
|-----|--------|---------|
| [Closeable](Concepts.md) | Concepts | Abstract interface for streams that must release an underlying resource when done. |
| [Controllable](Concepts.md) | Concepts | Combines `Openable` and `Closeable` into a single interface for streams whose lifecycle needs both an explicit open and close step. |
| [Openable](Concepts.md) | Concepts | Abstract interface for streams that must be explicitly opened before use. |
| [StreamScope](utils/StreamScope.md) | StreamScope | RAII wrapper that opens a `Controllable` stream on construction and closes it on destruction. |

### Stream configuration

Parameters and enumerations controlling how streams are opened and the data format they use

| API | Module | Summary |
|-----|--------|---------|
| [AccessMode](Stream.md) | Stream | Alias, scoped under `BoundedIOStream`, for `trio::AccessMode` — controls whether a stream is opened for reading or writing. |
| [AccessMode](types/Parameters.md) | Parameters | Enumerates how a stream is opened: for reading, writing, or both. |
| [OpenMode](Stream.md) | Stream | Alias, scoped under `BoundedIOStream`, for `trio::OpenMode` — controls whether a stream is opened in binary or textual mode. |
| [OpenMode](types/Parameters.md) | Parameters | Enumerates the byte-level mode a stream is opened in: binary or text. |

### Buffering and factory integration

Buffered stream behavior and factory-based creation and destruction for stream instances

| API | Module | Summary |
|-----|--------|---------|
| [Buffered](Concepts.md) | Concepts | Abstract interface for streams that buffer writes and need an explicit flush to guarantee data reaches the filesystem. |
| [DefaultInstanceCreator](streams/FileStream.md) | FileStream | Specialization telling `pma`'s generic factory machinery how to create a `trio::FileStream` instance by default. |
| [DefaultInstanceCreator](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Template specialization that binds `trio::MemoryMappedFileStream` to its factory-based creation function. |
| [DefaultInstanceCreator](streams/MemoryStream.md) | MemoryStream | Template specialization that binds `trio::MemoryStream` to its factory-based creation function. |
| [DefaultInstanceDestroyer](streams/FileStream.md) | FileStream | Template specialization that binds `trio::FileStream` to its factory-based destruction function. |
| [DefaultInstanceDestroyer](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Template specialization that binds `trio::MemoryMappedFileStream` to its factory-based destruction function. |
| [DefaultInstanceDestroyer](streams/MemoryStream.md) | MemoryStream | Template specialization that binds `trio::MemoryStream` to its factory-based destruction function. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| Concepts | [Concepts.md](Concepts.md) | 9 |
| Stream | [Stream.md](Stream.md) | 3 |
| FileStream | [streams/FileStream.md](streams/FileStream.md) | 3 |
| MemoryMappedFileStream | [streams/MemoryMappedFileStream.md](streams/MemoryMappedFileStream.md) | 3 |
| MemoryStream | [streams/MemoryStream.md](streams/MemoryStream.md) | 3 |
| Parameters | [types/Parameters.md](types/Parameters.md) | 2 |
| StreamScope | [utils/StreamScope.md](utils/StreamScope.md) | 1 |
