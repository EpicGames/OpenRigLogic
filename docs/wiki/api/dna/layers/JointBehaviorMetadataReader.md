# API Reference — `dna/layers/JointBehaviorMetadataReader`

---

<!-- ink:api name="JointBehaviorMetadataReader" module="dna/layers/JointBehaviorMetadataReader" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->

## `class DNAAPI JointBehaviorMetadataReader : public virtual DefinitionReader`

Read how each joint's translation, rotation and scale are represented internally when the rig is evaluated at runtime.

### When to use this

Reach for this interface when you need to read the representation contract for specific joints — for example, to mirror the rig runtime's evaluation strategy in a custom solver or to validate that DNA joint metadata matches your expected configuration. Use the paired writer interface when you need to assign these values rather than read them.

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
|------|------|-------------|
| `jointIndex` | `std::uint16_t` | required. A joint's position in the zero-indexed array of joints. Must be less than the value returned by `getJointCount`. |

### Returns

- `getJointTranslationRepresentation` → `TranslationRepresentation` — the internal representation used for this joint's translation component during evaluation.
- `getJointRotationRepresentation` → `RotationRepresentation` — the internal representation used for this joint's rotation component during evaluation.
- `getJointScaleRepresentation` → `ScaleRepresentation` — the internal representation used for this joint's scale component during evaluation.

### Constraints

- `jointIndex` must be less than the value returned by `getJointCount`.

### Watch out for

- Implementors should inherit from `Reader` itself and not this class.
- This metadata affects how joints are calculated, as the given representation is used internally by the implementation that evaluates them.
- `jointIndex` must be strictly less than `getJointCount()`. The interface does not guard against out-of-range indices; passing an invalid index produces undefined behavior.
- Do not subclass `JointBehaviorMetadataReader` directly. Implementations must inherit from `Reader` itself; this class is an accessor mixin only.

<!-- ink:api-end name="JointBehaviorMetadataReader" -->
