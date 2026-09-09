# API Reference — `dna/StreamReader`

---

<!-- ink:api name="StreamReader" module="dna/StreamReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class StreamReader : public Reader`

Extends `Reader` with the ability to actually pull rig data from a stream into internal structures.

### When to use this

Use `StreamReader` as the common base when you need to read DNA data from any stream-backed source rather than the plain in-memory `Reader` interface — concrete stream formats like `BinaryStreamReader` and `JSONStreamReader` derive from it.

### Method groups

| Group | Methods |
|-------|---------|
| I/O | read |

### Example

```cpp
StreamReader* reader = BinaryStreamReader::create(&stream);
reader->read();
// reader now has data loaded into internal structures
```

### Raises

- `SignatureMismatchError` — thrown when the stream's leading signature doesn't match the expected DNA format signature. Verify the file is a genuine DNA stream before retrying.
- `VersionMismatchError` — thrown when the stream's format version isn't supported by this reader. Use a compatible reader version or re-export the file.
- `InvalidDataError` — thrown when the stream's data fails structural validation during `read`. Check that the source stream wasn't truncated or corrupted.
- `SignatureMismatchError` — Set on the status context when the stream does not begin with the expected DNA file signature. Verify that the stream points to a valid DNA file and that the stream position is at the start.
- `InvalidDataError` — Set when the stream contains structurally malformed data. Inspect the source asset for corruption or truncation.

### Watch out for

- `StreamReader` cannot be instantiated directly — use a concrete subclass (`BinaryStreamReader`, `JSONStreamReader`). There is no factory on `StreamReader` itself.
- The three static `sc::StatusCode` members are error sentinels for the status-context pattern, not C++ exceptions. After calling `read()`, check `sc::Status::currentCode()` rather than wrapping in a try/catch.

<!-- ink:api-end name="StreamReader" -->
