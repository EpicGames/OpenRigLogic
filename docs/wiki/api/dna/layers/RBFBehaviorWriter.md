# API Reference — `dna/layers/RBFBehaviorWriter`

---

<!-- ink:api name="RBFBehaviorWriter" module="dna/layers/RBFBehaviorWriter" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class RBFBehaviorWriter : public virtual BehaviorWriter`

Write-only accessors for the RBF pose data associated with a rig — pose names, scale, control names, and the input/output control indices that connect poses to the rest of the rig.

### When to use this

Use this when building or editing a rig's RBF poses: naming poses and controls, setting pose scale, and wiring up which controls a pose reads from and which it drives. Implementors should inherit from `Writer` itself, not this class directly.

### Method groups

| Group | Methods |
|-------|---------|
| Clear | clearRBFPoses, clearRBFPoseControlNames |
| Pose identity | setRBFPoseName, setRBFPoseScale |
| Pose controls | setRBFPoseControlName, setRBFPoseInputControlIndices, setRBFPoseOutputControlIndices (list of control indices the pose drives) |

### Example

```cpp
// writer implements Writer, which composes RBFBehaviorWriter
writer->setRBFPoseName(0u, "smileWide");
writer->setRBFPoseScale(0u, 1.0f);
const std::uint16_t controlIndices[] = {2u, 5u};
writer->setRBFPoseInputControlIndices(0u, controlIndices, 2u);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `poseIndex` | `std::uint16_t` | required — position in the zero-indexed array of poses; storage resizes implicitly to fit. |
| `name` | `const char*` | required — null-terminated string; copied internally. |
| `scale` | `float` | required — the solver's pose scale factor. |
| `poseControlIndex` | `std::uint16_t` | required — position of the RBF pose control name; storage resizes implicitly to fit. |
| `controlIndices` | `const std::uint16_t*` | required — source address of control indices to copy. |
| `controlIndexCount` | `std::uint16_t` | required — number of control indices to copy. |

### Watch out for

- The final output weight of a pose is multiplied by the weights from `getRBFPoseOutputControlWeights` first, and can then optionally be further multiplied by the input control list set here — order matters when reasoning about the resulting weight.
- Storage arrays resize implicitly when writing to an index beyond the current size, but `setRBFPoseInputControlIndices`, `setRBFPoseOutputControlIndices`, `setRBFPoseOutputControlWeights`, `setRBFSolverRawControlIndices`, `setRBFSolverPoseIndices`, and `setRBFSolverRawControlValues` require `poseIndex` / `solverIndex` to already exist (must be less than the current pose/solver count). Create the pose or solver entry first before assigning its data arrays.
- `setRBFSolverRawControlIndices` expects the raw control count to match the distance method's arity: Quaternion, SwingAngle, and TwistAngle consume 4 raw controls per driving joint; Euclidean consumes 1 per scalar input.
- Names passed to `setRBFPoseName`, `setRBFPoseControlName`, and `setRBFSolverName` are copied on write — the caller may free the pointer immediately after the call returns.

<!-- ink:api-end name="RBFBehaviorWriter" -->
