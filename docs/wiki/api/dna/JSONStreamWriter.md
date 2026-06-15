# API Reference — `dna/JSONStreamWriter`

---

<!-- ink:api name="BinaryStreamReader" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class BinaryStreamReader`

Forward-declared peer type used as a data source when copying binary DNA data into a `JSONStreamWriter` via `setFrom`.

### When to use this

Pass a `BinaryStreamReader*` to `JSONStreamWriter::setFrom` when you need to convert an existing binary DNA stream into JSON output. `BinaryStreamReader` is defined in its own header — this forward declaration exists solely to enable the `setFrom` overload signature in `JSONStreamWriter.h`.

### Relationships

- `JSONStreamWriter::setFrom(const BinaryStreamReader*)` — consumes this type as the copy source
- `JSONStreamReader` — the JSON-format peer; use when copying from an existing JSON source instead

<!-- ink:api-end name="BinaryStreamReader" -->

<!-- ink:api name="DefaultInstanceCreator" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<dna::JSONStreamWriter>`

PMA policy struct that wires `pma::FactoryCreate` as the default construction strategy for `dna::JSONStreamWriter`.

### Why this exists

The `pma` memory management layer uses a traits pattern to decouple object construction from the calling code. By specializing `DefaultInstanceCreator` for `JSONStreamWriter`, PMA utilities such as `ScopedPtr` automatically know to call `JSONStreamWriter::create(...)` rather than `new`. This keeps smart-pointer and container code generic without hard-coding factory calls at each use site.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::JSONStreamWriter>` | required — the creator strategy type consumed by PMA utilities |

### Relationships

- `DefaultInstanceDestroyer<dna::JSONStreamWriter>` — the paired destruction policy; both must be set when using PMA smart pointers
- `pma::FactoryCreate` — the underlying factory functor this trait aliases
- `JSONStreamWriter::create` — the factory method invoked by `FactoryCreate`

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<dna::JSONStreamWriter>`

PMA policy struct that wires `pma::FactoryDestroy` as the default destruction strategy for `dna::JSONStreamWriter`.

### Why this exists

Mirrors `DefaultInstanceCreator` on the destruction side. PMA smart pointers and container utilities look up `DefaultInstanceDestroyer<T>::type` to find the functor to call when releasing a `T`. Specializing this for `JSONStreamWriter` ensures that `ScopedPtr<JSONStreamWriter>` automatically calls `JSONStreamWriter::destroy(instance)` on scope exit, preventing leaks without hard-coding the factory call at each use site.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::JSONStreamWriter>` | required — the destroyer strategy type consumed by PMA utilities |

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamWriter>` — the paired construction policy
- `pma::FactoryDestroy` — the underlying factory destructor this trait aliases
- `JSONStreamWriter::destroy` — the factory method invoked by `FactoryDestroy`

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="JSONStreamReader" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class JSONStreamReader`

Forward-declared peer type used as a data source when copying JSON DNA data into a `JSONStreamWriter` via `setFrom`.

### When to use this

Pass a `JSONStreamReader*` to `JSONStreamWriter::setFrom` when you need to re-serialize or transform an existing JSON DNA stream — for example, to filter to a specific `DataLayer` before writing. Use `BinaryStreamReader` instead when your source data is in binary format.

### Relationships

- `JSONStreamWriter::setFrom(const JSONStreamReader*)` — consumes this type as the copy source
- `BinaryStreamReader` — the binary-format peer; use when converting from binary DNA to JSON

<!-- ink:api-end name="JSONStreamReader" -->

<!-- ink:api name="JSONStreamWriter" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI JSONStreamWriter : public StreamWriter`

Write DNA rig data to a stream in human-readable JSON format, with configurable indentation. Use this instead of `BinaryStreamWriter` when you need human-readable output for debugging, tooling interchange, or text-based diffing.

### When to use this

Use `JSONStreamWriter` when you need to serialize DNA rig data to JSON — for example, to produce text output that can be inspected in an editor, diffed in version control, or consumed by JSON-based tooling. When output size and parse speed matter more than readability, use `BinaryStreamWriter` instead.

### Example

```cpp
// Create a stream to write into
auto* stream = SomeBoundedIOStream::create();

// Create a JSONStreamWriter with 2-space indentation
auto* writer = dna::JSONStreamWriter::create(stream, 2u);

// Populate from an existing binary reader
writer->setFrom(binaryReader, dna::DataLayer::All,
                dna::UnknownLayerPolicy::Preserve);

// Write and release
writer->write();
dna::JSONStreamWriter::destroy(writer);
```

### Parameters (create)

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — target stream to write JSON output into |
| `indentWidth` | `std::uint32_t` | optional — number of spaces per indentation level; defaults to `4` |
| `memRes` | `MemoryResource*` | optional — custom memory resource for allocations; defaults to the platform default allocator |

### Returns

`JSONStreamWriter*` — caller-owned pointer. Must be released by calling `destroy(instance)`.

### Watch out for

- You own the returned pointer. Forgetting to call `destroy` leaks memory. Pair every `create` call with a `destroy` call, or use a PMA smart pointer with the `DefaultInstanceDestroyer` policy.
- This class is conditionally compiled under `DNA_BUILD_WITH_JSON_SUPPORT`. Without that define the class is absent from the build.

### Description

`JSONStreamWriter` uses a factory pattern — construction via `create()`, destruction via `destroy()` — rather than direct `new`/`delete`, to keep allocations routed through the optional `MemoryResource`. The two `setFrom` overloads let you copy data from either a `BinaryStreamReader` or a `JSONStreamReader`, selecting which `DataLayer` to include and how to handle unknown layers via `UnknownLayerPolicy`.

<!-- ink:api-end name="JSONStreamWriter" -->

<!-- ink:api name="type" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `using type = pma::FactoryCreate<dna::JSONStreamWriter>`

Member alias inside `DefaultInstanceCreator<dna::JSONStreamWriter>` that resolves to the concrete factory creator type.

### Why this exists

This `using type` alias is the conventional member name expected by PMA's traits system. PMA utilities look up `DefaultInstanceCreator<T>::type` to find the factory functor to invoke when constructing a `T`. This alias makes `JSONStreamWriter` compatible with those utilities without any additional boilerplate.

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamWriter>` — the struct that contains this alias
- `pma::FactoryCreate<dna::JSONStreamWriter>` — the concrete type this resolves to

<!-- ink:api-end name="type" -->
