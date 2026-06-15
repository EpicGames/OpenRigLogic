# API Reference — `trio/utils/StreamScope`

---

<!-- ink:api name="StreamScope" module="trio/utils/StreamScope" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `StreamScope(Controllable* stream)`

RAII guard that automatically opens a `Controllable` stream on construction and closes it when the scope exits.

### When to use this

Use this whenever you need to tie the open/close lifecycle of a `Controllable` stream to a C++ scope — especially in functions with multiple return paths or exception-prone code, where manually pairing `open()` and `close()` calls would be error-prone. Prefer `StreamScope` over manual `open()`/`close()` pairs for the same reason you prefer `std::unique_ptr` over raw `new`/`delete`.

### Example

```cpp
// Open a stream for the duration of a processing block
auto reader = BinaryStreamReader::create("rig.dna", ...);
StreamScope scope{reader.get()};  // calls reader->open()

// reader is open here; use it freely
processRig(*reader);

// scope destructor calls reader->close() automatically
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream_` | `Controllable*` | required — pointer to the stream to manage; `open()` is called immediately on construction |

### Constraints

- Non-copyable: copy constructor and copy assignment are `= delete`. Use `std::move` to transfer ownership.
- Move-safe: the moved-from `StreamScope` holds `nullptr` and will not call `close()` on destruction.
- `noexcept`: the destructor and move operations are marked `noexcept`; no exceptions escape from lifetime management.

### Watch out for

- The constructor calls `stream->open()` immediately and unconditionally. Do not pass a stream that is already open unless `open()` is idempotent for that `Controllable` implementation.
- After a move, the source `StreamScope` no longer owns the stream — its destructor is a no-op. Only the destination scope will call `close()`.

<!-- ink:api-end name="StreamScope" -->
