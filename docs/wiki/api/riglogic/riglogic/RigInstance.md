# API Reference — `riglogic/riglogic/RigInstance`

---

<!-- ink:api name="DefaultInstanceCreator" module="riglogic/riglogic/RigInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::DefaultInstanceCreator<rl4::RigInstance>`

Trait specialization that directs the `pma` memory-management framework to use `FactoryCreate<rl4::RigInstance>` as the creation policy for `RigInstance` smart-pointer types.

### Why this exists

The `pma` allocator framework requires a policy type to know how to construct managed objects. This specialization registers `RigInstance` with the factory-based creation strategy (`FactoryCreate`) so that `pma` smart pointers and scoped holders can allocate `RigInstance` objects through the existing `RigInstance::create` factory, rather than calling `new` directly. Without this specialization the framework would attempt direct construction, which bypasses the mandatory `RigLogic` binding.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<rl4::RigInstance>` | The creation policy type selected for `RigInstance` by the `pma` framework |

### Relationships

- `DefaultInstanceDestroyer<rl4::RigInstance>` — companion specialization that registers the corresponding destruction policy
- `RigInstance::create` — the factory function this policy ultimately delegates to

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="riglogic/riglogic/RigInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::DefaultInstanceDestroyer<rl4::RigInstance>`

Trait specialization that directs the `pma` memory-management framework to use `FactoryDestroy<rl4::RigInstance>` as the destruction policy for `RigInstance` smart-pointer types.

### Why this exists

Companion to `DefaultInstanceCreator<rl4::RigInstance>`. The `pma` framework requires a matched destruction policy so that scoped holders and smart pointers call `RigInstance::destroy` (rather than `delete`) when releasing an instance. This ensures the factory-managed lifecycle contract is honored symmetrically on both creation and destruction.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<rl4::RigInstance>` | The destruction policy type selected for `RigInstance` by the `pma` framework |

### Relationships

- `DefaultInstanceCreator<rl4::RigInstance>` — companion specialization that registers the creation policy
- `RigInstance::destroy` — the factory function this policy ultimately delegates to

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="RigInstance" module="riglogic/riglogic/RigInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class RLAPI rl4::RigInstance`

Holds all per-instance rig state — input control buffers, output joint/blend-shape/animated-map buffers, and the active LOD — for a single character instance driven by a shared `RigLogic` evaluator.

### When to use this

Use `RigInstance` when you need an independent slot of rig state for one character instance evaluated against a shared DNA. Multiple `RigInstance` objects can coexist for the same `RigLogic`, each with its own input/output buffers — for example, to drive several characters from the same rig definition in parallel without state aliasing. To apply rig evaluation, pass the instance to `RigLogic::calculate`.

### Method groups

| Group | Methods |
|-------|--------|
| Lifecycle | `create`, `destroy` |
| GUI controls | `getGUIControlCount`, `getGUIControl`, `setGUIControl`, `getGUIControlValues` |
| Raw controls | `getRawControlCount`, `getRawControl`, `setRawControl`, `getRawControlValues` |
| PSD controls | `getPSDControlCount`, `getPSDControl`, `getPSDControlValues` |
| ML controls | `getMLControlCount`, `getMLControl`, `getMLControlValues` |
| ML operations | `getMLTypeCount`, `getMLOperationSetCount`, `getMLOperationCount`, `getMLMaskValues` |
| RBF controls | `getRBFControlCount`, `getRBFControl`, `getRBFControlValues` |
| Joint outputs | `getJointOutputs` |
| Blend shape outputs | `getBlendShapeOutputs` |
| Animated map outputs | `getAnimatedMapOutputs` |
| LOD | `getLOD`, `setLOD` |

### Example

```cpp
// Create a RigInstance bound to an existing RigLogic evaluator
rl4::RigInstance* instance = rl4::RigInstance::create(rigLogic, memoryResource);

// Set active LOD before evaluation (0 = highest detail)
instance->setLOD(0);

// Write raw control values (one per input control in the DNA)
for (std::uint16_t i = 0; i < instance->getRawControlCount(); ++i) {
    instance->setRawControl(i, controlValues[i]);
}

// Evaluate the rig
rigLogic->calculate(instance);

// Read joint output values (packed flat array)
rl4::ConstArrayView<float> joints = instance->getJointOutputs();
rl4::ConstArrayView<float> blendShapes = instance->getBlendShapeOutputs();
rl4::ConstArrayView<float> animatedMaps = instance->getAnimatedMapOutputs();

// Release when done
rl4::RigInstance::destroy(instance);
```

### Parameters (create)

| Name | Type | Description |
|------|------|-------------|
| `rigLogic` | `RigLogic*` | required — the parent evaluator this instance will be bound to |
| `memRes` | `MemoryResource*` | optional — custom memory resource; pass `nullptr` to use the default allocator |

### Watch out for

- You are responsible for calling `destroy(instance)` when the instance is no longer needed. Skipping `destroy` leaks the instance's memory.
- An instance created through one `RigLogic` object cannot be passed to a different `RigLogic::calculate` call.
- `getJointOutputs()` returns a flat packed array of floats covering all joints across all LODs. The stride and offset per joint are determined by the DNA.

<!-- ink:api-end name="RigInstance" -->

<!-- ink:api name="RigLogic" module="riglogic/riglogic/RigInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class rl4::RigLogic`

Evaluates a `RigInstance` to produce joint, blend shape, and animated map output values. Pass a `RigInstance` to `RigLogic::calculate` to drive the rig for a given frame.

### When to use this

Reach for `RigLogic` when you need to apply rig evaluation to an instance — it is the computation engine that consumes input control values set on a `RigInstance` and writes calculated outputs back to that instance. A `RigInstance` cannot evaluate itself; it must be paired with the `RigLogic` it was created from.

### Watch out for

- A `RigInstance` created through one `RigLogic` object cannot be evaluated by a different `RigLogic` object. The binding between an instance and its parent evaluator is established at creation time via `RigInstance::create`.

<!-- ink:api-end name="RigLogic" -->

<!-- ink:api name="type" module="riglogic/riglogic/RigInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type` (trait member alias in `pma::DefaultInstanceCreator<rl4::RigInstance>` and `pma::DefaultInstanceDestroyer<rl4::RigInstance>`)

The conventional C++ traits `type` member that names the selected creation or destruction policy for `rl4::RigInstance` within the `pma` framework.

### Why this exists

C++ traits structs expose their result through a nested `type` member by convention (`std::type_traits` style). Both `DefaultInstanceCreator<rl4::RigInstance>::type` (`FactoryCreate<rl4::RigInstance>`) and `DefaultInstanceDestroyer<rl4::RigInstance>::type` (`FactoryDestroy<rl4::RigInstance>`) follow this pattern so the `pma` framework can uniformly extract the policy via `typename Traits::type`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` (creator) | `FactoryCreate<rl4::RigInstance>` | Creation policy; used by `pma` to allocate `RigInstance` via the factory |
| `type` (destroyer) | `FactoryDestroy<rl4::RigInstance>` | Destruction policy; used by `pma` to release `RigInstance` via the factory |

<!-- ink:api-end name="type" -->
