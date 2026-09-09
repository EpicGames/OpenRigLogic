# API Reference — `dna/Reader`

---

<!-- ink:api name="Reader" module="dna/Reader" last_commit="api_scan" updated="2026-09-09" api_kind="callable" cpp_abstract_class="true" -->

## `class Reader : public RBFBehaviorReader, public GeometryReader, public MachineLearnedBehaviorExtReader, public JointBehaviorMetadataReader, public TwistSwingBehaviorReader`

Aggregate reader interface combining every DNA layer's reader interface into one type.

### When to use this

Use `Reader` when you want a single interface that exposes every loadable layer (RBF behavior, geometry, machine-learned behavior, joint behavior metadata, twist/swing behavior) rather than composing the layer interfaces yourself. Because layers can be selectively loaded, it can be convenient to slice off just the individual layer interfaces you need instead of holding the full `Reader`.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | unload |

### Example

```cpp
Reader* reader = /* obtained from a concrete StreamReader */;
reader->unload(DataLayer::Geometry);  // unloads Geometry and any layer depending on it
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `layer` | `DataLayer` | required — layer whose data should be unloaded, along with all layers dependent on it |

### Watch out for

- The artificial separation into multiple interfaces mirrors the DNA file's own layer structure; since layers can be selectively loaded, code that only needs one layer should consider using that layer's narrower interface instead of the full `Reader`.
- `unload` propagates to dependent layers. Unloading an upstream layer will also drop all layers that require it, which may be more than expected.

<!-- ink:api-end name="Reader" -->
