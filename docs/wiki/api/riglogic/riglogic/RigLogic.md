# API Reference — `riglogic/riglogic/RigLogic`

---

<!-- ink:api name="Configuration" module="riglogic/riglogic/RigLogic" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RigLogic::Configuration`

An alias, inside `RigLogic`, for `rl4::Configuration` — the options struct used to create and inspect a `RigLogic` instance.

### Relationships

- `Configuration` (riglogic/riglogic/Configuration) — *the underlying type this alias refers to.*
- `RigLogic` — *`RigLogic::create` accepts one, and `RigLogic::getConfiguration` returns one.*

<!-- ink:api-end name="Configuration" -->

<!-- ink:api name="DefaultInstanceCreator" module="riglogic/riglogic/RigLogic" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceCreator`

A `pma` trait specialization that tells the memory management utilities how to construct a `RigLogic` by default.

### Why this exists

Like the `RigInstance` specialization, this routes construction of `RigLogic` through `FactoryCreate<rl4::RigLogic>`, which calls `RigLogic::create` rather than a raw constructor, respecting the custom allocation strategy and DNA-loading initialization `RigLogic` requires.

### Relationships

- `RigLogic` — *the type this specialization creates, via `RigLogic::create`.*
- `DefaultInstanceDestroyer` — *the paired trait that destroys the same type.*

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="riglogic/riglogic/RigLogic" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceDestroyer`

A `pma` trait specialization that tells the memory management utilities how to destroy a `RigLogic` by default.

### Why this exists

Pairs with `DefaultInstanceCreator` to route destruction of `RigLogic` through `FactoryDestroy<rl4::RigLogic>`, which calls `RigLogic::destroy` rather than a raw `delete`, since the destructor is protected.

### Relationships

- `RigLogic` — *the type this specialization destroys, via `RigLogic::destroy`.*
- `DefaultInstanceCreator` — *the paired trait that creates the same type.*

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="RigInstance" module="riglogic/riglogic/RigLogic" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class RigInstance` (forward declaration)

Forward-declared here as the counterpart type that `RigLogic` creates and evaluates instances of; see the `RigInstance` entry in `riglogic/riglogic/RigInstance` for its full interface.

<!-- ink:api-end name="RigInstance" -->

<!-- ink:api name="RigLogic" module="riglogic/riglogic/RigLogic" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class RLAPI RigLogic`

Loads and optimizes DNA rig data, then calculates rig output values for any number of `RigInstance` objects based on their input control values.

### When to use this

Create a single `RigLogic` instance per DNA/rig definition, then create one `RigInstance` per character instance that needs evaluating. Because `RigLogic` holds no instance-specific state, it is safe to share and use from multiple threads concurrently to evaluate different `RigInstance` objects.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | create, destroy, restore, dump |
| Configuration | getConfiguration |
| LOD | getLODCount, getRBFSolverIndicesForLOD |

### Example

```cpp
rl4::Configuration config;
rl4::RigLogic* rigLogic = rl4::RigLogic::create(dnaReader, config);
rl4::RigInstance* instance = rl4::RigInstance::create(rigLogic);
// ... evaluate instance ...
rl4::RigLogic::destroy(rigLogic);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `reader` | `const dna::Reader*` | Source from which to copy and optimize DNA data used for rig evaluation. |
| `config` | `const Configuration&` | optional. Determines which algorithm implementation is used and which submodules to load; defaults to `{}`. |
| `memRes` | `MemoryResource*` | optional. Custom memory resource for allocations; if not given, a default allocation mechanism is used. |

### Returns

`RigLogic*` — A newly created `RigLogic` instance; the caller is responsible for releasing it via `destroy`.

### Watch out for

- The destructor is protected — instances must be released through `RigLogic::destroy`, not `delete`.
- `restore` loads a previously `dump`ed state and is faster than `create` because it skips the storage optimization phase.
- Ownership of the returned pointer belongs to the caller. You must call `RigLogic::destroy(instance)` when done; deleting the pointer directly is undefined.
- `restore` skips the storage-optimization phase that `create` performs. Use `restore` when startup time matters and you have a previously `dump`ed stream; use `create` when loading a DNA for the first time.

<!-- ink:api-end name="RigLogic" -->
