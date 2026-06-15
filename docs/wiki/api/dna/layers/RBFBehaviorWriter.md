# API Reference — `dna/layers/RBFBehaviorWriter`

---

<!-- ink:api name="RBFBehaviorWriter" module="dna/layers/RBFBehaviorWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI RBFBehaviorWriter : public virtual BehaviorWriter`

Write RBF solver and pose data into a rig's DNA — names, scale, control mappings, solver configuration, and LOD associations.

### When to use this

Use this interface when constructing or mutating the RBF behavior layer of a rig: populating poses, binding pose controls, configuring solvers, and mapping solvers to levels of detail. Do not inherit directly from this class — inherit from `Writer` instead, as noted in the class `@warning`.

### Method groups

| Group | Methods |
|---|---|
| Poses | `clearRBFPoses`, `setRBFPoseName`, `setRBFPoseScale` |
| Pose controls | `clearRBFPoseControlNames`, `setRBFPoseControlName`, `setRBFPoseInputControlIndices`, `setRBFPoseOutputControlIndices`, `setRBFPoseOutputControlWeights` |
| Solvers | `clearRBFSolvers`, `clearRBFSolverIndices`, `setRBFSolverIndices`, `setRBFSolverName`, `setRBFSolverRawControlIndices`, `setRBFSolverPoseIndices`, `setRBFSolverRawControlValues` |
| Solver configuration | `setRBFSolverType`, `setRBFSolverRadius`, `setRBFSolverAutomaticRadius`, `setRBFSolverWeightThreshold`, `setRBFSolverDistanceMethod`, `setRBFSolverNormalizeMethod`, `setRBFSolverFunctionType`, `setRBFSolverTwistAxis` |
| LOD mapping | `clearLODRBFSolverMappings`, `setLODRBFSolverMapping` |

### Example

```cpp
// Populate one pose and one solver, then map to LOD 0
RBFBehaviorWriter* writer = ...; // obtained via a Writer subclass

// Pose
writer->setRBFPoseName(0, "jaw_open_pose");
writer->setRBFPoseScale(0, 1.0f);

// Pose controls — inputs that multiply the pose output, and outputs the pose drives
const uint16_t inputControls[] = {11, 12, 13, 14};
writer->setRBFPoseInputControlIndices(0, inputControls, 4);
const uint16_t outputControls[] = {7};
writer->setRBFPoseOutputControlIndices(0, outputControls, 1);
const float outputWeights[] = {0.85f};
writer->setRBFPoseOutputControlWeights(0, outputWeights, 1);

// Solver
writer->setRBFSolverName(0, "jaw_rbf_solver");
writer->setRBFSolverType(0, RBFSolverType::RBF);
writer->setRBFSolverDistanceMethod(0, RBFDistanceMethod::Quaternion);
writer->setRBFSolverNormalizeMethod(0, RBFNormalizeMethod::Softmax);
writer->setRBFSolverRadius(0, 0.5f);
writer->setRBFSolverWeightThreshold(0, 0.01f);
const uint16_t rawControls[] = {11, 12, 13, 14};
writer->setRBFSolverRawControlIndices(0, rawControls, 4);
const uint16_t poseIndices[] = {0};
writer->setRBFSolverPoseIndices(0, poseIndices, 1);

// LOD mapping — associate solver index list 0 with LOD 0
const uint16_t solverIdx[] = {0};
writer->setRBFSolverIndices(0, solverIdx, 1);
writer->setLODRBFSolverMapping(0, 0);
```

### Watch out for

- Storage arrays resize implicitly when writing to an index beyond the current size, but `setRBFPoseInputControlIndices`, `setRBFPoseOutputControlIndices`, `setRBFPoseOutputControlWeights`, `setRBFSolverRawControlIndices`, `setRBFSolverPoseIndices`, and `setRBFSolverRawControlValues` require `poseIndex` / `solverIndex` to already exist (must be less than the current pose/solver count). Create the pose or solver entry first before assigning its data arrays.
- `setRBFSolverRawControlIndices` expects the raw control count to match the distance method's arity: Quaternion, SwingAngle, and TwistAngle consume 4 raw controls per driving joint; Euclidean consumes 1 per scalar input.
- Names passed to `setRBFPoseName`, `setRBFPoseControlName`, and `setRBFSolverName` are copied on write — the caller may free the pointer immediately after the call returns.

<!-- ink:api-end name="RBFBehaviorWriter" -->
