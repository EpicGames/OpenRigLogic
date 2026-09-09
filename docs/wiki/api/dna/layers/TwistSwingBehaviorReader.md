# API Reference — `dna/layers/TwistSwingBehaviorReader`

---

<!-- ink:api name="TwistSwingBehaviorReader" module="dna/layers/TwistSwingBehaviorReader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TwistSwingBehaviorReader : public virtual DefinitionReader`

Read-only accessors to the swing and twist data associated with a rig — which joint rotations drive twist/swing transformations, which joints they output to, and the blend weights applied.

### When to use this

Use this when evaluating how a driver joint's rotation should propagate twist or swing onto other joints — for example, distributing a forearm's twist along multiple joints in a chain. Implementors should inherit from `Reader` itself, not this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| Twist | getTwistCount, getTwistSetupTwistAxis, getTwistInputControlIndices, getTwistOutputJointIndices, getTwistBlendWeights |
| Swing | getSwingCount, getSwingSetupTwistAxis, getSwingInputControlIndices, getSwingOutputJointIndices (and corresponding blend weights) |

### Example

```cpp
// reader implements Reader, which composes TwistSwingBehaviorReader
for (std::uint16_t twistIdx = 0u; twistIdx < reader->getTwistCount(); ++twistIdx) {
    auto axis = reader->getTwistSetupTwistAxis(twistIdx);
    auto drivenJoints = reader->getTwistOutputJointIndices(twistIdx);
    auto weights = reader->getTwistBlendWeights(twistIdx);
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `twistIndex` | `std::uint16_t` | required — position of the twist parameter group; must be less than `getTwistCount`. |
| `swingIndex` | `std::uint16_t` | required — position of the swing parameter group; must be less than `getSwingCount`. |

### Returns

`std::uint16_t` / `TwistAxis` / `ConstArrayView<std::uint16_t>` / `ConstArrayView<float>` — counts, axis, driver/driven joint index views, or blend weights, depending on the accessor called.

### Watch out for

- The raw control indices representing the driver joint rotation are always exactly 4 values, corresponding to a single quaternion's [x, y, z, w] attributes.
- There is exactly one blend weight per output joint in the returned list — the two lists are parallel and must be indexed together.
- All index parameters (`twistIndex`, `swingIndex`) must be strictly less than the value returned by `getTwistCount()` or `getSwingCount()` respectively. Passing an out-of-range index is undefined behavior — validate with the count getter before each call.
- Do not subclass `TwistSwingBehaviorReader` directly. Implementors must subclass `Reader`; this class is an intermediate layer in the interface hierarchy.

<!-- ink:api-end name="TwistSwingBehaviorReader" -->
