# API Reference — `dna/layers/MachineLearnedBehaviorExt`

---

<!-- ink:api name="MachineLearnedBehaviorOperationType" module="dna/layers/MachineLearnedBehaviorExt" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `MachineLearnedBehaviorOperationType`

Tag the type of operation a machine-learned behavior node performs within the DNA rig evaluation graph.

### Why this exists

ML behavior evaluation in a DNA rig is decomposed into discrete typed steps — gather inputs, run inference, scatter outputs, combine results — rather than a single monolithic operation. This enum makes the step type explicit so the evaluator can dispatch each node to the correct kernel without runtime type inspection. Using a typed enum over a raw integer or string prevents mis-dispatch and makes the operation pipeline readable at a glance.

### Fields

| Name | Type | Description |
|---|---|---|
| `Unspecified` | `MachineLearnedBehaviorOperationType` | Default sentinel; operation type has not been assigned. Treat a node with this value as misconfigured. |
| `Gather` | `MachineLearnedBehaviorOperationType` | Reads input values from a source buffer and feeds them into the ML node. |
| `Scatter` | `MachineLearnedBehaviorOperationType` | Writes output values produced by the ML node back to a target buffer. |
| `MLP` | `MachineLearnedBehaviorOperationType` | Executes a multi-layer perceptron inference pass on the gathered inputs. |
| `WeightedSum` | `MachineLearnedBehaviorOperationType` | Combines multiple input contributions using per-weight summation. |

### Relationships

- `MachineLearnedBehaviorParameterKey` — *key enum used to configure parameters on the same ML behavior node*

<!-- ink:api-end name="MachineLearnedBehaviorOperationType" -->

<!-- ink:api name="MachineLearnedBehaviorParameterKey" module="dna/layers/MachineLearnedBehaviorExt" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `MachineLearnedBehaviorParameterKey`

Identify which joint-space configuration parameter to read or write on a machine-learned behavior node.

### Why this exists

ML behavior nodes drive joint transforms, and those transforms depend on coordinate conventions that vary per rig and per engine — rotation representation, axis orientation, sign handedness, and Euler order. Rather than passing configuration as untyped key strings or raw indices, this enum makes every configurable aspect a named constant. Callers pass a `MachineLearnedBehaviorParameterKey` value to a parameter accessor, and the type system prevents passing an invalid key at compile time.

### Fields

| Name | Type | Description |
|---|---|---|
| `JointTranslationType` | `MachineLearnedBehaviorParameterKey` | Specifies the translation convention the joint uses. |
| `JointRotationType` | `MachineLearnedBehaviorParameterKey` | Specifies the rotation representation (e.g. quaternion, Euler angles). |
| `JointScaleType` | `MachineLearnedBehaviorParameterKey` | Specifies the scale convention the joint uses. |
| `JointCoordinateSystemAxisX` | `MachineLearnedBehaviorParameterKey` | Maps the joint's local X axis to a world-space axis. |
| `JointCoordinateSystemAxisY` | `MachineLearnedBehaviorParameterKey` | Maps the joint's local Y axis to a world-space axis. |
| `JointCoordinateSystemAxisZ` | `MachineLearnedBehaviorParameterKey` | Maps the joint's local Z axis to a world-space axis. |
| `JointRotationSignAxisX` | `MachineLearnedBehaviorParameterKey` | Sign correction applied to rotation about the joint's X axis. |
| `JointRotationSignAxisY` | `MachineLearnedBehaviorParameterKey` | Sign correction applied to rotation about the joint's Y axis. |
| `JointRotationSignAxisZ` | `MachineLearnedBehaviorParameterKey` | Sign correction applied to rotation about the joint's Z axis. |
| `JointRotationSequence` | `MachineLearnedBehaviorParameterKey` | Euler rotation application order (e.g. XYZ, ZYX) for this joint. |

### Relationships

- `MachineLearnedBehaviorOperationType` — *operation-type enum for the same ML behavior node*

<!-- ink:api-end name="MachineLearnedBehaviorParameterKey" -->
