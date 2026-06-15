# API Reference — `dna/layers/TwistSwingBehaviorReader`

---

<!-- ink:api name="TwistSwingBehaviorReader" module="dna/layers/TwistSwingBehaviorReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TwistSwingBehaviorReader : public virtual DefinitionReader`

Read the twist and swing deformation parameters for a rig — the per-group axis, driver control indices, output joint indices, and blend weights for both motion types.

### When to use this

Use this interface when you need to inspect or serialize the twist/swing behavioral layer of a DNA asset. It pairs with `TwistSwingBehaviorWriter` for read/write access. Do not implement this interface directly — implementors should subclass `Reader` instead, as this class is an internal layer in the reader hierarchy.

### Method groups

| Group | Methods |
|-------|---------|
| Twist | `getTwistCount`, `getTwistSetupTwistAxis`, `getTwistInputControlIndices`, `getTwistOutputJointIndices`, `getTwistBlendWeights` |
| Swing | `getSwingCount`, `getSwingSetupTwistAxis`, `getSwingInputControlIndices`, `getSwingOutputJointIndices`, `getSwingBlendWeights` |

### Example

```cpp
// Iterate all twist groups and read their driver quaternion control indices
const uint16_t twistCount = reader->getTwistCount();
for (uint16_t i = 0u; i < twistCount; ++i) {
    TwistAxis axis = reader->getTwistSetupTwistAxis(i);
    // Always exactly 4 elements: quaternion [x, y, z, w]
    auto driverIndices = reader->getTwistInputControlIndices(i);
    auto outputJoints  = reader->getTwistOutputJointIndices(i);
    auto weights       = reader->getTwistBlendWeights(i);
}

// Iterate all swing groups symmetrically
const uint16_t swingCount = reader->getSwingCount();
for (uint16_t i = 0u; i < swingCount; ++i) {
    TwistAxis axis = reader->getSwingSetupTwistAxis(i);
    auto driverIndices = reader->getSwingInputControlIndices(i);
    auto outputJoints  = reader->getSwingOutputJointIndices(i);
    auto weights       = reader->getSwingBlendWeights(i);
}
```

### Returns

`ConstArrayView<T>` — a non-owning view over the underlying data. Do not store the view beyond the lifetime of the reader object.

### Watch out for

- All index parameters (`twistIndex`, `swingIndex`) must be strictly less than the value returned by `getTwistCount()` or `getSwingCount()` respectively. Passing an out-of-range index is undefined behavior — validate with the count getter before each call.
- `getTwistInputControlIndices` and `getSwingInputControlIndices` always return exactly 4 indices representing a single quaternion `[x, y, z, w]`. Do not treat the view length as variable.
- Do not subclass `TwistSwingBehaviorReader` directly. Implementors must subclass `Reader`; this class is an intermediate layer in the interface hierarchy.

<!-- ink:api-end name="TwistSwingBehaviorReader" -->
