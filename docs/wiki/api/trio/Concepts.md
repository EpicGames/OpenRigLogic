# API Reference — `trio/Concepts`

---

<!-- ink:api name="Bounded" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Bounded`

Abstract interface for streams that have a known, finite size in bytes.

### When to use this

Implement `Bounded` when a stream's total size can be queried up front, such as a file — useful for pre-allocating buffers or validating read ranges before issuing reads.

### Example

```cpp
std::uint64_t totalBytes = stream->size();
```

### Returns

`std::uint64_t` — size of the stream in bytes.

<!-- ink:api-end name="Bounded" -->

<!-- ink:api name="Buffered" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Buffered`

Abstract interface for streams that buffer writes and need an explicit flush to guarantee data reaches the filesystem.

### When to use this

Implement `Buffered` when writes to the stream may be held in memory before being committed, so callers can force a flush at points where durability matters (e.g., before closing or after a critical write).

### Example

```cpp
stream->write(data, size);
stream->flush();
// data is now guaranteed to be committed to disk
```

### Watch out for

- `flush()` is distinct from `close()`. Closing a stream does not guarantee that buffered data is flushed unless the implementation explicitly does so. Call `flush()` before `close()` when durability is required.

<!-- ink:api-end name="Buffered" -->

<!-- ink:api name="Closeable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Closeable`

Abstract interface for streams that must release an underlying resource when done.

### When to use this

Implement `Closeable` alongside `Openable` when a stream wraps a resource that needs explicit release, such as a file handle.

### Example

```cpp
stream->close();
// underlying resource is released
```

<!-- ink:api-end name="Closeable" -->

<!-- ink:api name="Controllable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Controllable : public Openable, public Closeable`

Combines `Openable` and `Closeable` into a single interface for streams whose lifecycle needs both an explicit open and close step.

### When to use this

Use `Controllable` as the base for any stream type where you want to require both open and close support at once, instead of implementing `Openable` and `Closeable` separately and hoping callers use both.

### Example

```cpp
trio::Controllable* stream = /* ... */;
stream->open();
// use the stream
stream->close();
```

<!-- ink:api-end name="Controllable" -->

<!-- ink:api name="Openable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Openable`

Abstract interface for streams that must be explicitly opened before use.

### When to use this

Implement `Openable` when a stream wraps a resource (e.g., a file handle) that needs an explicit open step before reads or writes are valid.

### Example

```cpp
stream->open();
// stream is now ready for read/write calls
```

<!-- ink:api-end name="Openable" -->

<!-- ink:api name="Readable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class TRIOAPI Readable`

Abstract interface for anything bytes can be read from — into a buffer or into another stream.

### When to use this

Implement `Readable` on any stream-like type that needs to produce byte input, so calling code that only reads data can depend on this interface rather than a concrete stream type. Use the buffer-read overload to pull bytes into memory, and the stream-read overload to pipe bytes directly into a `Writable` destination.

### Method groups

| Group | Methods |
|-------|---------|
| read | read(char* destination, std::size_t size), read(Writable* destination, std::size_t size) |

### Example

```cpp
char buffer[4096];
std::size_t bytesRead = stream->read(buffer, sizeof(buffer));
// process buffer[0..bytesRead)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `destination` | `char*` / `Writable*` | required. Where read bytes are placed, either a raw buffer or another stream. |
| `size` | `std::size_t` | required. Number of bytes to read. |

### Returns

`std::size_t` — the number of bytes actually read.

### Watch out for

- The destructor is `protected`, so instances must be destroyed through the owning concrete type, not deleted directly through a `Readable*`.

<!-- ink:api-end name="Readable" -->

<!-- ink:api name="Resizable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Resizable`

Abstract interface for streams whose underlying storage can be resized to an exact byte length.

### When to use this

Implement `Resizable` when a stream needs to grow or truncate its backing storage to a specific size — for example, pre-allocating a file to its final size, or truncating it after an overwrite.

### Example

```cpp
stream->resize(1024u);
// underlying storage is now exactly 1024 bytes
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `size` | `std::uint64_t` | required. Requested size of the underlying storage, in bytes. |

<!-- ink:api-end name="Resizable" -->

<!-- ink:api name="Seekable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI Seekable`

Abstract interface for streams that support random access via a position cursor.

### When to use this

Implement `Seekable` when a stream needs to support jumping to arbitrary offsets (e.g., a file), rather than only sequential reads/writes.

### Example

```cpp
std::uint64_t pos = stream->tell();
stream->seek(0u);
// stream cursor is now at the start
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `position` | `std::uint64_t` | required. Offset relative to the start of the stream (0 = start). |

### Returns

`std::uint64_t` — (from `tell()`) the current position relative to the stream's start.

<!-- ink:api-end name="Seekable" -->

<!-- ink:api name="Writable" module="trio/Concepts" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class TRIOAPI Writable`

Abstract interface for anything bytes can be written to — a byte buffer or another stream.

### When to use this

Implement `Writable` on any stream-like type that needs to accept byte output (e.g., a file or memory stream), so calling code that only writes data can depend on this interface rather than a concrete stream type. Use the buffer-write overload for raw byte data, and the stream-write overload when writing bytes to come from another `Readable` stream.

### Method groups

| Group | Methods |
|-------|---------|
| write | write(const char* source, std::size_t size), write(Readable* source, std::size_t size) |

### Example

```cpp
void saveHeader(trio::Writable* stream, const char* header, std::size_t size) {
    stream->write(header, size);
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `source` | `const char*` / `Readable*` | required. Data to write, either a raw buffer or another stream to read from. |
| `size` | `std::size_t` | required. Number of bytes to write. |

### Returns

`std::size_t` — the number of bytes actually written.

### Watch out for

- The destructor is `protected`, so instances must be destroyed through the owning concrete type, not deleted directly through a `Writable*`.

<!-- ink:api-end name="Writable" -->
