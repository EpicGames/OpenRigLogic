# API Reference — `dna/layers/HeaderWriter`

---

<!-- ink:api name="HeaderWriter" module="dna/layers/HeaderWriter" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI HeaderWriter`

Set the file format generation and version of a rig's DNA data.

### When to use this

Use this interface when implementing a custom writer that needs to stamp file format metadata into the DNA header. Subclass `Writer` (not `HeaderWriter` directly) to obtain a complete writer implementation — `HeaderWriter` is one of several layered interfaces `Writer` composes.

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

| Name | Type | Description |
|------|------|-------------|
| `generation` | `std::uint16_t` | required. File format generation to store. |
| `version` | `std::uint16_t` | required. File format version to store. |

### Watch out for

- The destructor is protected, so you cannot delete an instance through this interface.
- Do not inherit from `HeaderWriter` directly. Inherit from `Writer`, which composes `HeaderWriter` along with the other layer writer interfaces. Subclassing `HeaderWriter` alone produces an incomplete writer.
<!-- ink:api-end name="HeaderWriter" -->
