# API Reference — `dna/BinaryStreamReader`

---

<!-- ink:api name="BinaryStreamReader" module="dna/BinaryStreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class DNAAPI BinaryStreamReader : public StreamReader`

Reads DNA rig data from a binary stream, with factory methods that let callers control which data layers and levels of detail get loaded.

### When to use this

Use `BinaryStreamReader::create` when you need to load a `.dna` file (or any `BoundedIOStream` of DNA binary data) and either want the full dataset, or want to limit loading to a specific `DataLayer` and LOD range to reduce memory and load time. Loading a later layer (e.g. `Behavior`) implicitly loads its dependencies (`Definition`, which in turn depends on `Descriptor`), so callers don't need to request each layer manually.

### Method groups

| Group | Methods |
|-------|---------|
| Creation | create (stream + config), create (stream + layer + policy + maxLOD), create (stream + layer + policy + maxLOD + minLOD) |

### Example

```cpp
dna::BoundedIOStream* stream = /* opened .dna file stream */;
dna::BinaryStreamReader* reader = dna::BinaryStreamReader::create(
    stream,
    dna::DataLayer::Behavior,
    dna::UnknownLayerPolicy::Preserve,
    /*maxLOD=*/0u);
// reader now owns the Behavior layer and its Definition/Descriptor dependencies
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — source stream from which data is going to be read |
| `config` | `const Configuration&` | optional — filtering and adaptation parameters |
| `layer` | `DataLayer` | required (in layer-based overloads) — the layer up to which data needs to be loaded |
| `policy` | `UnknownLayerPolicy` | optional — whether unknown layers are preserved or ignored; defaults to `Preserve` |
| `maxLOD` | `std::uint16_t` | optional — maximum level of detail to load; 0 loads all LODs; must be less than `getLODCount()` |
| `minLOD` | `std::uint16_t` | optional — minimum level of detail to load, paired with `maxLOD` |
| `memRes` | `MemoryResource*` | optional — memory resource used for allocations; a default mechanism is used if omitted |

### Returns

`BinaryStreamReader*` — a newly created reader instance. The caller is responsible for releasing it by calling `destroy`.

### Watch out for

- The caller owns the returned pointer and must release it via `destroy` — `create` does not return a smart pointer.
- `maxLOD` (and `minLOD`, where applicable) must be less than the value returned by `getLODCount()`, or behavior is undefined.
- The returned pointer is caller-owned. Failing to call `destroy` leaks memory. Do not call `delete` directly — use `destroy`.
- `Geometry` and `Behavior` layers are independent — loading `Geometry` does not load `Behavior` data and vice versa. Both depend on and implicitly load `Definition`.

<!-- ink:api-end name="BinaryStreamReader" -->

<!-- ink:api name="DefaultInstanceCreator" module="dna/BinaryStreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<> struct DefaultInstanceCreator<dna::BinaryStreamReader>`

A `pma` framework specialization that tells the generic instance-creation machinery how to construct a `BinaryStreamReader` by default.

### Why this exists

Generic `pma` code that creates instances of arbitrary types by template parameter needs a per-type hook for construction. This specialization routes `BinaryStreamReader` creation through `pma::FactoryCreate<dna::BinaryStreamReader>`, matching the factory-based `create()` methods `BinaryStreamReader` actually exposes, rather than a plain constructor call.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | typedef | `pma::FactoryCreate<dna::BinaryStreamReader>` — the creation strategy used for this type. |

### Relationships

- `BinaryStreamReader` — *the type this specialization knows how to create*
- `DefaultInstanceDestroyer<dna::BinaryStreamReader>` — *the paired specialization for destruction*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/BinaryStreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<> struct DefaultInstanceDestroyer<dna::BinaryStreamReader>`

A `pma` framework specialization that tells the generic instance-destruction machinery how to release a `BinaryStreamReader` created via the factory pattern.

### Why this exists

Because `BinaryStreamReader` is created through `pma::FactoryCreate` rather than `new`, it must also be destroyed through the matching `pma::FactoryDestroy` rather than `delete`. This specialization ensures generic `pma` cleanup code calls the correct destruction path automatically instead of requiring every call site to know this detail.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | typedef | `pma::FactoryDestroy<dna::BinaryStreamReader>` — the destruction strategy used for this type. |

### Relationships

- `BinaryStreamReader` — *the type this specialization knows how to destroy*
- `DefaultInstanceCreator<dna::BinaryStreamReader>` — *the paired specialization for creation*

<!-- ink:api-end name="DefaultInstanceDestroyer" -->
