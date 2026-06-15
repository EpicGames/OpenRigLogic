# API Reference — `riglogic/riglogic/RigLogic`

---

<!-- ink:api name="Configuration" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Configuration`

Controls which algorithm implementation `RigLogic` uses for rig evaluation and which submodules are loaded into memory.

### Why this exists

Separating configuration from the `RigLogic` instance allows callers to tune memory allocation and computation strategy at construction time without changing the evaluation API. The alias re-exports `rl4::Configuration` so callers do not need to include a separate header for the most common construction path.

### Relationships

- `RigLogic::create` — accepts `Configuration` as its second parameter
- `RigLogic::getConfiguration` — returns the `Configuration` active on a live instance
- `rl4::Configuration` — the authoritative definition; this is a type alias

<!-- ink:api-end name="Configuration" -->

<!-- ink:api name="DefaultInstanceCreator" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<rl4::RigLogic>`

Template specialization that wires `rl4::RigLogic` into the `pma` smart-pointer and factory infrastructure using `FactoryCreate` as the creation strategy.

### Why this exists

The `pma` allocator framework uses `DefaultInstanceCreator` and `DefaultInstanceDestroyer` trait types to decouple object construction from ownership management. By specializing these traits for `rl4::RigLogic`, the library allows `pma`-managed handles to call the correct factory entry points (`RigLogic::create` / `RigLogic::destroy`) automatically, rather than requiring every call site to manage raw pointer lifetime manually.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<rl4::RigLogic>` | The creation-strategy type used by `pma` machinery to construct `RigLogic` instances |

### Relationships

- `DefaultInstanceDestroyer<rl4::RigLogic>` — paired destroyer specialization
- `RigLogic::create` — the underlying factory function invoked via this trait

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<rl4::RigLogic>`

Template specialization that wires `rl4::RigLogic` into the `pma` ownership infrastructure using `FactoryDestroy` as the destruction strategy.

### Why this exists

Paired with `DefaultInstanceCreator<rl4::RigLogic>`, this specialization ensures that `pma`-managed handles release `RigLogic` instances through the correct `RigLogic::destroy` factory path rather than a direct `delete`. This enforces the ownership contract documented on `RigLogic::create`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<rl4::RigLogic>` | The destruction-strategy type invoked by `pma` machinery when a managed `RigLogic` handle goes out of scope |

### Relationships

- `DefaultInstanceCreator<rl4::RigLogic>` — paired creator specialization
- `RigLogic::destroy` — the underlying factory function invoked via this trait

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="RigInstance" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RigInstance`

Holds the per-rig evaluation state driven by a `RigLogic` instance.

### Why this exists

`RigLogic` is intentionally stateless so that one solver instance can drive many characters simultaneously. `RigInstance` is the counterpart that carries the per-character data (control values, output buffers) that must remain isolated between rigs. By separating these concerns, the same compiled rig logic can be shared across threads without locking.

### Relationships

- `RigLogic` — creates and evaluates `RigInstance` objects; call `RigLogic::create` first

<!-- ink:api-end name="RigInstance" -->

<!-- ink:api name="RigLogic" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class RigLogic`

Evaluate rig output values from input control values. A single instance is stateless and can drive any number of `RigInstance` objects concurrently.

### When to use this

Use `RigLogic` when you need to evaluate a character rig at runtime — translating animator control values into joint transforms, blend shapes, and corrective outputs. Because `RigLogic` carries no per-character mutable state, it is safe to share one instance across threads, each operating on a distinct `RigInstance`.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | `create`, `destroy`, `restore`, `dump` |
| Configuration | `getConfiguration` |
| LOD | `getLODCount`, `getRBFSolverIndicesForLOD`, `getMLIndicesForLOD` |

### Example

```cpp
// Create RigLogic from a DNA reader
rl4::Configuration config{};
rl4::RigLogic* rl = rl4::RigLogic::create(reader, config, memRes);

// Query LOD count and get solver indices for LOD 0
std::uint16_t lodCount = rl->getLODCount();
auto rbfIndices = rl->getRBFSolverIndicesForLOD(0);
auto mlIndices  = rl->getMLIndicesForLOD(0);

// Snapshot state for fast re-instantiation later
rl->dump(outputStream);

// Release when done
rl4::RigLogic::destroy(rl);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `reader` | `const dna::Reader*` | required — source from which DNA data is copied and optimized for rig evaluation |
| `config` | `const Configuration&` | optional — selects algorithm implementation and controls which submodules are loaded; defaults to `{}` |
| `memRes` | `MemoryResource*` | optional — custom allocator; if `nullptr`, a default allocator is used |

### Watch out for

- Ownership of the returned pointer belongs to the caller. You must call `RigLogic::destroy(instance)` when done; deleting the pointer directly is undefined.
- `restore` skips the storage-optimization phase that `create` performs. Use `restore` when startup time matters and you have a previously `dump`ed stream; use `create` when loading a DNA for the first time.

<!-- ink:api-end name="RigLogic" -->

<!-- ink:api name="type" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type`

Type alias member within the `DefaultInstanceCreator` and `DefaultInstanceDestroyer` specializations for `rl4::RigLogic`; resolves to the factory strategy type used by the `pma` ownership infrastructure.

### Relationships

- `DefaultInstanceCreator<rl4::RigLogic>` — defines `type = FactoryCreate<rl4::RigLogic>`
- `DefaultInstanceDestroyer<rl4::RigLogic>` — defines `type = FactoryDestroy<rl4::RigLogic>`

<!-- ink:api-end name="type" -->
