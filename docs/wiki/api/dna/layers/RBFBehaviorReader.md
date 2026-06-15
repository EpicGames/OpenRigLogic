# API Reference — `dna/layers/RBFBehaviorReader`

---

<!-- ink:api name="RBFBehaviorReader" module="dna/layers/RBFBehaviorReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class RBFBehaviorReader : public virtual BehaviorReader`

Query all RBF (Radial Basis Function) pose and control data associated with a rig. Use this interface when you need to read RBF solver inputs — pose names, joint output indices, blend shape channel indices, animated map indices, joint output values, scale factors, and control names — without modifying them.

### When to use this

Reach for `RBFBehaviorReader` when you need read-only access to a rig's RBF behavior layer — for example, to inspect pose configurations, iterate over RBF controls, or extract solver outputs for downstream processing. For write operations, use the paired `RBFBehaviorWriter` interface instead.

### Method groups

| Group | Methods |
|-------|---------|
| RBF Poses | `getRBFPoseCount`, `getRBFPoseName`, `getRBFPoseJointOutputIndices`, `getRBFPoseBlendShapeChannelOutputIndices`, `getRBFPoseAnimatedMapOutputIndices`, `getRBFPoseJointOutputValues`, `getRBFPoseScale` |
| RBF Pose Controls | `getRBFPoseControlCount`, `getRBFPoseControlName` |

### Example

```cpp
// reader is a concrete Reader implementation (not RBFBehaviorReader directly)
const std::uint16_t poseCount = reader->getRBFPoseCount();
for (std::uint16_t i = 0u; i < poseCount; ++i) {
    StringView poseName = reader->getRBFPoseName(i);
    float scale = reader->getRBFPoseScale(i);
    ConstArrayView<std::uint16_t> jointIndices = reader->getRBFPoseJointOutputIndices(i);
    ConstArrayView<float> jointValues = reader->getRBFPoseJointOutputValues(i);
    // process pose outputs...
}

const std::uint16_t controlCount = reader->getRBFPoseControlCount();
for (std::uint16_t c = 0u; c < controlCount; ++c) {
    StringView controlName = reader->getRBFPoseControlName(c);
    // enumerate distinct RBF controls...
}
```

### Constraints

- All `poseIndex` arguments must be less than `getRBFPoseCount()`; indices at or beyond this bound produce undefined behavior.
- All `poseControlIndex` arguments must be less than `getRBFPoseControlCount()`.
- `getRBFPoseControlCount()` returns the number of *distinct* output control indices across all RBF poses — not the sum of per-pose control counts.

### Watch out for

- Do not subclass `RBFBehaviorReader` directly. Implementations must inherit from `Reader` itself, not from this interface — inheriting from `RBFBehaviorReader` bypasses required `Reader` infrastructure.
- `getRBFPoseControlCount()` counts unique controls only. This is distinct from the per-pose index arrays returned by `getRBFPoseBlendShapeChannelOutputIndices`, `getRBFPoseJointOutputIndices`, and `getRBFPoseAnimatedMapOutputIndices`, which may reference the same control index in multiple poses.

<!-- ink:api-end name="RBFBehaviorReader" -->
