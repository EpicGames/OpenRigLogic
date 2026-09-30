# API Reference — `dna/layers/JointBehaviorMetadataWriter`

---

<!-- ink:api name="JointBehaviorMetadataWriter" module="dna/layers/JointBehaviorMetadataWriter" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->

## `class DNAAPI JointBehaviorMetadataWriter : public virtual DefinitionWriter`

Set how each joint's translation, rotation and scale are represented internally when the rig is evaluated at runtime.

### Description
The representation metadata set through this interface is consumed by the evaluator implementation at rig-evaluation time. The choice of representation (e.g., quaternion vs. Euler for rotation) can affect numerical stability and performance of the downstream joint solver. This class exposes write-only setters; it extends `DefinitionWriter` and is part of the layered writer hierarchy under `Writer`.

### When to use this

Use this interface when you need to control the internal numeric representation used to evaluate individual joint TRS components at runtime — for example, to switch a joint's rotation from quaternion to Euler for a specific evaluator back-end. This is a write-only interface; pair it with `JointBehaviorMetadataReader` to read back previously set values.

### Example

```cpp
// writer is a dna::JointBehaviorMetadataWriter (for example a dna::Writer)
writer->clearJointRepresentations();
std::uint16_t jointIndex = 0;  // must be less than getJointCount()
writer->setJointTranslationRepresentation(jointIndex, translationRepresentation);
writer->setJointRotationRepresentation(jointIndex, rotationRepresentation);
writer->setJointScaleRepresentation(jointIndex, scaleRepresentation);
// the joint now evaluates using the chosen representations
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `jointIndex` | `std::uint16_t` | required. A joint's position in the zero-indexed array of joints. Must be less than the value returned by `getJointCount`. |
| `representation` | `TranslationRepresentation`, `RotationRepresentation` or `ScaleRepresentation` | required. The desired representation of the joint's translation, rotation or scale component, respectively. |

### Constraints

- `jointIndex` must be less than the value returned by `getJointCount`.
- `jointIndex` must be less than the value returned by `getJointCount()`. Passing an out-of-range index is undefined behavior per the `@warning` on each setter.
- Do not inherit directly from `JointBehaviorMetadataWriter`. Concrete implementations must inherit from `Writer`.

### Watch out for

- Implementors should inherit from `Writer` itself and not this class.
- This metadata affects how joints are calculated, as the given representation is used internally by the implementation that evaluates them.
- `clearJointRepresentations` deletes all representations.
- Implementors must inherit from `Writer`, not from this class directly. Inheriting from `JointBehaviorMetadataWriter` alone will result in an incomplete implementation that bypasses required Writer lifecycle machinery.
- Call `clearJointRepresentations()` before re-populating all joint representations to avoid stale entries from a prior configuration.

<!-- ink:api-end name="JointBehaviorMetadataWriter" -->
