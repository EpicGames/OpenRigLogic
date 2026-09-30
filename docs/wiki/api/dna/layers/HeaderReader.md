# API Reference — `dna/layers/HeaderReader`

---

<!-- ink:api name="HeaderReader" module="dna/layers/HeaderReader" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI HeaderReader`

Read the file format generation and version of a rig's DNA data.

### When to use this

Query this interface when you need to determine the DNA file format version before deciding how to parse or validate a rig. Both accessors together identify the exact format revision — use `getFileFormatGeneration()` to distinguish major, structurally incompatible revisions and `getFileFormatVersion()` to distinguish minor revisions within a generation. Do not inherit from this class directly; inherit from `Reader` instead.

### Example

```cpp
// reader is a concrete Reader instance (e.g. BinaryStreamReader)
HeaderReader* hdr = reader;
std::uint16_t gen = hdr->getFileFormatGeneration();
std::uint16_t ver = hdr->getFileFormatVersion();
// e.g. gen == 3, ver == 0 identifies a specific DNA format revision
```

### Returns

`std::uint16_t` — the file format generation from `getFileFormatGeneration`, or the file format version from `getFileFormatVersion`.

### Watch out for

- The destructor is protected, so you cannot delete an instance through this interface.
- Do not inherit from `HeaderReader` directly. The Doxygen `@warning` states that implementors must inherit from `Reader` itself; `HeaderReader` is an abstract accessor layer only.
<!-- ink:api-end name="HeaderReader" -->
