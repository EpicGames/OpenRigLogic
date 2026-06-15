# API Reference — `trio/streams/MemoryMappedFileStream`

---

<!-- ink:api name="DefaultInstanceCreator" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::DefaultInstanceCreator<trio::MemoryMappedFileStream>`

Policy specialization that registers `FactoryCreate` as the allocation strategy for `MemoryMappedFileStream` within the `pma` memory management system.

### Why this exists

The `pma` system uses trait structs to decouple object creation from object use — callers that need a `MemoryMappedFileStream` via `pma` machinery ask `DefaultInstanceCreator` which factory to use, rather than calling `new` directly. This specialization answers that query by pointing to `FactoryCreate<trio::MemoryMappedFileStream>`, which delegates to the class's own `create` static method. The result is that `pma`-managed smart pointer types work correctly with `MemoryMappedFileStream` without requiring manual factory wiring at each call site.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<trio::MemoryMappedFileStream>` | The factory policy type selected by `pma` when constructing a `MemoryMappedFileStream` via this trait. |

### Relationships

- `MemoryMappedFileStream` — the class this policy creates; exposes `create` as its factory entry point
- `DefaultInstanceDestroyer<trio::MemoryMappedFileStream>` — the symmetric destruction policy

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::DefaultInstanceDestroyer<trio::MemoryMappedFileStream>`

Policy specialization that registers `FactoryDestroy` as the deallocation strategy for `MemoryMappedFileStream` within the `pma` memory management system.

### Why this exists

Symmetric to `DefaultInstanceCreator`, this trait answers the `pma` system's query of "how do I destroy a `MemoryMappedFileStream`?" by pointing to `FactoryDestroy<trio::MemoryMappedFileStream>`, which delegates to the class's own `destroy` static method. This ensures that `pma`-managed smart pointers call the correct cleanup path — `MemoryMappedFileStream::destroy` — rather than invoking `delete` directly, which would bypass any internal teardown logic.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<trio::MemoryMappedFileStream>` | The destruction policy type selected by `pma` when releasing a `MemoryMappedFileStream` via this trait. |

### Relationships

- `MemoryMappedFileStream` — the class this policy destroys; exposes `destroy` as its cleanup entry point
- `DefaultInstanceCreator<trio::MemoryMappedFileStream>` — the symmetric creation policy

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="MemoryMappedFileStream" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `trio::MemoryMappedFileStream`

Map a file directly into memory for high-performance I/O without per-call syscall overhead.

### When to use this

Use this when you need fast, random-access reads or writes over large files where the cost of individual `read()`/`write()` syscalls becomes a bottleneck. Prefer this over a standard `FileStream` when you need the OS to page file contents on demand rather than staging through an explicit buffer. Not suitable for network streams or pipes — only file-backed storage.

### Example

```cpp
// Open a DNA file for reading via memory-mapped I/O
auto* stream = trio::MemoryMappedFileStream::create(
    "/data/characters/hero.dna",
    trio::AccessMode::Read
);
// ... use stream with a reader ...
trio::MemoryMappedFileStream::destroy(stream);
```

### Parameters — `create`

| Name | Type | Description |
|------|------|-------------|
| `path` | `const char*` | required — UTF-8 encoded path to the file to open. |
| `accessMode` | `AccessMode` | required — Controls whether the file is opened for reading or writing. |
| `memRes` | `MemoryResource*` | optional — Custom allocator for the instance. Uses the default allocator when `nullptr`. |

### Returns — `create`

`MemoryMappedFileStream*` — Owning pointer to the newly created instance. Caller is responsible for releasing it via `destroy`.

### Watch out for

- The returned pointer from `create` is **not** RAII-managed. You must call `destroy` explicitly to release it; failing to do so is a memory leak. Pair every `create` with a `destroy`, or use `pma::ScopedPtr` if available.
- Copy construction and copy assignment are deleted. Move construction and move assignment are available, but after a move the source instance is no longer valid — do not call `destroy` on it.

<!-- ink:api-end name="MemoryMappedFileStream" -->

<!-- ink:api name="type" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type` (pma traits alias)

The resolved factory or destroyer policy type carried by `DefaultInstanceCreator` and `DefaultInstanceDestroyer` trait specializations for `MemoryMappedFileStream`.

### Why this exists

In the `pma` traits system, the member alias `using type = ...` is the conventional way to expose the selected policy to generic code (e.g., `typename DefaultInstanceCreator<T>::type`). For `MemoryMappedFileStream`, this resolves to `FactoryCreate<trio::MemoryMappedFileStream>` (in the creator) or `FactoryDestroy<trio::MemoryMappedFileStream>` (in the destroyer). Client code that accesses `::type` receives the concrete factory or destructor functor without needing to name it explicitly.

### Relationships

- `DefaultInstanceCreator<trio::MemoryMappedFileStream>` — the creator trait that holds `type = FactoryCreate<...>`
- `DefaultInstanceDestroyer<trio::MemoryMappedFileStream>` — the destroyer trait that holds `type = FactoryDestroy<...>`

<!-- ink:api-end name="type" -->
