# API Reference — `dna/Writer`

---

<!-- ink:api name="Reader" module="dna/Writer" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class Reader;`

Forward declaration of the DNA `Reader` type, used by `Writer::setFrom` as the source to copy data from.

### Why this exists
`Writer::setFrom` needs to call every getter on a `Reader` and forward each value into the matching setter on itself, but it only needs the `Reader` interface at the call site — so `Reader` is forward-declared here rather than included, keeping the writer header decoupled from the full reader hierarchy.

### When to use this

Pass a `Reader` pointer to `Writer::setFrom` when you want to copy DNA content into a Writer. The Writer hierarchy mirrors the structure of the Reader hierarchy.

### Example

```cpp
// `reader` is a dna::Reader, `writer` is a dna::Writer implementation
writer->setFrom(reader, dna::DataLayer::All, dna::UnknownLayerPolicy::Preserve);
```

### Watch out for

- `Writer.h` only forward-declares `Reader`. Include the Reader header where you need its full definition.
<!-- ink:api-end name="Reader" -->

<!-- ink:api name="Writer" module="dna/Writer" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI Writer : public RBFBehaviorWriter, public GeometryWriter, public MachineLearnedBehaviorExtWriter, public JointBehaviorMetadataWriter, public TwistSwingBehaviorWriter`

Write every DNA layer through one interface, and clone an existing Reader into it with `setFrom`.

Abstract unified write interface for all DNA data layers. Use this as the base type when you need to write a complete DNA asset — it combines geometry, behavior, machine-learned behavior, RBF, joint metadata, and twist/swing writing into a single entry point.

### When to use this

Reach for `Writer` when you need to mutate or construct a DNA asset. Use `setFrom` to bulk-copy an entire existing asset from a `Reader` — optionally restricting which layers are transferred via `DataLayer`. Note that the layer hierarchy cannot be written selectively at the class level; the unified interface is intentional.

Implementors should inherit from `Writer`, not from the individual layer writers. The layers are combined because, as with the Reader hierarchy, it is not possible to selectively write only specific layers.

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

`setFrom(const Reader* source, DataLayer layer = DataLayer::All, UnknownLayerPolicy policy = UnknownLayerPolicy::Preserve, MemoryResource* memRes = nullptr)`

| Name | Type | Description |
|------|------|-------------|
| `source` | `const Reader*` | required. The source DNA Reader from which the data is copied. |
| `layer` | `DataLayer` | optional, defaults to `DataLayer::All`. Limits which layers are taken over from the source. |
| `policy` | `UnknownLayerPolicy` | optional, defaults to `UnknownLayerPolicy::Preserve`. Whether unknown layers are preserved or ignored. |
| `memRes` | `MemoryResource*` | optional, defaults to `nullptr`. Memory resource for temporary allocations during copying. |

### Description

`setFrom` copies all data from the given Reader by calling each Reader getter and passing the return values to the matching Writer setters. It is implemented in the abstract class itself, so it works for all DNA Writers.

### Watch out for

- `setFrom` is implemented in the abstract base class itself and calls every getter on `source` then every matching setter on `this`. For large DNA assets, pass a `MemoryResource` to avoid heap fragmentation during the copy.

<!-- ink:api-end name="Writer" -->
