# API Reference — `dna/Writer`

---

<!-- ink:api name="Reader" module="dna/Writer" last_commit="api_scan" updated="2026-07-31" api_kind="data_shape" -->

## `Reader`

A forward-declared reader type that `Writer::setFrom` accepts as the source when copying an entire DNA document into a writer.

### Why this exists

`Writer::setFrom` needs to call every getter on a `Reader` and forward each value into the matching setter on itself, but it only needs the `Reader` interface at the call site — so `Reader` is forward-declared here rather than included, keeping the writer header decoupled from the full reader hierarchy.

### Relationships

- `Writer` — *`setFrom(const Reader* source, ...)` copies all data from a `Reader` instance into the `Writer`, respecting the given `DataLayer` and `UnknownLayerPolicy`.*

<!-- ink:api-end name="Reader" -->

<!-- ink:api name="Writer" module="dna/Writer" last_commit="api_scan" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI Writer : public RBFBehaviorWriter, public GeometryWriter, public MachineLearnedBehaviorExtWriter, public JointBehaviorMetadataWriter, public TwistSwingBehaviorWriter`

Abstract unified write interface for all DNA data layers. Use this as the base type when you need to write a complete DNA asset — it combines geometry, behavior, machine-learned behavior, RBF, joint metadata, and twist/swing writing into a single entry point.

### When to use this

Reach for `Writer` when you need to mutate or construct a DNA asset. Use `setFrom` to bulk-copy an entire existing asset from a `Reader` — optionally restricting which layers are transferred via `DataLayer`. Note that the layer hierarchy cannot be written selectively at the class level; the unified interface is intentional.

### Example

```cpp
// Obtain a Writer instance (e.g., from BinaryStreamWriter factory)
dna::Writer* writer = dna::BinaryStreamWriter::create(stream, memRes);
const dna::Reader* reader = dna::BinaryStreamReader::create(inputStream, memRes);

// Copy all layers, preserving any unknown data
writer->setFrom(reader, dna::DataLayer::All, dna::UnknownLayerPolicy::Preserve);

// Copy only geometry, discarding unrecognized layers
writer->setFrom(reader, dna::DataLayer::Geometry, dna::UnknownLayerPolicy::Discard);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `source` | `const Reader*` | required — the DNA Reader to copy data from |
| `layer` | `DataLayer` | optional — limits which layers are transferred; defaults to `DataLayer::All` |
| `policy` | `UnknownLayerPolicy` | optional — whether unrecognized layers are preserved or discarded; defaults to `UnknownLayerPolicy::Preserve` |
| `memRes` | `MemoryResource*` | optional — memory resource for temporary allocations during the copy; defaults to `nullptr` |

### Watch out for

- The `Writer` class intentionally cannot write only specific layers — the unified interface mirrors the `Reader` hierarchy by design. Use `DataLayer` in `setFrom` to control which layers are *read from source*, not which are written.
- `setFrom` is implemented in the abstract base class itself and calls every getter on `source` then every matching setter on `this`. For large DNA assets, pass a `MemoryResource` to avoid heap fragmentation during the copy.

<!-- ink:api-end name="Writer" -->
