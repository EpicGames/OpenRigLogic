# API Reference — `dna/BinaryStreamWriter`

---

<!-- ink:api name="BinaryStreamWriter" module="dna/BinaryStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class BinaryStreamWriter : public StreamWriter`

Serialize DNA rig data to a binary stream. Use `create()` to get an instance, populate it via `setFrom()`, then write to the stream.

### When to use this

Use this class when you need to write rig data in the compact binary DNA format — for example, when converting from a JSON source or copying a filtered subset of layers from an existing binary file. If you need JSON output instead, use `JSONStreamWriter`.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | `create`, `destroy`, `~BinaryStreamWriter` |
| Data transfer | `setFrom` (BinaryStreamReader), `setFrom` (JSONStreamReader) |

### Example

```cpp
// Create a writer targeting an output stream
auto* writer = dna::BinaryStreamWriter::create(outputStream);

// Copy all layers from an existing binary reader
writer->setFrom(binaryReader, dna::DataLayer::All, dna::UnknownLayerPolicy::Preserve);

// Write to stream, then release
writer->write();
dna::BinaryStreamWriter::destroy(writer);
```

### Returns

`create()` returns `BinaryStreamWriter*` — a heap-allocated instance. The caller owns this pointer and must release it with `destroy()`.

### Watch out for

- `create()` returns a raw pointer. You are responsible for calling `destroy()` when done — failing to do so leaks memory. Prefer wrapping in a `pma::ScopedPtr` or equivalent RAII guard.
- `setFrom()` accepts an optional `MemoryResource*`. If omitted, the default allocator is used. Pass an explicit resource if you need arena or pool allocation for large rigs.
- The `setFrom(JSONStreamReader*)` overload is only available when the library is built with `DNA_BUILD_WITH_JSON_SUPPORT`.

<!-- ink:api-end name="BinaryStreamWriter" -->

<!-- ink:api name="DefaultInstanceCreator" module="dna/BinaryStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<dna::BinaryStreamWriter>`

PMA trait specialization that routes `BinaryStreamWriter` instance creation through its factory method rather than `operator new`.

### Why this exists

`BinaryStreamWriter` uses a factory pattern (`create`/`destroy`) instead of public constructors, so the PMA system needs an explicit policy to know how to allocate it. This specialization wires `pma::FactoryCreate` as the creation strategy, ensuring that any PMA-managed scope that constructs a `BinaryStreamWriter` goes through the correct factory path and respects the memory resource passed at creation time.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::BinaryStreamWriter>` | required — the allocator strategy type used by PMA when constructing a `BinaryStreamWriter` |

### Relationships

- `DefaultInstanceDestroyer<dna::BinaryStreamWriter>` — paired destroyer trait; must be present alongside this specialization
- `pma::FactoryCreate` — the underlying creation policy this trait delegates to
- `BinaryStreamWriter::create` — the factory method called by this policy

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/BinaryStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<dna::BinaryStreamWriter>`

PMA trait specialization that routes `BinaryStreamWriter` instance destruction through its `destroy()` factory method rather than `delete`.

### Why this exists

Because `BinaryStreamWriter` exposes a `destroy(instance)` static method instead of a public destructor, PMA needs an explicit policy to release instances correctly. This specialization wires `pma::FactoryDestroy` as the destruction strategy, ensuring that any PMA-managed scope that owns a `BinaryStreamWriter` calls the correct teardown path.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::BinaryStreamWriter>` | required — the deallocator strategy type used by PMA when destroying a `BinaryStreamWriter` |

### Relationships

- `DefaultInstanceCreator<dna::BinaryStreamWriter>` — paired creator trait; must be present alongside this specialization
- `pma::FactoryDestroy` — the underlying destruction policy this trait delegates to
- `BinaryStreamWriter::destroy` — the factory method called by this policy

<!-- ink:api-end name="DefaultInstanceDestroyer" -->
