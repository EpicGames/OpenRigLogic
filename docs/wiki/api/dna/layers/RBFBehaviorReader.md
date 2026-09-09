# API Reference — `dna/layers/RBFBehaviorReader`

---

<!-- ink:api name="RBFBehaviorReader" module="dna/layers/RBFBehaviorReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class RBFBehaviorReader : public virtual BehaviorReader`

Read-only accessors to the RBF (radial basis function) pose data associated with a rig — pose names, their driven joint/blend-shape/animated-map outputs, output values, scale, and the controls that feed them.

### When to use this

Use this when you need to inspect or evaluate a rig's RBF poses: which joints, blend shape channels, and animated maps each pose drives, what values it targets, and which input controls feed it. Implementors should inherit from `Reader` itself, not this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| Pose identity | getRBFPoseCount, getRBFPoseName |
| Pose outputs | getRBFPoseJointOutputIndices, getRBFPoseBlendShapeChannelOutputIndices, getRBFPoseAnimatedMapOutputIndices, getRBFPoseJointOutputValues, getRBFPoseScale |
| Pose controls | getRBFPoseControlCount, getRBFPoseControlName |

### Example

```cpp
// reader implements Reader, which composes RBFBehaviorReader
for (std::uint16_t poseIdx = 0u; poseIdx < reader->getRBFPoseCount(); ++poseIdx) {
    auto poseName = reader->getRBFPoseName(poseIdx);
    auto jointOutputs = reader->getRBFPoseJointOutputIndices(poseIdx);
    auto jointValues = reader->getRBFPoseJointOutputValues(poseIdx);
    auto scale = reader->getRBFPoseScale(poseIdx);
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `poseIndex` | `std::uint16_t` | required — position in the zero-indexed array of poses; must be less than `getRBFPoseCount`. |
| `poseControlIndex` | `std::uint16_t` | required — position in the zero-indexed array of RBF pose control names; must be less than `getRBFPoseControlCount`. |

### Returns

`std::uint16_t` / `StringView` / `ConstArrayView<std::uint16_t>` / `ConstArrayView<float>` / `float` — counts, names, index views, joint output values, or pose scale, depending on the accessor called.

### Constraints

- All `poseIndex` arguments must be less than `getRBFPoseCount()`; indices at or beyond this bound produce undefined behavior.

### Watch out for

- Do not subclass `RBFBehaviorReader` directly. Implementations must inherit from `Reader` itself, not from this interface — inheriting from `RBFBehaviorReader` bypasses required `Reader` infrastructure.

<!-- ink:api-end name="RBFBehaviorReader" -->
