# API Reference — `dna/BinaryStreamReader`

---

<!-- ink:api name="BinaryStreamReader" module="dna/BinaryStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class BinaryStreamReader : public StreamReader`

Read a DNA binary stream by creating a scoped reader instance that loads a configurable subset of data layers and LOD levels.

### When to use this

Use `BinaryStreamReader` when you need to deserialize a DNA binary file from a `BoundedIOStream` and want precise control over which data layers are loaded and which LOD levels are included. Choose the `Configuration`-based overload when filtering parameters are managed centrally; choose the `DataLayer` + LOD overloads when you need explicit per-call control. For JSON-format DNA files, use `JSONStreamReader` instead.

### Example

```cpp
// Open a binary DNA stream and read only Behavior data for the top two LODs
BoundedIOStream* stream = createFileStream("character.dna");
BinaryStreamReader* reader = BinaryStreamReader::create(
    stream,
    DataLayer::Behavior,
    UnknownLayerPolicy::Preserve,
    /*maxLOD=*/1u
);
// Use reader to access descriptor, definition, and behavior data
// ...
BinaryStreamReader::destroy(reader);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — source stream from which binary DNA data is read. |
| `config` | `const Configuration&` | optional (default `{}`) — centralized filtering and adaptation parameters; see `Configuration`. Used in overload 1 only. |
| `layer` | `DataLayer` | required (overloads 2 and 3) — the highest data layer to load. Lower layers are loaded implicitly: `Definition` loads `Descriptor`; `Behavior` loads `Definition`; `Geometry` loads `Definition` (but not `Behavior`). |
| `policy` | `UnknownLayerPolicy` | optional (default `Preserve`) — whether unrecognized layers are preserved in memory or discarded. |
| `maxLOD` | `std::uint16_t` | optional (default `0`) — the maximum (coarsest) LOD index to load. `0` loads all LODs. Must be less than `getLODCount()`. |
| `minLOD` | `std::uint16_t` | optional (overload 3, default implied) — the minimum (finest) LOD index to load. Combined with `maxLOD`, defines a range; `[0, LODCount-1]` loads all LODs. Must be less than `getLODCount()`. |
| `memRes` | `MemoryResource*` | optional (default `nullptr`) — memory resource for all internal allocations. When `nullptr`, the default allocator is used. |

### Returns

`BinaryStreamReader*` — a caller-owned pointer to the newly created reader. You must release it by calling `BinaryStreamReader::destroy(reader)` — do not use `delete`.

### Watch out for

- The returned pointer is caller-owned. Failing to call `destroy` leaks memory. Do not call `delete` directly — use `destroy`.
- `maxLOD` and `minLOD` must each be less than the value returned by `getLODCount()`. Passing an out-of-range value results in undefined behavior.
- `Geometry` and `Behavior` layers are independent — loading `Geometry` does not load `Behavior` data and vice versa. Both depend on and implicitly load `Definition`.

<!-- ink:api-end name="BinaryStreamReader" -->

<!-- ink:api name="DefaultInstanceCreator" module="dna/BinaryStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<dna::BinaryStreamReader>`

Specializes the `pma` factory-creation policy for `BinaryStreamReader` so that `pma` managed handles construct instances via `FactoryCreate`.

### Why this exists

The `pma` memory management library uses a traits-based policy system to determine how objects are constructed when managed via `pma` smart handles. `BinaryStreamReader` uses a private constructor with factory-only creation, so the default construction strategy is incompatible. This specialization tells `pma` to use `FactoryCreate<BinaryStreamReader>` — which routes through `BinaryStreamReader::create()` — instead of a direct constructor call.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::BinaryStreamReader>` | The creation policy type used by `pma` handles for `BinaryStreamReader`. |

### Relationships

- `DefaultInstanceDestroyer<dna::BinaryStreamReader>` — the paired destruction policy; always defined alongside this specialization.
- `pma::FactoryCreate` — the underlying creation mechanism this trait delegates to.
- `BinaryStreamReader::create()` — the factory method invoked by `FactoryCreate`.

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/BinaryStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<dna::BinaryStreamReader>`

Specializes the `pma` factory-destruction policy for `BinaryStreamReader` so that `pma` managed handles release instances via `FactoryDestroy`.

### Why this exists

`BinaryStreamReader` instances must be released through `BinaryStreamReader::destroy()`, not via `delete`. Without this specialization, `pma` managed handles would attempt direct deletion, bypassing the factory destructor and causing undefined behavior. This trait routes `pma` handle teardown through `FactoryDestroy<BinaryStreamReader>`, which calls the correct `destroy()` factory method.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::BinaryStreamReader>` | The destruction policy type used by `pma` handles for `BinaryStreamReader`. |

### Relationships

- `DefaultInstanceCreator<dna::BinaryStreamReader>` — the paired creation policy; always defined alongside this specialization.
- `pma::FactoryDestroy` — the underlying destruction mechanism this trait delegates to.
- `BinaryStreamReader::destroy()` — the factory method invoked by `FactoryDestroy`.

<!-- ink:api-end name="DefaultInstanceDestroyer" -->
