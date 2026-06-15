# API Reference — `dna/JSONStreamReader`

---

<!-- ink:api name="DefaultInstanceCreator" module="dna/JSONStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<dna::JSONStreamReader>`

Policy specialization that maps `pma`'s generic construction mechanism to `JSONStreamReader`'s `create()` factory method.

### Why this exists

`pma` uses `DefaultInstanceCreator<T>` as a customization point so that managed containers and scoped helpers can construct any type without calling `new` directly. This specialization tells `pma` to use `FactoryCreate<dna::JSONStreamReader>` — which delegates to `JSONStreamReader::create()` — rather than a raw constructor. Without this specialization, `pma` infrastructure could not safely manage `JSONStreamReader` instances.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::JSONStreamReader>` | required — the factory creator type used by `pma` to construct `JSONStreamReader` instances |

### Relationships

- `JSONStreamReader` — the type this policy governs construction of
- `DefaultInstanceDestroyer<dna::JSONStreamReader>` — paired destruction policy
- `pma::FactoryCreate` — the underlying creator implementation

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/JSONStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<dna::JSONStreamReader>`

Policy specialization that maps `pma`'s generic destruction mechanism to `JSONStreamReader`'s `destroy()` factory method.

### Why this exists

`pma` uses `DefaultInstanceDestroyer<T>` as a customization point so that managed scopes and containers can release any type without calling `delete` directly. This specialization tells `pma` to use `FactoryDestroy<dna::JSONStreamReader>` — which delegates to `JSONStreamReader::destroy()` — ensuring destruction always goes through the correct factory path and avoids mismatched allocator teardown.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::JSONStreamReader>` | required — the factory destroyer type used by `pma` to release `JSONStreamReader` instances |

### Relationships

- `JSONStreamReader` — the type this policy governs destruction of
- `DefaultInstanceCreator<dna::JSONStreamReader>` — paired construction policy
- `pma::FactoryDestroy` — the underlying destroyer implementation

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="JSONStreamReader" module="dna/JSONStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class JSONStreamReader : public StreamReader`

Factory-managed stream reader that deserializes DNA data from a JSON-formatted source. Use this when your DNA asset is stored in JSON format rather than the binary format handled by `BinaryStreamReader`.

### When to use this

Reach for this when your DNA pipeline produces or stores assets as JSON — for example, during development or tooling workflows where human-readable output matters. For production runtime loading, prefer `BinaryStreamReader`: it is faster and produces smaller in-memory representations. Both classes share the same `StreamReader` interface, so switching between them requires only a factory-call change.

### Example

```cpp
// Open a JSON DNA file and read it into the runtime structure
auto* stream = pma::makeScoped<dna::FileStream>("character.dna", dna::FileStream::AccessMode::Read);
auto* reader = dna::JSONStreamReader::create(stream.get());
reader->read();
// Use reader via the StreamReader interface...
dna::JSONStreamReader::destroy(reader);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — the JSON-formatted source stream to read DNA data from |
| `memRes` | `MemoryResource*` | optional — custom memory resource for internal allocations; uses the default allocator when `nullptr` |

### Returns

`JSONStreamReader*` — a heap-allocated reader instance. The caller owns this pointer and must release it by calling `destroy()`.

### Watch out for

- The returned pointer is **caller-owned**. Failing to call `destroy(instance)` leaks memory. Do not delete it with `delete` — always use the paired `destroy()` static method.
- `memRes` governs only internal allocations made by the reader. The `stream` lifetime must exceed the reader's lifetime; the reader does not take ownership of the stream.

<!-- ink:api-end name="JSONStreamReader" -->

<!-- ink:api name="type" module="dna/JSONStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type`

Type alias that resolves to `pma::FactoryCreate<dna::JSONStreamReader>`, the factory creator policy used by `pma` managed infrastructure to construct `JSONStreamReader` instances.

### Why this exists

The `pma` customization-point protocol requires a nested `type` alias inside each `DefaultInstanceCreator` specialization. This alias is the single token that `pma` reads when it needs to know how to construct a `T` — here, delegating to `JSONStreamReader::create()`.

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamReader>` — the enclosing policy struct that declares this alias
- `pma::FactoryCreate<dna::JSONStreamReader>` — the resolved type

<!-- ink:api-end name="type" -->
