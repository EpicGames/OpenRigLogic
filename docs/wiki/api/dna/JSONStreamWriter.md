# API Reference — `dna/JSONStreamWriter`

---

<!-- ink:api name="BinaryStreamReader" module="dna/JSONStreamWriter" last_commit="api_scan" updated="2026-06-10" api_kind="callable" -->

## `class BinaryStreamReader`

Forward-declared peer type used as a data source when copying binary DNA data into a `JSONStreamWriter` via `setFrom`.

### When to use this

Pass a `BinaryStreamReader*` to `JSONStreamWriter::setFrom` when you need to convert an existing binary DNA stream into JSON output. `BinaryStreamReader` is defined in its own header — this forward declaration exists solely to enable the `setFrom` overload signature in `JSONStreamWriter.h`.

### Relationships

- `JSONStreamWriter::setFrom(const BinaryStreamReader*)` — consumes this type as the copy source
- `JSONStreamReader` — the JSON-format peer; use when copying from an existing JSON source instead

<!-- ink:api-end name="BinaryStreamReader" -->

<!-- ink:api name="DefaultInstanceCreator" module="dna/JSONStreamWriter" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `pma::DefaultInstanceCreator<dna::JSONStreamWriter>`

Template specialization that tells generic `pma` factory code to use `JSONStreamWriter::create` when constructing instances of this type.

### Why this exists

Routes construction of `JSONStreamWriter` through its static `create` method so generic `pma` ownership utilities don't need to special-case this type's custom allocation.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::JSONStreamWriter>` | the creator implementation used by generic `pma` code for this type |

### Relationships

- `DefaultInstanceDestroyer<dna::JSONStreamWriter>` — *the matching specialization used to destroy instances created this way*
- `JSONStreamWriter::create` — *the factory method this specialization delegates to*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/JSONStreamWriter" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `pma::DefaultInstanceDestroyer<dna::JSONStreamWriter>`

Template specialization that tells generic `pma` factory code to use `JSONStreamWriter::destroy` when releasing instances of this type.

### Why this exists

Because `JSONStreamWriter` instances are created through a custom factory, they must be released through the matching `destroy` static method rather than `delete`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::JSONStreamWriter>` | the destroyer implementation used by generic `pma` code for this type |

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamWriter>` — *the matching specialization used to create instances*
- `JSONStreamWriter::destroy` — *the factory method this specialization delegates to*

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="JSONStreamReader" module="dna/JSONStreamWriter" last_commit="api_scan" updated="2026-06-10" api_kind="callable" -->

## `class JSONStreamReader`

Forward-declared peer type used as a data source when copying JSON DNA data into a `JSONStreamWriter` via `setFrom`.

### When to use this

Pass a `JSONStreamReader*` to `JSONStreamWriter::setFrom` when you need to re-serialize or transform an existing JSON DNA stream — for example, to filter to a specific `DataLayer` before writing. Use `BinaryStreamReader` instead when your source data is in binary format.

### Relationships

- `JSONStreamWriter::setFrom(const JSONStreamReader*)` — consumes this type as the copy source
- `BinaryStreamReader` — the binary-format peer; use when converting from binary DNA to JSON

<!-- ink:api-end name="JSONStreamReader" -->

<!-- ink:api name="JSONStreamWriter" module="dna/JSONStreamWriter" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class JSONStreamWriter : public StreamWriter`

Write DNA rig data out as human-readable JSON, and optionally populate it directly from an existing `BinaryStreamReader` or `JSONStreamReader`.

### When to use this

Use `JSONStreamWriter` when you need a diffable, hand-editable representation of the rig — for example exporting a binary DNA to JSON for review or version control. Use `BinaryStreamWriter` instead when output size and load speed matter more than readability.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | create, destroy |
| Data transfer | setFrom |

### Example

```cpp
FileStream stream("rig.json", FileStream::AccessMode::Write, FileStream::OpenMode::Text);
JSONStreamWriter* writer = JSONStreamWriter::create(&stream, 4u);
writer->setFrom(binaryReader, DataLayer::All, UnknownLayerPolicy::Preserve);
writer->write();
JSONStreamWriter::destroy(writer);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — stream into which the data is written |
| `indentWidth` | `std::uint32_t` | optional — number of spaces used for indentation; defaults to 4 |
| `memRes` | `MemoryResource*` | optional — allocator for internal structures; defaults to a built-in allocator when omitted |

### Watch out for

- You own the returned pointer. Forgetting to call `destroy` leaks memory. Pair every `create` call with a `destroy` call, or use a PMA smart pointer with the `DefaultInstanceDestroyer` policy.
- This class is conditionally compiled under `DNA_BUILD_WITH_JSON_SUPPORT`. Without that define the class is absent from the build.

<!-- ink:api-end name="JSONStreamWriter" -->
