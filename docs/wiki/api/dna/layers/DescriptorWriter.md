# API Reference — `dna/layers/DescriptorWriter`

---

<!-- ink:api name="DescriptorWriter" module="dna/layers/DescriptorWriter" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI DescriptorWriter : public HeaderWriter`

Write metadata about the character and the rig: name, archetype, gender, age, key-value metadata, coordinate conventions, LOD count and database info.

### When to use this

Use it when you author a DNA and need to set its descriptor. Implementors should inherit from `Writer` itself, not from this class.

### Method groups

| Group | Methods |
|-------|---------|
| Character | setName, setArchetype, setGender, setAge |
| MetaData | clearMetaData, setMetaData |
| Conventions | setTranslationUnit, setRotationUnit, setCoordinateSystem, setRotationSequence, setRotationSign, setFaceWindingOrder |
| LOD | setLODCount, setDBMaxLOD |
| DB | setDBComplexity, setDBName |

### Example

```cpp
// Obtain a writer from BinaryStreamWriter or equivalent Writer subclass.
// DescriptorWriter* dw = writer->getDescriptorWriter();

dw->setName("Ada");
dw->setArchetype(Archetype::other);
dw->setGender(Gender::female);
dw->setAge(30);

// Coordinate frame: Y-up, centimetre, XYZ rotation order.
dw->setTranslationUnit(TranslationUnit::cm);
dw->setCoordinateSystem(CoordinateSystem::RightHand_YUp);
dw->setRotationUnit(RotationUnit::degrees);
dw->setRotationSequence(RotationSequence::XYZ);

// Six LOD levels: 0 (highest detail) ... 5 (lowest).
dw->setLODCount(6);
dw->setDBMaxLOD(5);

// Optional key-value metadata; overwrite or delete by key.
dw->setMetaData("rig_version", "2.4.1");
dw->setMetaData("obsolete_key", nullptr);  // deletes the entry

dw->setDBComplexity("Face");
dw->setDBName("MetaHumans");
```

### Parameters

| Name | Type | Description |
|---|---|---|
| `name` *(setName)* | `const char*` | required — null-terminated character name; the string is copied (allocation occurs). |
| `archetype` | `Archetype` | required — enum value describing the character archetype. |
| `gender` | `Gender` | required — enum value describing the character gender. |
| `age` | `std::uint16_t` | required — character age in years. |
| `key` *(setMetaData)* | `const char*` | required — unique null-terminated key; duplicate calls overwrite the previous value. |
| `value` *(setMetaData)* | `const char*` | required — null-terminated metadata value; pass `nullptr` to delete the key. |
| `unit` *(setTranslationUnit)* | `TranslationUnit` | required — spatial unit for translation values (e.g., centimetres, metres). |
| `unit` *(setRotationUnit)* | `RotationUnit` | required — angular unit for rotation values (e.g., degrees, radians). |
| `system` | `CoordinateSystem` | required — handedness and up-axis convention for the rig. |
| `rotationSequence` | `RotationSequence` | required — Euler rotation application order (e.g., XYZ). |
| `rotationSign` | `const RotationSign&` | required — sign convention for rotation axes. |
| `faceWindingOrder` | `FaceWindingOrder` | required — vertex winding order for face geometry. |
| `lodCount` *(setLODCount)* | `std::uint16_t` | required — total number of LOD levels; level 0 is highest detail. |
| `lod` *(setDBMaxLOD)* | `std::uint16_t` | required — maximum LOD index stored in the DNA data for this character. |
| `name` *(setDBComplexity)* | `const char*` | required — null-terminated name of the input control interface; string is copied. |
| `name` *(setDBName)* | `const char*` | required — null-terminated name of the originating database; string is copied. |

### Watch out for

- Consecutive `setMetaData` calls with the same key overwrite existing data.
- Passing nullptr as the value removes the key.
- Strings are copied, which costs an additional allocation.
- Do not subclass `DescriptorWriter` directly — the class docstring states that implementors must inherit from `Writer` itself. Subclassing `DescriptorWriter` bypasses the full writer contract.
- Passing `nullptr` as the `value` argument to `setMetaData` deletes the associated key rather than storing a null value. This is a deletion shortcut, not a no-op.
- Every `const char*` name or value is copied internally, so the caller's buffer may be freed after the call — but each copy involves an allocation; avoid calling string setters in tight per-frame loops.
<!-- ink:api-end name="DescriptorWriter" -->
