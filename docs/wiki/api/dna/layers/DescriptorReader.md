# API Reference — `dna/layers/DescriptorReader`

---

<!-- ink:api name="DescriptorReader" module="dna/layers/DescriptorReader" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI DescriptorReader : public HeaderReader`

Read metadata about the character and the rig: name, archetype, gender, age, key-value metadata, coordinate conventions, LOD count and database info.

### When to use this

Use it when you need to know how a DNA's data is to be interpreted (units, coordinate system, rotation conventions) or where the character comes from. Implementors should inherit from `Reader` itself, not from this class.

### Method groups

| Group | Methods |
|-------|---------|
| Character | getName, getArchetype, getGender, getAge |
| MetaData | getMetaDataCount, getMetaDataKey, getMetaDataValue |
| Conventions | getTranslationUnit, getRotationUnit, getCoordinateSystem, getRotationSequence, getRotationSign, getFaceWindingOrder |
| LOD | getLODCount, getDBMaxLOD |
| DB | getDBComplexity, getDBName |

### Example

```cpp
// Obtain a DescriptorReader via a concrete Reader (e.g., BinaryStreamReader)
dna::DescriptorReader* desc = reader;

// Identity group
dna::StringView name     = desc->getName();
dna::Archetype archetype = desc->getArchetype();

// Coordinate conventions group
dna::CoordinateSystem coordSys = desc->getCoordinateSystem();

// LOD group
std::uint16_t lodCount = desc->getLODCount();   // e.g., 6 → levels 0–5
std::uint16_t maxLOD   = desc->getDBMaxLOD();   // relative to LOD-0 in the database

// Database group
dna::StringView dbName = desc->getDBName();
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `index` | `std::uint32_t` | required. Position in the key-value array. Must be less than `getMetaDataCount`. |
| `key` | `const char*` | required. Null-terminated key. |

### Returns

`StringView` for strings, enums for conventions, and `std::uint16_t` counts. For `getMetaDataValue`, an unknown key yields a view containing nullptr with size 0.

### Watch out for

- `getMetaDataValue` requires a null-terminated key.
- `getDBMaxLOD` is relative to LOD-0 from the database.
- Characters from the same database must have the same Definition, but may differ in complexity or LOD.
- Inherit from `Reader` (not `DescriptorReader`) when implementing a custom reader. Direct inheritance from this interface is not supported.
- `getMetaDataKey(index)` requires `index < getMetaDataCount()`. Passing an out-of-range index is undefined behavior.
- `getMetaDataValue(key)` requires a null-terminated `key`. If the key has no associated value, the returned `StringView` contains `nullptr` with size 0 — check the size before dereferencing.
<!-- ink:api-end name="DescriptorReader" -->
