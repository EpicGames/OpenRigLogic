# API Reference — `dna/layers/MachineLearnedBehaviorExt`

---

<!-- ink:api name="MachineLearnedBehaviorOperationType" module="dna/layers/MachineLearnedBehaviorExt" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `MachineLearnedBehaviorOperationType`

Enumerates the kind of computation a single ML operation performs within a neural network layer.

### Why this exists

Neural network evaluation in a rig is expressed as a sequence of typed operations rather than a fixed pipeline, so each operation needs a tag describing what it does before its parameters can be interpreted. This lets `MachineLearnedBehaviorReader`/`Writer` store heterogeneous operation kinds (data movement and arithmetic) in the same operation list.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Unspecified` | enumerator | No operation type has been assigned. |
| `Gather` | enumerator | Collects values from a source into a new arrangement. |
| `Scatter` | enumerator | Writes values into a destination at computed positions. |
| `MLP` | enumerator | Evaluates a multi-layer perceptron sub-network. |
| `WeightedSum` | enumerator | Combines inputs using per-input weights. |

### Relationships

- `MachineLearnedBehaviorParameterKey` — *identifies which static parameter is being read for an operation of this type.*
- `MachineLearnedBehaviorExtReader::getMLOperationType` — *returns this type for a given operation.*
- `MachineLearnedBehaviorExtWriter::setMLOperationType` — *assigns this type to a given operation.*

<!-- ink:api-end name="MachineLearnedBehaviorOperationType" -->

<!-- ink:api name="MachineLearnedBehaviorParameterKey" module="dna/layers/MachineLearnedBehaviorExt" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `MachineLearnedBehaviorParameterKey`

Enumerates the static parameter keys an ML operation can reference, covering joint transform types, coordinate axes, rotation signs, and rotation sequence.

### Why this exists

Operations such as `Gather` or `Scatter` need to know which semantic joint attribute a static parameter refers to (translation vs. rotation vs. scale, or which axis and sign convention applies) without hard-coding that meaning into the operation itself. This key lets the same operation types be reused across joints with different coordinate-system conventions.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `JointTranslationType` | enumerator | Selects the joint's translation component. |
| `JointRotationType` | enumerator | Selects the joint's rotation component. |
| `JointScaleType` | enumerator | Selects the joint's scale component. |
| `JointCoordinateSystemAxisX` | enumerator | Selects the joint coordinate system's X axis definition. |
| `JointCoordinateSystemAxisY` | enumerator | Selects the joint coordinate system's Y axis definition. |
| `JointCoordinateSystemAxisZ` | enumerator | Selects the joint coordinate system's Z axis definition. |
| `JointRotationSignAxisX` | enumerator | Selects the sign convention for rotation around X. |
| `JointRotationSignAxisY` | enumerator | Selects the sign convention for rotation around Y. |
| `JointRotationSignAxisZ` | enumerator | Selects the sign convention for rotation around Z. |
| `JointRotationSequence` | enumerator | Selects the joint's rotation application order. |

### Relationships

- `MachineLearnedBehaviorOperationType` — *the operation kind that consumes parameters keyed by this enum.*
- `MachineLearnedBehaviorExtReader::getMLOperationParameters` — *returns values indexed using these keys.*

<!-- ink:api-end name="MachineLearnedBehaviorParameterKey" -->
