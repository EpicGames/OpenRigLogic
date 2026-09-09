# API Reference — `trio/Stream`

---

<!-- ink:api name="AccessMode" module="trio/Stream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `AccessMode`

Alias, scoped under `BoundedIOStream`, for `trio::AccessMode` — controls whether a stream is opened for reading or writing.

### Why this exists

`BoundedIOStream::AccessMode` re-exports `trio::AccessMode` so code working with `BoundedIOStream` (and its subclasses like `FileStream`) can refer to the access-mode type through the stream type itself, without an extra `trio::` qualification.

### Relationships

- `trio::AccessMode` — *the underlying type this alias re-exports*
- `FileStream::create` — *consumer that takes an `AccessMode` argument*

<!-- ink:api-end name="AccessMode" -->

<!-- ink:api name="BoundedIOStream" module="trio/Stream" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class TRIOAPI BoundedIOStream : public Controllable, public Readable, public Writable, public Seekable, public Bounded`

Base interface for a fully-featured, finite-size I/O stream — one type that can be opened/closed, read, written, seeked, and sized.

### When to use this

Depend on `BoundedIOStream` when code needs the full set of stream capabilities (control, read, write, seek, size) rather than composing the individual `trio::Concepts` interfaces by hand. `FileStream` is the concrete implementation used for on-disk files.

### Method groups

| Group | Methods |
|-------|---------|
| lifecycle | (inherited from `Controllable`: open, close) |
| io | (inherited from `Readable`/`Writable`: read, write) |
| position | (inherited from `Seekable`: tell, seek) |
| size | (inherited from `Bounded`: size) |

### Example

```cpp
trio::BoundedIOStream* stream = /* obtained from a concrete implementation */;
stream->open();
std::uint64_t total = stream->size();
stream->close();
```

### Constraints

- Exposes static `sc::StatusCode` members (`OpenError`, `ReadError`, `WriteError`, `AlreadyOpenError`, `SeekError`) for reporting stream-level failures.

### Watch out for

- `AlreadyOpenError` and `OpenError` are separate status codes. Checking only `OpenError` after a failed open will miss the case where the stream was opened twice. Test `status == BoundedIOStream::AlreadyOpenError` explicitly if double-open is a plausible caller error.

<!-- ink:api-end name="BoundedIOStream" -->

<!-- ink:api name="OpenMode" module="trio/Stream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `OpenMode`

Alias, scoped under `BoundedIOStream`, for `trio::OpenMode` — controls whether a stream is opened in binary or textual mode.

### Why this exists

`BoundedIOStream::OpenMode` re-exports `trio::OpenMode` for the same reason as `AccessMode`: so it can be referenced via the stream type itself.

### Relationships

- `trio::OpenMode` — *the underlying type this alias re-exports*
- `FileStream::create` — *consumer that takes an `OpenMode` argument*

<!-- ink:api-end name="OpenMode" -->
