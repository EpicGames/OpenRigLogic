# API Reference — `trio/Stream`

---

<!-- ink:api name="AccessMode" module="trio/Stream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `AccessMode`

Scoped alias for `trio::AccessMode` that controls whether a stream is opened for reading, writing, or both.

### Why this exists

Importing `trio::AccessMode` directly into `BoundedIOStream` lets callers qualify the enum at the stream type (`BoundedIOStream::AccessMode::ReadWrite`) rather than reaching into the outer namespace. This keeps call sites readable and ensures the mode type stays coupled to the stream type that uses it.

### Relationships

- `BoundedIOStream` — passes `AccessMode` to its open operation to determine I/O permissions
- `OpenMode` — paired alias used in the same open call to control encoding or binary/text mode

<!-- ink:api-end name="AccessMode" -->

<!-- ink:api name="BoundedIOStream" module="trio/Stream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class BoundedIOStream : public Controllable, public Readable, public Writable, public Seekable, public Bounded`

Unified polymorphic stream handle that combines open/close control, read, write, seek, and size-bound queries into a single object. Use this when code needs to accept any bounded stream uniformly — passing `BoundedIOStream&` instead of separate reader and writer references.

### When to use this

Reach for `BoundedIOStream` when a function or component must perform both reads and writes on a stream whose total size is known or bounded — file streams and memory buffers are the primary cases. If you only need read access, prefer `Readable` directly to express the narrower constraint.

### Watch out for

- `AlreadyOpenError` and `OpenError` are separate status codes. Checking only `OpenError` after a failed open will miss the case where the stream was opened twice. Test `status == BoundedIOStream::AlreadyOpenError` explicitly if double-open is a plausible caller error.

### Example

```cpp
// Obtain a concrete BoundedIOStream from a factory (implementation-defined)
BoundedIOStream* stream = /* factory */;

sc::StatusCode status = stream->open(BoundedIOStream::AccessMode::ReadWrite,
                                      BoundedIOStream::OpenMode::Binary);
if (status == BoundedIOStream::OpenError) {
    // handle file-not-found or permission error
} else if (status == BoundedIOStream::AlreadyOpenError) {
    // stream was already open — close first
}

// Use Readable / Writable / Seekable interface methods normally
stream->close();
```

<!-- ink:api-end name="BoundedIOStream" -->

<!-- ink:api name="OpenMode" module="trio/Stream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `OpenMode`

Scoped alias for `trio::OpenMode` that specifies how a stream is opened — typically selecting binary versus text encoding.

### Why this exists

Mirrors the `AccessMode` alias pattern: importing `trio::OpenMode` directly into `BoundedIOStream` allows callers to use `BoundedIOStream::OpenMode::Binary` without navigating the outer namespace. Both aliases are passed together in any open call.

### Relationships

- `BoundedIOStream` — passes `OpenMode` alongside `AccessMode` when opening a stream
- `AccessMode` — paired alias; both are required arguments to the open operation

<!-- ink:api-end name="OpenMode" -->
