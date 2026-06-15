# API Reference — `trio`

_28 entries across 6 modules._

## By Task

### Stream abstraction

Core stream interface for I/O operations.

| API | Module | Summary |
|-----|--------|---------|
| [Stream](Stream.md) | Stream | Abstract base class for byte-level I/O |
| [Concepts](Concepts.md) | Concepts | Stream concept definitions and checks |

### File streams

Stream implementations for file I/O.

| API | Module | Summary |
|-----|--------|---------|
| [FileStream](streams/FileStream.md) | FileStream | Byte stream backed by a file |
| [MemoryMappedFileStream](streams/MemoryMappedFileStream.md) | MemoryMappedFileStream | Memory-mapped file stream for fast access |

### Memory streams

Stream implementations for in-memory I/O.

| API | Module | Summary |
|-----|--------|---------|
| [MemoryStream](streams/MemoryStream.md) | MemoryStream | Byte stream backed by in-memory buffer |

### Stream utilities

Helper utilities for stream operations.

| API | Module | Summary |
|-----|--------|---------|
| [StreamScope](utils/StreamScope.md) | StreamScope | RAII guard for stream lifecycle |
| [Parameters](types/Parameters.md) | Parameters | Stream configuration and parameters |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| Concepts | [Concepts.md](Concepts.md) | 9 |
| FileStream | [streams/FileStream.md](streams/FileStream.md) | 3 |
| MemoryMappedFileStream | [streams/MemoryMappedFileStream.md](streams/MemoryMappedFileStream.md) | 4 |
| MemoryStream | [streams/MemoryStream.md](streams/MemoryStream.md) | 3 |
| Parameters | [types/Parameters.md](types/Parameters.md) | 2 |
| Stream | [Stream.md](Stream.md) | 3 |
| StreamScope | [utils/StreamScope.md](utils/StreamScope.md) | 1 |
