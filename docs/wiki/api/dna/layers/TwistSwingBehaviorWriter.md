# API Reference — `dna/layers/TwistSwingBehaviorWriter`

---

<!-- ink:api name="TwistSwingBehaviorWriter" module="dna/layers/TwistSwingBehaviorWriter" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class TwistSwingBehaviorWriter : public virtual DefinitionWriter`

Write-only accessors to the swing and twist data associated with a rig — defining twist/swing parameter groups, their axis, driver control indices, driven joints, and blend weights.

### When to use this

Use this when authoring or editing twist/swing rigging: setting up which joint rotation drives a twist chain, which joints it outputs to, and how the effect is blended across them. Implementors should inherit from `Writer` itself, not this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| Twist | clearTwists, deleteTwist, setTwistSetupTwistAxis, setTwistInputControlIndices, setTwistOutputJointIndices, setTwistBlendWeights |
| Swing | clearSwings, deleteSwing, setSwingSetupTwistAxis, setSwingInputControlIndices, setSwingOutputJointIndices, setSwingBlendWeights |

### Example

```cpp
// writer implements Writer, which composes TwistSwingBehaviorWriter
writer->setTwistSetupTwistAxis(0u, dna::TwistAxis::X);
const std::uint16_t controlIndices[] = {0u, 1u, 2u, 3u}; // quaternion x,y,z,w
writer->setTwistInputControlIndices(0u, controlIndices, 4u);
const std::uint16_t jointIndices[] = {10u, 11u};
writer->setTwistOutputJointIndices(0u, jointIndices, 2u);
const float blendWeights[] = {1.0f, 0.5f};
writer->setTwistBlendWeights(0u, blendWeights, 2u);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `twistIndex` | `std::uint16_t` | required — position of the twist parameter group; storage resizes implicitly to fit. |
| `twistAxis` | `TwistAxis` | required — the twist axis of the parameter group. |
| `controlIndices` | `const std::uint16_t*` | required — pointer to raw control indices representing the driver joint rotation; always 4 values (quaternion x,y,z,w). |
| `controlIndexCount` | `std::uint16_t` | required — number of raw control indices to copy. |
| `jointIndices` | `const std::uint16_t*` | required — pointer to joint indices of the twist-driven joints. |
| `jointIndexCount` | `std::uint16_t` | required — number of joint indices to copy. |
| `blendWeights` | `const float*` | required — source address of twist blend weights to copy; one weight per output joint. |
| `blendWeightCount` | `std::uint16_t` | required — number of blend weights to copy. |

### Watch out for

- `deleteTwist` removes a specific parameter group by index — distinct from `clearTwists`, which removes all of them.
- Parameter group storage is implicitly resized to fit whatever index is written to, so groups can be set out of order.
- Inherit from `Writer`, not directly from `TwistSwingBehaviorWriter`. Direct inheritance violates the intended interface hierarchy.
- Passing `controlIndexCount != 4` for quaternion input methods may produce undefined results — the driver joint rotation is always encoded as a single quaternion.

### Constraints

- Blend weight arrays must contain one entry per output joint in the corresponding joint index array.

<!-- ink:api-end name="TwistSwingBehaviorWriter" -->
