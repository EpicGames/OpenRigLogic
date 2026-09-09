# API Reference — `dna/JSONStreamReader`

---

<!-- ink:api name="DefaultInstanceCreator" module="dna/JSONStreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `pma::DefaultInstanceCreator<dna::JSONStreamReader>`

Template specialization that tells generic `pma` factory code to use `JSONStreamReader::create` when constructing instances of this type.

### Why this exists

Generic `pma` factory utilities need a uniform construction hook per type. This specialization routes construction of `JSONStreamReader` through its static `create` method rather than a raw constructor.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryCreate<dna::JSONStreamReader>` | the creator implementation used by generic `pma` code for this type |

### Relationships

- `DefaultInstanceDestroyer<dna::JSONStreamReader>` — *the matching specialization used to destroy instances created this way*
- `JSONStreamReader::create` — *the factory method this specialization delegates to*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="dna/JSONStreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `pma::DefaultInstanceDestroyer<dna::JSONStreamReader>`

Template specialization that tells generic `pma` factory code to use `JSONStreamReader::destroy` when releasing instances of this type.

### Why this exists

Because `JSONStreamReader` instances are created through a custom factory rather than `new`, they must be released through the matching `destroy` static method rather than `delete`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `pma::FactoryDestroy<dna::JSONStreamReader>` | the destroyer implementation used by generic `pma` code for this type |

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamReader>` — *the matching specialization used to create instances*
- `JSONStreamReader::destroy` — *the factory method this specialization delegates to*

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="JSONStreamReader" module="dna/JSONStreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class JSONStreamReader : public StreamReader`

Read DNA rig data from a stream that holds the human-readable JSON representation instead of the compact binary format.

### When to use this

Use `JSONStreamReader` when the source DNA data is stored as JSON — for example a hand-edited or diff-friendly rig file. For the standard compact format, use `BinaryStreamReader` instead. Available only when the library is built with `DNA_BUILD_WITH_JSON_SUPPORT`.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | create, destroy |

### Example

```cpp
FileStream stream("rig.json", FileStream::AccessMode::Read, FileStream::OpenMode::Text);
JSONStreamReader* reader = JSONStreamReader::create(&stream);
reader->read();
JSONStreamReader::destroy(reader);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `stream` | `BoundedIOStream*` | required — source stream from which data is read |
| `memRes` | `MemoryResource*` | optional — allocator for internal structures; defaults to a built-in allocator when omitted |

### Watch out for

- The returned pointer is **caller-owned**. Failing to call `destroy(instance)` leaks memory. Do not delete it with `delete` — always use the paired `destroy()` static method.
- `memRes` governs only internal allocations made by the reader. The `stream` lifetime must exceed the reader's lifetime; the reader does not take ownership of the stream.

<!-- ink:api-end name="JSONStreamReader" -->
