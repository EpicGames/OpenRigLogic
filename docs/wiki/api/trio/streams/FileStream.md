# API Reference — `trio/streams/FileStream`

---

<!-- ink:api name="DefaultInstanceCreator" module="trio/streams/FileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<trio::FileStream>`

Template specialization that wires `FactoryCreate<trio::FileStream>` as the default construction policy for `trio::FileStream` in the `pma` allocator framework.

### Why this exists

The `pma` memory-management layer uses `DefaultInstanceCreator` and `DefaultInstanceDestroyer` traits to separate object lifecycle policy from call sites. Specializing this trait for `trio::FileStream` ensures that any `pma`-managed smart pointer wrapping a `FileStream` calls `FileStream::create()` rather than `new`, preserving the library's ownership contract and custom allocator support.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<trio::FileStream>` | The concrete creator type used by `pma` to construct `FileStream` instances |

### Relationships

- `DefaultInstanceDestroyer<trio::FileStream>` — paired destructor specialization; together these two traits fully cover the `FileStream` lifecycle
- `FileStream::create` — the factory method invoked by `FactoryCreate`

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="trio/streams/FileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<trio::FileStream>`

Template specialization that wires `FactoryDestroy<trio::FileStream>` as the default destruction policy for `trio::FileStream` in the `pma` allocator framework.

### Why this exists

Paired with `DefaultInstanceCreator<trio::FileStream>`, this specialization ensures that `pma`-managed smart pointers call `FileStream::destroy()` instead of `delete` when releasing a `FileStream` instance. This preserves the library's factory-based ownership contract and prevents mismatched allocation/deallocation.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<trio::FileStream>` | The concrete destroyer type used by `pma` to release `FileStream` instances |

### Relationships

- `DefaultInstanceCreator<trio::FileStream>` — paired creator specialization
- `FileStream::destroy` — the factory method invoked by `FactoryDestroy`

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="FileStream" module="trio/streams/FileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class FileStream : public BoundedIOStream`

Open a file for reading or writing through the `trio` `BoundedIOStream` interface. Use `create()` to obtain an instance and `destroy()` to release it when done.

### When to use this

Reach for `FileStream` when you need a file-backed stream that integrates with the `trio` abstraction layer — for example, to feed a DNA reader or writer with file data. Use this instead of a raw `FILE*` or `std::fstream` when the rest of your code expects a `BoundedIOStream`.

### Example

```cpp
// Open a DNA file for reading
FileStream* stream = FileStream::create(
    "/path/to/character.dna",
    AccessMode::Read,
    OpenMode::Binary
);
// ... use stream with a Reader ...
FileStream::destroy(stream);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `path` | `const char*` | required — UTF-8 encoded path to the file to open |
| `accessMode` | `AccessMode` | required — whether to open for reading or writing |
| `openMode` | `OpenMode` | required — whether to open in binary or textual mode |
| `memRes` | `MemoryResource*` | optional — custom memory allocator; uses the default mechanism when `nullptr` |

### Returns

`FileStream*` — raw owning pointer to the created instance. The caller is responsible for releasing it via `destroy()`.

### Watch out for

- Ownership is manual: the returned pointer from `create()` **must** be released by calling `destroy()`. Failing to do so leaks the stream. Use `pma::DefaultInstanceCreator`/`DefaultInstanceDestroyer` specializations to automate this with a `pma`-managed wrapper.
- Copy construction and copy assignment are deleted. Move semantics are supported.

<!-- ink:api-end name="FileStream" -->
