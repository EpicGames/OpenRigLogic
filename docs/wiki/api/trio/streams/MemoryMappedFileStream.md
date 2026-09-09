# API Reference — `trio/streams/MemoryMappedFileStream`

---

<!-- ink:api name="DefaultInstanceCreator" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceCreator<trio::MemoryMappedFileStream>`

Template specialization that binds `trio::MemoryMappedFileStream` to its factory-based creation function.

### Why this exists

This specialization lets generic `pma` code create a `MemoryMappedFileStream` without knowing its concrete factory function, by routing through `FactoryCreate<trio::MemoryMappedFileStream>`. It pairs with `DefaultInstanceDestroyer` to keep creation and destruction consistent for this type.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<trio::MemoryMappedFileStream>` | required. The callable type used to create a `MemoryMappedFileStream` instance. |

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<trio::MemoryMappedFileStream>`

Template specialization that binds `trio::MemoryMappedFileStream` to its factory-based destruction function.

### Why this exists

Mirrors `DefaultInstanceCreator<trio::MemoryMappedFileStream>` on the destruction side, routing through `FactoryDestroy<trio::MemoryMappedFileStream>` so generic `pma` code can free instances without a hardcoded destructor call.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<trio::MemoryMappedFileStream>` | required. The callable type used to destroy a `MemoryMappedFileStream` instance created via the factory. |

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="MemoryMappedFileStream" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class MemoryMappedFileStream : public BoundedIOStream, public Buffered, public Resizable`

Memory mapped file stream. Use this when you need direct memory access to file contents instead of buffered read/write calls.

### When to use this

Reach for `MemoryMappedFileStream` when you want the OS to map file contents directly into memory, avoiding explicit read/write buffering overhead. Use the regular `FileStream` instead when you only need straightforward sequential I/O and don't need memory-mapped access.

### Example

```cpp
trio::MemoryMappedFileStream* stream =
    trio::MemoryMappedFileStream::create("character.dna", trio::AccessMode::Read);
// ... use stream via BoundedIOStream / Buffered / Resizable interfaces ...
trio::MemoryMappedFileStream::destroy(stream);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `path` | `const char*` | required. UTF-8 encoded path to the file to be opened. |
| `accessMode` | `AccessMode` | required. Controls whether the file is opened for reading or writing. |
| `memRes` | `MemoryResource*` | optional. Memory resource used for allocating the instance; a default allocation mechanism is used if not given. |

### Returns

`MemoryMappedFileStream*` — pointer to the newly created stream instance.

### Watch out for

- The user is responsible for releasing the returned pointer by calling `destroy` — there is no implicit cleanup.
- Copy construction and copy assignment are deleted; only move construction/assignment are supported.
- The returned pointer from `create` is **not** RAII-managed. You must call `destroy` explicitly to release it; failing to do so is a memory leak. Pair every `create` with a `destroy`, or use `pma::ScopedPtr` if available.
- Copy construction and copy assignment are deleted. Move construction and move assignment are available, but after a move the source instance is no longer valid — do not call `destroy` on it.

<!-- ink:api-end name="MemoryMappedFileStream" -->
