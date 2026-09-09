# API Reference — `dna/BinaryStreamWriter`

---

<!-- ink:api name="BinaryStreamReader" module="dna/BinaryStreamWriter" last_commit="api_scan" updated="2026-07-31" api_kind="data_shape" -->

## `BinaryStreamReader`

A forward-declared reader type that `BinaryStreamWriter` accepts as a data source when copying DNA content between streams.

### Why this exists

`BinaryStreamWriter::setFrom(const BinaryStreamReader* source, ...)` needs a reader type to pull data from without the writer header owning the reader's full definition, so `BinaryStreamReader` is only forward-declared here to avoid a circular include between the reader and writer headers.

### Relationships

- `BinaryStreamWriter` — *consumes a `BinaryStreamReader*` in `setFrom` to copy binary DNA data into itself.*
- `JSONStreamReader` — *the alternate reader type accepted by the JSON-enabled overload of `setFrom`.*

<!-- ink:api-end name="BinaryStreamReader" -->

<!-- ink:api name="BinaryStreamWriter" module="dna/BinaryStreamWriter" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class BinaryStreamWriter : public StreamWriter`

Write DNA rig data to a binary stream, and optionally populate it directly from an existing `BinaryStreamReader` or `JSONStreamReader` instance.

### When to use this

Use `BinaryStreamWriter` when you need to persist rig data in the compact binary DNA format — for example converting a JSON-authored rig back to binary, or copying select layers from one binary stream to another. Reach for `JSONStreamWriter` instead if the output needs to be human-readable.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | create, destroy |
| Data transfer | setFrom |

### Example

```cpp
FileStream stream("output.dna", FileStream::AccessMode::Write, FileStream::OpenMode::Binary);
BinaryStreamWriter* writer = BinaryStreamWriter::create(&stream);
writer->setFrom(binaryReader, DataLayer::All, UnknownLayerPolicy::Preserve);
writer->write();
BinaryStreamWriter::destroy(writer);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — stream into which the data is written |
| `memRes` | `MemoryResource*` | optional — allocator for internal structures; defaults to a built-in allocator when omitted |

### Watch out for

- `setFrom(const JSONStreamReader*, ...)` is only available when the library is built with `DNA_BUILD_WITH_JSON_SUPPORT` defined; calling it in a build without JSON support will not compile.
- The instance returned by `create` must be released via `destroy`, not `delete`.
- `create()` returns a raw pointer. You are responsible for calling `destroy()` when done — failing to do so leaks memory. Prefer wrapping in a `pma::ScopedPtr` or equivalent RAII guard.
- `setFrom()` accepts an optional `MemoryResource*`. If omitted, the default allocator is used. Pass an explicit resource if you need arena or pool allocation for large rigs.

<!-- ink:api-end name="BinaryStreamWriter" -->

<!-- ink:api name="DefaultInstanceCreator" module="dna/BinaryStreamWriter" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `pma::DefaultInstanceCreator<dna::BinaryStreamWriter>`

Template specialization that tells generic `pma` factory code to use `BinaryStreamWriter::create` when constructing instances of this type.

### Why this exists

Generic smart-pointer and factory utilities in `pma` need a uniform way to construct arbitrary types without special-casing each one. This specialization routes construction of `BinaryStreamWriter` through its own static `create` method rather than a raw constructor, preserving the class's custom allocation and factory semantics.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::BinaryStreamWriter>` | the creator implementation used by generic `pma` code for this type |

### Relationships

- `DefaultInstanceDestroyer<dna::BinaryStreamWriter>` — *the matching specialization used to destroy instances created this way*
- `BinaryStreamWriter::create` — *the factory method this specialization delegates to*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/BinaryStreamWriter" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `pma::DefaultInstanceDestroyer<dna::BinaryStreamWriter>`

Template specialization that tells generic `pma` factory code to use `BinaryStreamWriter::destroy` when releasing instances of this type.

### Why this exists

Because `BinaryStreamWriter` instances are created through a custom factory rather than `new`, they must be released through the matching `destroy` static method. This specialization lets generic ownership utilities in `pma` call the correct cleanup path automatically instead of calling `delete` on the pointer.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::BinaryStreamWriter>` | the destroyer implementation used by generic `pma` code for this type |

### Relationships

- `DefaultInstanceCreator<dna::BinaryStreamWriter>` — *the matching specialization used to create instances*
- `BinaryStreamWriter::destroy` — *the factory method this specialization delegates to*

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="JSONStreamReader" module="dna/BinaryStreamWriter" last_commit="api_scan" updated="2026-07-31" api_kind="data_shape" -->

## `JSONStreamReader`

A forward-declared reader type that `BinaryStreamWriter` accepts as a data source when the build supports JSON, letting a writer be populated from JSON-formatted DNA data.

### Why this exists

The `setFrom(const JSONStreamReader* source, ...)` overload is only compiled when `DNA_BUILD_WITH_JSON_SUPPORT` is defined, so `JSONStreamReader` is forward-declared behind the same guard rather than included directly — this keeps JSON support optional without forcing all consumers of `BinaryStreamWriter.h` to pull in JSON dependencies.

### Constraints

- Only available when the library is built with `DNA_BUILD_WITH_JSON_SUPPORT` defined.

### Relationships

- `BinaryStreamWriter` — *consumes a `JSONStreamReader*` in the JSON-only overload of `setFrom`.*
- `BinaryStreamReader` — *the non-JSON counterpart reader type accepted by the other `setFrom` overload.*

<!-- ink:api-end name="JSONStreamReader" -->

