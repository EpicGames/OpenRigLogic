# API Reference — `riglogic/riglogic/RigInstance`

---

<!-- ink:api name="DefaultInstanceCreator" module="riglogic/riglogic/RigInstance" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceCreator`

A `pma` trait specialization that tells the memory management utilities how to construct a `RigInstance` by default.

### Why this exists

`pma`'s managed-instance utilities need a uniform way to create and destroy instances of arbitrary types without every caller specifying the creation strategy explicitly. Specializing `DefaultInstanceCreator<rl4::RigInstance>` to use `FactoryCreate<rl4::RigInstance>` routes construction through `RigInstance`'s own factory method instead of a raw constructor call, respecting its custom allocation and lifetime rules.

### Relationships

- `RigInstance` — *the type this specialization creates, via `RigInstance::create`.*
- `DefaultInstanceDestroyer` — *the paired trait that destroys the same type.*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="riglogic/riglogic/RigInstance" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceDestroyer`

A `pma` trait specialization that tells the memory management utilities how to destroy a `RigInstance` by default.

### Why this exists

Pairing with `DefaultInstanceCreator`, this specialization routes destruction of a `RigInstance` through `FactoryDestroy<rl4::RigInstance>`, which calls `RigInstance::destroy` rather than a raw `delete`, since the destructor is protected and lifetime is factory-managed.

### Relationships

- `RigInstance` — *the type this specialization destroys, via `RigInstance::destroy`.*
- `DefaultInstanceCreator` — *the paired trait that creates the same type.*

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="RigInstance" module="riglogic/riglogic/RigInstance" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class RLAPI RigInstance`

Holds the instance-specific input and output buffers for evaluating one instance of a rig, including level-of-detail and control values.

### When to use this

Create a `RigInstance` whenever you need to drive and evaluate a specific instance of a rig — for example, one character on screen. Multiple `RigInstance` objects can be created from the same `RigLogic` instance, since `RigLogic` itself is stateless, but a `RigInstance` created through one `RigLogic` instance cannot be evaluated by a different `RigLogic` instance.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | create, destroy |
| GUI controls | getGUIControlCount, getGUIControl, setGUIControl, getGUIControlValues |
| Raw controls | getRawControlCount, getRawControl, setRawControl, getRawControlValues |
| PSD controls | getPSDControlCount, getPSDControl, getPSDControlValues |
| ML controls | getMLControlCount, getMLControl, getMLControlValues, getMLTypeCount, getMLOperationSetCount, getMLOperationCount, getMLMaskValues |
| RBF controls | getRBFControlCount, getRBFControl, getRBFControlValues |
| Outputs | getJointOutputs, getBlendShapeOutputs |

### Example

```cpp
rl4::RigInstance* instance = rl4::RigInstance::create(rigLogic);
instance->setGUIControl(0, 0.75f);
auto joints = instance->getJointOutputs();
rl4::RigInstance::destroy(instance);
```

### Raises

- Passing a `RigInstance` created from one `RigLogic` instance to another `RigLogic` instance for evaluation is unsupported — keep each `RigInstance` paired with the `RigLogic` instance that created it.

### Watch out for

- The destructor is protected — instances must be released through `RigInstance::destroy`, not `delete`.
- You are responsible for calling `destroy(instance)` when the instance is no longer needed. Skipping `destroy` leaks the instance's memory.
- `getJointOutputs()` returns a flat packed array of floats covering all joints across all LODs. The stride and offset per joint are determined by the DNA.

<!-- ink:api-end name="RigInstance" -->

<!-- ink:api name="RigLogic" module="riglogic/riglogic/RigInstance" last_commit="api_scan" updated="2026-06-10" api_kind="callable" -->

## `class rl4::RigLogic`

Evaluates a `RigInstance` to produce joint, blend shape, and animated map output values. Pass a `RigInstance` to `RigLogic::calculate` to drive the rig for a given frame.

### When to use this

Reach for `RigLogic` when you need to apply rig evaluation to an instance — it is the computation engine that consumes input control values set on a `RigInstance` and writes calculated outputs back to that instance. A `RigInstance` cannot evaluate itself; it must be paired with the `RigLogic` it was created from.

### Watch out for

- A `RigInstance` created through one `RigLogic` object cannot be evaluated by a different `RigLogic` object. The binding between an instance and its parent evaluator is established at creation time via `RigInstance::create`.

<!-- ink:api-end name="RigLogic" -->
