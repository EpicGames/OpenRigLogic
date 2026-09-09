# API Reference — `trio/types/Parameters`

---

<!-- ink:api name="AccessMode" module="trio/types/Parameters" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `AccessMode`

Enumerates how a stream is opened: for reading, writing, or both.

### Why this exists

`AccessMode` gives stream constructors (like `FileStream::create` and `MemoryMappedFileStream::create`) a single, explicit way to express read/write intent instead of separate boolean flags. `ReadWrite` is defined as the bitwise combination of `Read` and `Write`, so code can test capability with a bitmask rather than an equality check.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Read` | `enum value = 1` | Open the stream for reading only. |
| `Write` | `enum value = 2` | Open the stream for writing only. |
| `ReadWrite` | `enum value = 3` | Open the stream for both reading and writing (`Read \| Write`). |

<!-- ink:api-end name="AccessMode" -->

<!-- ink:api name="OpenMode" module="trio/types/Parameters" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `OpenMode`

Enumerates the byte-level mode a stream is opened in: binary or text.

### Why this exists

`OpenMode` separates the notion of *how bytes are interpreted* from `AccessMode`'s *what operations are allowed*, so a stream can be opened as, for example, binary + read/write without conflating the two concerns into one enum.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Binary` | `enum value = 4` | Open the stream in binary mode, with no text translation. |
| `Text` | `enum value = 8` | Open the stream in text mode. |

<!-- ink:api-end name="OpenMode" -->
