# API Reference — `dna/Reader`

---

<!-- ink:api name="Reader" module="dna/Reader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class Reader : public RBFBehaviorReader, public GeometryReader, public MachineLearnedBehaviorExtReader, public JointBehaviorMetadataReader, public TwistSwingBehaviorReader`

Unified DNA read interface that composes all layer-specific reader interfaces into a single entry point, with support for selectively unloading layers.

### When to use this

Use `Reader` when you need to read DNA data across multiple layers — geometry, behavior, machine-learned behavior, joint metadata, and twist/swing — through one coherent handle. Because DNA files are structured in independently loadable layers, `Reader` also lets you unload a layer and all layers that depend on it once you no longer need them, reducing memory overhead in layer-selective workflows.

### Example

```cpp
// Obtain a Reader instance via a concrete factory (e.g. BinaryStreamReader)
// Then selectively release layers no longer needed:
reader->unload(DataLayer::Geometry);  // frees Geometry data and any dependent layers
```

### Methods

| Name | Signature | Description |
|------|-----------|-------------|
| `unload` | `void unload(DataLayer layer)` | Unloads all data for the given layer and every layer that depends on it. |

### Watch out for

- `Reader` inherits from five separate layer-reader interfaces (`RBFBehaviorReader`, `GeometryReader`, `MachineLearnedBehaviorExtReader`, `JointBehaviorMetadataReader`, `TwistSwingBehaviorReader`). The separation mirrors the DNA file's own layer structure. Holding only a sub-interface pointer (e.g. `GeometryReader*`) after calling `unload(DataLayer::Geometry)` is intentional — it makes the reduced capability explicit at the type level.
- `unload` propagates to dependent layers. Unloading an upstream layer will also drop all layers that require it, which may be more than expected.

<!-- ink:api-end name="Reader" -->
