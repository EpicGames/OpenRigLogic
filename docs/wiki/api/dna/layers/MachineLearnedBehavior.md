# API Reference — `dna/layers/MachineLearnedBehavior`

---

<!-- ink:api name="ActivationFunction" module="dna/layers/MachineLearnedBehavior" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ActivationFunction`

Specify the nonlinear activation function applied at each neuron layer in a machine-learned rig behavior network.

### Why this exists

Activation functions are a configuration concern embedded in DNA files — they must survive serialization, versioning, and cross-platform loading. Using a scoped enum rather than a raw integer or string makes every assignment compiler-checked and ensures that adding a new function in a future DNA version is a source-visible change. The five enumerators cover all activation functions supported by the RigLogic ML backend.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `linear` | `ActivationFunction` | Identity pass-through; output equals input. Suited for output layers predicting unbounded values. |
| `relu` | `ActivationFunction` | Rectified linear unit; clamps negative inputs to zero. Standard hidden-layer choice for feedforward rig networks. |
| `leakyrelu` | `ActivationFunction` | Leaky ReLU; passes a small negative slope instead of hard-clamping negatives. Avoids dead-neuron issues that can occur with `relu`. |
| `tanh` | `ActivationFunction` | Hyperbolic tangent; squashes output to (−1, 1). Use for layers whose outputs should fall in a bounded symmetric range. |
| `sigmoid` | `ActivationFunction` | Logistic sigmoid; squashes output to (0, 1). Use for output layers producing probabilities or normalized blend weights. |

### Construction

```cpp
// Assign when configuring an ML behavior layer descriptor
dna::ActivationFunction activation = dna::ActivationFunction::relu;
```

### Relationships

- `MachineLearnedBehavior` — the ML behavior layer type that consumes this enum to configure its neuron activation

<!-- ink:api-end name="ActivationFunction" -->
