# API Reference — `dna/layers/JointBehaviorMetadataReader`

---

<!-- ink:api name="JointBehaviorMetadataReader" module="dna/layers/JointBehaviorMetadataReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `JointBehaviorMetadataReader`

Query the internal representation hints that govern how each joint's translation, rotation, and scale components are evaluated during rig execution.

### When to use this

Reach for this interface when you need to read the representation contract for specific joints — for example, to mirror the rig runtime's evaluation strategy in a custom solver or to validate that DNA joint metadata matches your expected configuration. Use the paired writer interface when you need to assign these values rather than read them.

### Method groups

| Group | Methods |
|---|---|
| Component representation | `getJointTranslationRepresentation`, `getJointRotationRepresentation`, `getJointScaleRepresentation` |

### Example

```cpp
// reader is a fully loaded Reader* obtained from BinaryStreamReader::create()
const JointBehaviorMetadataReader* meta = reader;

std::uint16_t jointIndex = 0u;  // must be < reader->getJointCount()

TranslationRepresentation  txRep  = meta->getJointTranslationRepresentation(jointIndex);
RotationRepresentation     rotRep = meta->getJointRotationRepresentation(jointIndex);
ScaleRepresentation        sclRep = meta->getJointScaleRepresentation(jointIndex);
// txRep, rotRep, sclRep now carry the representation enums used by the rig evaluator
```

### Parameters

| Name | Type | Description |
|---|---|---|
| `jointIndex` | `std::uint16_t` | required — zero-based position of the joint in the joint array; must be less than `getJointCount()` |

### Returns

- `getJointTranslationRepresentation` → `TranslationRepresentation` — the internal representation used for this joint's translation component during evaluation.
- `getJointRotationRepresentation` → `RotationRepresentation` — the internal representation used for this joint's rotation component during evaluation.
- `getJointScaleRepresentation` → `ScaleRepresentation` — the internal representation used for this joint's scale component during evaluation.

### Watch out for

- `jointIndex` must be strictly less than `getJointCount()`. The interface does not guard against out-of-range indices; passing an invalid index produces undefined behavior.
- Do not subclass `JointBehaviorMetadataReader` directly. Implementations must inherit from `Reader` itself; this class is an accessor mixin only.

<!-- ink:api-end name="JointBehaviorMetadataReader" -->
