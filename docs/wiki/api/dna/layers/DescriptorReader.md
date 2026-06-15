# API Reference — `dna/layers/DescriptorReader`

---

<!-- ink:api name="DescriptorReader" module="dna/layers/DescriptorReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI DescriptorReader : public HeaderReader`

Query character identity, rig configuration, coordinate conventions, LOD range, and database provenance from a loaded DNA asset.

### When to use this

Use `DescriptorReader` when you need to inspect a character's metadata — name, archetype, LOD range, coordinate system, or database origin — before performing a full behavior or geometry read. This interface is for consumers of DNA data; if you are implementing a reader type, inherit from `Reader` instead of this class.

### Method groups

| Group | Methods |
|-------|---------|
| Identity | `getName`, `getArchetype`, `getGender`, `getAge` |
| Metadata | `getMetaDataCount`, `getMetaDataKey`, `getMetaDataValue` |
| Coordinate conventions | `getTranslationUnit`, `getRotationUnit`, `getCoordinateSystem`, `getRotationSequence`, `getRotationSign`, `getFaceWindingOrder` |
| LOD | `getLODCount`, `getDBMaxLOD` |
| Database | `getDBComplexity`, `getDBName` |

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

### Watch out for

- Inherit from `Reader` (not `DescriptorReader`) when implementing a custom reader. Direct inheritance from this interface is not supported.
- `getMetaDataKey(index)` requires `index < getMetaDataCount()`. Passing an out-of-range index is undefined behavior.
- `getMetaDataValue(key)` requires a null-terminated `key`. If the key has no associated value, the returned `StringView` contains `nullptr` with size 0 — check the size before dereferencing.

<!-- ink:api-end name="DescriptorReader" -->
