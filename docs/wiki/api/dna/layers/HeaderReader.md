# API Reference — `dna/layers/HeaderReader`

---

<!-- ink:api name="HeaderReader" module="dna/layers/HeaderReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class HeaderReader`

Read the file format generation and version numbers from a DNA rig's header block.

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

`std::uint16_t` — unsigned 16-bit integer representing the file format generation or version number stored in the DNA header.

### Watch out for

- Do not inherit from `HeaderReader` directly. The Doxygen `@warning` states that implementors must inherit from `Reader` itself; `HeaderReader` is an abstract accessor layer only.

<!-- ink:api-end name="HeaderReader" -->
