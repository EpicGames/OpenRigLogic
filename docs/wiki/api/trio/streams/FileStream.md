# API Reference — `trio/streams/FileStream`

---

<!-- ink:api name="DefaultInstanceCreator" module="trio/streams/FileStream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceCreator<trio::FileStream>`

Specialization telling `pma`'s generic factory machinery how to create a `trio::FileStream` instance by default.

### Why this exists

Generic `pma` factory code needs a uniform way to construct arbitrary types without each call site knowing the specific creation mechanism. This specialization binds `trio::FileStream` to `FactoryCreate<trio::FileStream>`, so factory-based code can create `FileStream` instances without special-casing it.

### Relationships

- `trio::FileStream` — *the type this specialization creates*
- `DefaultInstanceDestroyer<trio::FileStream>` — *paired specialization for destruction*
- `FactoryCreate<trio::FileStream>` — *the creation strategy this binds to*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="trio/streams/FileStream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<trio::FileStream>`

Template specialization that binds `trio::FileStream` to its factory-based destruction function.

### Why this exists

`pma`'s generic memory management relies on `DefaultInstanceDestroyer<T>` specializations to know how to free a `T*` without the caller hardcoding a destructor call. This specialization routes destruction of `FileStream` instances through `FactoryDestroy<trio::FileStream>`, keeping allocation and deallocation symmetric with the paired `DefaultInstanceCreator` specialization.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<trio::FileStream>` | required. The callable type used to destroy a `FileStream` instance created via the factory. |

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="FileStream" module="trio/streams/FileStream" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TRIOAPI FileStream : public BoundedIOStream`

Standard file stream implementation — read from or write to a file on disk through the `BoundedIOStream` interface.

### When to use this

Use `FileStream::create` whenever you need to open a real on-disk file for reading or writing through the `trio` stream interfaces, rather than implementing `BoundedIOStream` yourself. Use a custom `memRes` only when you need control over the allocator; otherwise the default allocation mechanism is used.

### Example

```cpp
trio::FileStream* stream = trio::FileStream::create(
    "character.dna",
    trio::AccessMode::Read,
    trio::OpenMode::Binary);

std::uint64_t size = stream->size();
// ... read data via stream->read(...) ...

trio::FileStream::destroy(stream);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `path` | `const char*` | required. UTF-8 encoded path to the file to open. |
| `accessMode` | `AccessMode` | required. Whether the file is opened for reading or writing. |
| `openMode` | `OpenMode` | required. Whether the file is opened in binary or textual mode. |
| `memRes` | `MemoryResource*` | optional. Memory resource used to allocate the instance; defaults to the standard allocation mechanism when omitted. |
| `instance` | `FileStream*` | required (for `destroy`). The instance to free. |

### Returns

`FileStream*` — a newly created `FileStream` instance, or the interface used to free one via `destroy`.

### Watch out for

- The caller owns the pointer returned by `create` and is responsible for releasing it via `destroy` — there is no automatic cleanup.
- Copy construction and copy assignment are deleted; only move construction/assignment are available.
- Ownership is manual: the returned pointer from `create()` **must** be released by calling `destroy()`. Failing to do so leaks the stream. Use `pma::DefaultInstanceCreator`/`DefaultInstanceDestroyer` specializations to automate this with a `pma`-managed wrapper.

<!-- ink:api-end name="FileStream" -->
