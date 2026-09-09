# API Reference — `trio`

_24 entries across 7 modules._

## By Task

### Core stream interfaces

Abstract interfaces defining the capabilities of streams: opening, closing, reading, writing, and seeking

| API | Module | Summary |
|-----|--------|---------|
| [Bounded](Concepts.md) | Concepts | Abstract interface for streams that have a known, finite size in bytes. |
| [Buffered](Concepts.md) | Concepts | Abstract interface for streams that buffer writes and need an explicit flush to guarantee data reaches the filesystem. |
| [Closeable](Concepts.md) | Concepts | Abstract interface for streams that must release an underlying resource when done. |
| [Controllable](Concepts.md) | Concepts | Combines `Openable` and `Closeable` into a single interface for streams whose lifecycle needs both an explicit open and close step. |
| [Openable](Concepts.md) | Concepts | Abstract interface for streams that must be explicitly opened before use. |
| [Readable](Concepts.md) | Concepts | Abstract interface for anything bytes can be read from — into a buffer or into another stream. |
| [Resizable](Concepts.md) | Concepts | Abstract interface for streams whose underlying storage can be resized to an exact byte length. |
| [Seekable](Concepts.md) | Concepts | Abstract interface for streams that support random access via a position cursor. |
| [Writable](Concepts.md) | Concepts | Abstract interface for anything bytes can be written to — a byte buffer or another stream. |

### Stream implementations

Concrete stream types for working with files, memory-mapped files, and in-memory buffers

| API | Module | Summary |
|-----|--------|---------|
| [BoundedIOStream](Stream.md) | Stream | Base interface for a fully-featured, finite-size I/O stream — one type that can be opened/closed, read, written, seeked, and sized. |
| [FileStream](streams/FileStream.md) | FileStream | Standard file stream implementation — read from or write to a file on disk through the `BoundedIOStream` interface. |
| [MemoryMappedFileStream](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Memory mapped file stream. Use this when you need direct memory access to file contents instead of buffered read/write calls. |
| [MemoryStream](streams/MemoryStream.md) | MemoryStream | In-memory stream. Use this when you need a `BoundedIOStream`-compatible buffer that isn't backed by a file, for example to stage serialized data before writing it out. |

### Stream configuration

Enumerations and parameters controlling how streams are opened and accessed

| API | Module | Summary |
|-----|--------|---------|
| [AccessMode](Stream.md) | Stream | Alias, scoped under `BoundedIOStream`, for `trio::AccessMode` — controls whether a stream is opened for reading or writing. |
| [AccessMode](types/Parameters.md) | Parameters | Enumerates how a stream is opened: for reading, writing, or both. |
| [OpenMode](Stream.md) | Stream | Alias, scoped under `BoundedIOStream`, for `trio::OpenMode` — controls whether a stream is opened in binary or textual mode. |
| [OpenMode](types/Parameters.md) | Parameters | Enumerates the byte-level mode a stream is opened in: binary or text. |

### Stream lifecycle management

RAII helpers and factory specializations for stream creation and destruction

| API | Module | Summary |
|-----|--------|---------|
| [DefaultInstanceCreator](streams/FileStream.md) | FileStream | Specialization telling `pma`'s generic factory machinery how to create a `trio::FileStream` instance by default. |
| [DefaultInstanceCreator](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Template specialization that binds `trio::MemoryMappedFileStream` to its factory-based creation function. |
| [DefaultInstanceCreator](streams/MemoryStream.md) | MemoryStream | Template specialization that binds `trio::MemoryStream` to its factory-based creation function. |
| [DefaultInstanceDestroyer](streams/FileStream.md) | FileStream | Template specialization that binds `trio::FileStream` to its factory-based destruction function. |
| [DefaultInstanceDestroyer](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Template specialization that binds `trio::MemoryMappedFileStream` to its factory-based destruction function. |
| [DefaultInstanceDestroyer](streams/MemoryStream.md) | MemoryStream | Template specialization that binds `trio::MemoryStream` to its factory-based destruction function. |
| [StreamScope](utils/StreamScope.md) | StreamScope | RAII wrapper that opens a `Controllable` stream on construction and closes it on destruction. |

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
