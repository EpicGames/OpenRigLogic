# API Reference — `trio/utils/StreamScope`

---

<!-- ink:api name="StreamScope" module="trio/utils/StreamScope" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class StreamScope`

RAII wrapper that opens a `Controllable` stream on construction and closes it on destruction.

### When to use this

Use `StreamScope` when you want a stream opened and closed automatically around a block of code, so an early return or exception can't leave the stream open. Prefer this over manually calling `open()`/`close()` on a `Controllable*` whenever the open/close pair should be tied to a scope's lifetime.

### Example

```cpp
trio::MemoryMappedFileStream* stream =
    trio::MemoryMappedFileStream::create("character.dna", trio::AccessMode::Read);
{
    trio::StreamScope scope{stream};
    // stream is open here; scope->close() runs automatically at end of block
}
trio::MemoryMappedFileStream::destroy(stream);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream_` | `Controllable*` | required. The stream to open immediately and close when the `StreamScope` is destroyed. |

### Watch out for

- Copy construction and copy assignment are deleted; only move construction/assignment are supported, and moving-from leaves the source's stream pointer null so it won't be closed twice.
- The destructor only calls `close()` if the stored stream pointer is non-null — a moved-from `StreamScope` is safe to destroy without side effects.
- The constructor calls `stream->open()` immediately and unconditionally. Do not pass a stream that is already open unless `open()` is idempotent for that `Controllable` implementation.
- After a move, the source `StreamScope` no longer owns the stream — its destructor is a no-op. Only the destination scope will call `close()`.

### Constraints

- Non-copyable: copy constructor and copy assignment are `= delete`. Use `std::move` to transfer ownership.
- `noexcept`: the destructor and move operations are marked `noexcept`; no exceptions escape from lifetime management.

<!-- ink:api-end name="StreamScope" -->
