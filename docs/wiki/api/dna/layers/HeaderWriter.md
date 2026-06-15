# API Reference — `dna/layers/HeaderWriter`

---

<!-- ink:api name="HeaderWriter" module="dna/layers/HeaderWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI HeaderWriter`

Set the file format generation and version fields in a rig's DNA header during a write pass.

### When to use this

Use this interface when implementing a custom writer that needs to stamp file format metadata into the DNA header. Subclass `Writer` (not `HeaderWriter` directly) to obtain a complete writer implementation — `HeaderWriter` is one of several layered interfaces `Writer` composes.

### Method groups

| Group | Methods |
|-------|---------|
| File format metadata | `setFileFormatGeneration`, `setFileFormatVersion` |

### Example

```cpp
// Concrete writer subclasses Writer, which inherits HeaderWriter
class MyDNAWriter : public dna::Writer {
public:
    void writeHeader() {
        // Stamp the file format generation and version
        setFileFormatGeneration(1);
        setFileFormatVersion(0);
    }
};
```

### Parameters

**`setFileFormatGeneration`**

| Name | Type | Description |
|------|------|-------------|
| `generation` | `std::uint16_t` | required — the file format generation number to embed in the DNA header |

**`setFileFormatVersion`**

| Name | Type | Description |
|------|------|-------------|
| `version` | `std::uint16_t` | required — the file format version number to embed in the DNA header |

### Watch out for

- Do not inherit from `HeaderWriter` directly. Inherit from `Writer`, which composes `HeaderWriter` along with the other layer writer interfaces. Subclassing `HeaderWriter` alone produces an incomplete writer.

<!-- ink:api-end name="HeaderWriter" -->
