# API Reference — `dna/layers/JointBehaviorMetadataWriter`

---

<!-- ink:api name="JointBehaviorMetadataWriter" module="dna/layers/JointBehaviorMetadataWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class JointBehaviorMetadataWriter : public virtual DefinitionWriter`

Configure per-joint internal representation metadata that controls how each joint's translation, rotation, and scale components are evaluated during rig execution.

### When to use this

Use this interface when you need to control the internal numeric representation used to evaluate individual joint TRS components at runtime — for example, to switch a joint's rotation from quaternion to Euler for a specific evaluator back-end. This is a write-only interface; pair it with `JointBehaviorMetadataReader` to read back previously set values.

### Example

```cpp
// writer is a concrete class that inherits from Writer (not from JointBehaviorMetadataWriter directly)
writer->clearJointRepresentations();

// Set each component's representation for joint at index 0
writer->setJointTranslationRepresentation(0, TranslationRepresentation::Vector3);
writer->setJointRotationRepresentation(0, RotationRepresentation::Quaternion);
writer->setJointScaleRepresentation(0, ScaleRepresentation::Vector3);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `jointIndex` | `std::uint16_t` | required — zero-based index of the target joint; must be less than `getJointCount()` |
| `representation` | `TranslationRepresentation` / `RotationRepresentation` / `ScaleRepresentation` | required — the desired internal representation for the joint's TRS component |

### Constraints

- `jointIndex` must be less than the value returned by `getJointCount()`. Passing an out-of-range index is undefined behavior per the `@warning` on each setter.
- Do not inherit directly from `JointBehaviorMetadataWriter`. Concrete implementations must inherit from `Writer`.

### Watch out for

- Implementors must inherit from `Writer`, not from this class directly. Inheriting from `JointBehaviorMetadataWriter` alone will result in an incomplete implementation that bypasses required Writer lifecycle machinery.
- Call `clearJointRepresentations()` before re-populating all joint representations to avoid stale entries from a prior configuration.

### Description

The representation metadata set through this interface is consumed by the evaluator implementation at rig-evaluation time. The choice of representation (e.g., quaternion vs. Euler for rotation) can affect numerical stability and performance of the downstream joint solver. This class exposes write-only setters; it extends `DefinitionWriter` and is part of the layered writer hierarchy under `Writer`.

<!-- ink:api-end name="JointBehaviorMetadataWriter" -->
