# API Reference — `dna/layers/TwistSwingBehaviorWriter`

---

<!-- ink:api name="TwistSwingBehaviorWriter" module="dna/layers/TwistSwingBehaviorWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI TwistSwingBehaviorWriter : public virtual DefinitionWriter`

Define how swing and twist deformation data is written into a rig's DNA. Implement this interface (via `Writer`) to supply or update the quaternion-driven joint rotation parameters that control twist and swing bone chains.

### When to use this

Reach for this interface when authoring or replacing the twist/swing deformation setup for a rig — setting driver quaternion inputs, driven joint indices, and per-joint blend weights. If you are only reading existing data, use the corresponding reader interface instead.

### Method groups

| Group | Methods |
|-------|---------|
| Twist management | `clearTwists`, `deleteTwist` |
| Twist setup | `setTwistSetupTwistAxis`, `setTwistInputControlIndices`, `setTwistOutputJointIndices`, `setTwistBlendWeights` |
| Swing management | `clearSwings`, `deleteSwing` |
| Swing setup | `setSwingSetupTwistAxis`, `setSwingInputControlIndices`, `setSwingOutputJointIndices`, `setSwingBlendWeights` |

### Example

```cpp
// Configure twist group at index 0: Y-axis, single quaternion driver, two driven joints
writer->setTwistSetupTwistAxis(0, TwistAxis::Y);

const std::uint16_t twistControls[] = {12, 13, 14, 15};  // quaternion [x,y,z,w] control indices
writer->setTwistInputControlIndices(0, twistControls, 4);

const std::uint16_t twistJoints[] = {3, 7};
writer->setTwistOutputJointIndices(0, twistJoints, 2);

const float twistWeights[] = {0.5f, 1.0f};  // one weight per driven joint
writer->setTwistBlendWeights(0, twistWeights, 2);

// Mirror configuration for a swing group
writer->setSwingSetupTwistAxis(0, TwistAxis::Y);
const std::uint16_t swingControls[] = {16, 17, 18, 19};
writer->setSwingInputControlIndices(0, swingControls, 4);
```

### Constraints

- Control indices for both twist and swing inputs are always exactly 4 values, representing a single quaternion `[x, y, z, w]`.
- Blend weight arrays must contain one entry per output joint in the corresponding joint index array.
- Parameter group storage is implicitly resized when a group index beyond the current count is written — no explicit resize call is required.

### Watch out for

- Inherit from `Writer`, not directly from `TwistSwingBehaviorWriter`. Direct inheritance violates the intended interface hierarchy.
- Passing `controlIndexCount != 4` for quaternion input methods may produce undefined results — the driver joint rotation is always encoded as a single quaternion.

<!-- ink:api-end name="TwistSwingBehaviorWriter" -->
