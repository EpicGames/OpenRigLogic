# API Reference — `dna/StreamReader`

---

<!-- ink:api name="StreamReader" module="dna/StreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class StreamReader : public Reader`

Abstract base for DNA stream readers — defines the `read()` contract and error status codes shared by all concrete reader implementations.

### When to use this

Inherit from `StreamReader` (via `BinaryStreamReader` or `JSONStreamReader`) when you need to load DNA data from an I/O stream. Use the static status codes — `SignatureMismatchError`, `VersionMismatchError`, `InvalidDataError` — to interpret failure results after calling `read()` without needing to know which concrete subclass is in use.

### Example

```cpp
// Obtain a concrete reader via BinaryStreamReader::create()
dna::BinaryStreamReader* reader = dna::BinaryStreamReader::create(stream);
reader->read();

// Check the status context after read()
if (sc::Status::currentCode() == dna::StreamReader::SignatureMismatchError) {
    // The stream does not contain a valid DNA file
}
if (sc::Status::currentCode() == dna::StreamReader::VersionMismatchError) {
    // The DNA version is not supported by this reader build
}

dna::BinaryStreamReader::destroy(reader);
```

### Raises

- `SignatureMismatchError` — Set on the status context when the stream does not begin with the expected DNA file signature. Verify that the stream points to a valid DNA file and that the stream position is at the start.
- `VersionMismatchError` — Set when the DNA file version is incompatible with this reader. Check that the DNA file was produced by a compatible toolchain version.
- `InvalidDataError` — Set when the stream contains structurally malformed data. Inspect the source asset for corruption or truncation.

### Watch out for

- `StreamReader` cannot be instantiated directly — use a concrete subclass (`BinaryStreamReader`, `JSONStreamReader`). There is no factory on `StreamReader` itself.
- The three static `sc::StatusCode` members are error sentinels for the status-context pattern, not C++ exceptions. After calling `read()`, check `sc::Status::currentCode()` rather than wrapping in a try/catch.

<!-- ink:api-end name="StreamReader" -->
