# API Reference — `dna/Configuration`

---

<!-- ink:api name="Configuration" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `Configuration`

Bundles all the load-time options for reading a DNA file — which layers, which LODs, and which coordinate/unit conventions to apply.

### Why this exists

Reader construction had a growing list of independent load options (layer selection, LOD filtering, coordinate transform policy). `Configuration` collects them into one struct with sensible defaults so callers only need to set the fields relevant to their use case, instead of threading many separate parameters through reader APIs.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `layer` | `DataLayer` | optional — layer up to which data is loaded; defaults to `DataLayer::All` |
| `unknownLayerPolicy` | `UnknownLayerPolicy` | optional — whether unknown layers are preserved or ignored; defaults to `Preserve` |
| `lods` | `ConstArrayView<std::uint16_t>` | optional — exact LODs to load; when used, `maxLOD`/`minLOD` are ignored. All values must be less than the value returned by `getLODCount` |
| `maxLOD` | `std::uint16_t` | optional — maximum level of detail to load; must be less than `getLODCount` |
| `minLOD` | `std::uint16_t` | optional — minimum level of detail to load; must be less than `getLODCount` |
| `coordinateSystemTransformPolicy` | `CoordinateSystemTransformPolicy` | optional — whether to convert to `coordinateSystem`; defaults to `Preserve` |
| `coordinateSystem` | `CoordinateSystem` | optional — destination axis directions used when transforming |
| `rotationSequence` | `RotationSequence` | optional — global rotation composition order; defaults to `xyz` |
| `rotationSign` | `RotationSign` | optional — per-axis rotation sign convention; defaults to all-positive |
| `faceWindingOrder` | `FaceWindingOrder` | optional — target face winding order; defaults to `ccw` |

### Construction

```cpp
Configuration config;
config.layer = DataLayer::Geometry;
config.maxLOD = 0;
config.minLOD = 3;
config.coordinateSystemTransformPolicy = CoordinateSystemTransformPolicy::Transform;
```

### Constraints

- Using `lods` causes `maxLOD`/`minLOD` to be ignored.
- All values in `lods`, and both `maxLOD` and `minLOD`, must be less than `getLODCount`.

<!-- ink:api-end name="Configuration" -->

<!-- ink:api name="CoordinateSystem" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `CoordinateSystem`

Alias for `tdm::coord_sys`, describing the axis directions for all coordinate axes.

### Why this exists

`CoordinateSystem` makes the target coordinate convention explicit and inspectable, rather than baking an implicit convention into conversion code. `Configuration::coordinateSystem` uses it as the destination system when `coordinateSystemTransformPolicy` is set to `Transform`.

### Relationships

- `Configuration` — *the `coordinateSystem` field specifies the destination convention*
- `CoordinateSystemTransformPolicy` — *governs whether conversion to this system happens*
- `Direction` — *the per-axis component type*

<!-- ink:api-end name="CoordinateSystem" -->

<!-- ink:api name="CoordinateSystemTransformPolicy" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `CoordinateSystemTransformPolicy`

Controls whether loaded rig data is converted into a destination coordinate system or left as authored.

### Why this exists

Rigs can be authored in different coordinate conventions than the engine consuming them expects. `Preserve` performs no conversion at all, while `Transform` converts to the `Configuration::coordinateSystem` unless the data is already in that system — avoiding redundant transforms when the source already matches.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Preserve` | `CoordinateSystemTransformPolicy` | perform no coordinate conversion |
| `Transform` | `CoordinateSystemTransformPolicy` | convert to `Configuration::coordinateSystem` unless already matching |

### Relationships

- `Configuration` — *the `coordinateSystemTransformPolicy` field controls this behavior*
- `CoordinateSystem` — *the destination system used when transforming*

<!-- ink:api-end name="CoordinateSystemTransformPolicy" -->

<!-- ink:api name="DataLayer" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DataLayer`

Bitmask enum identifying the loadable layers of a DNA rig, from the lightweight `Descriptor` up through `All`.

### Why this exists

DNA files are structured in dependent layers — geometry and behavior data implicitly require the definition layer to make sense, for instance. `DataLayer` encodes those dependencies directly in the enum values (each higher layer ORs in the layers it depends on) so that requesting `Geometry` automatically pulls in `Definition` without the caller having to enumerate every prerequisite.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Descriptor` | `DataLayer` | base metadata layer |
| `Definition` | `DataLayer` | rig topology/definition; implicitly loads `Descriptor` |
| `Behavior` | `DataLayer` | rig behavior; implicitly loads `Definition` |
| `Geometry` | `DataLayer` | mesh geometry; implicitly loads `Definition` |
| `GeometryWithoutBlendShapes` | `DataLayer` | mesh geometry excluding blend shapes; implicitly loads `Definition` |
| `MachineLearnedBehavior` | `DataLayer` | ML behavior data; implicitly loads `Definition` |
| `RBFBehavior` | `DataLayer` | RBF behavior data; implicitly loads `Behavior` |
| `JointBehaviorMetadata` | `DataLayer` | joint behavior metadata; implicitly loads `Definition` |
| `TwistSwingBehavior` | `DataLayer` | twist/swing behavior; implicitly loads `Definition` |
| `All` | `DataLayer` | union of every layer |

### Construction

```cpp
Configuration config;
config.layer = DataLayer::Geometry;  // also implicitly loads Definition
```

### Relationships

- `Configuration` — *stores the `DataLayer` value selecting how much of the DNA to load*
- `Reader::unload` — *takes a `DataLayer` to unload a layer and everything dependent on it*

### Constraints

- Each enumerator encodes its own dependencies via bitwise OR; requesting a higher-level layer automatically satisfies lower-level requirements at the reader level.
- `All` does not include `GeometryWithoutBlendShapes` — that value is mutually exclusive with `Geometry`.

<!-- ink:api-end name="DataLayer" -->

<!-- ink:api name="Direction" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `Direction`

Alias for `tdm::axis_dir`, representing a signed direction along a coordinate axis.

### Relationships

- `CoordinateSystem` — *composed of `Direction` values, one per axis*
- `RotationDirection` — *related axis-direction alias used for rotation sign*

<!-- ink:api-end name="Direction" -->

<!-- ink:api name="FaceWindingOrder" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `FaceWindingOrder`

Face vertex winding order of the geometry data, viewed along the outward surface normal.

### Why this exists

Different renderers and DCC tools expect different winding conventions. Authored DNAs use CCW (right-handed), while some engines expect CW (left-handed, DirectX-style). Converters normalize geometry to this value when `CoordinateSystemTransformPolicy::Transform` is active, so downstream consumers get consistently wound faces regardless of source convention.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `ccw` | `FaceWindingOrder` | counter-clockwise; cross product of consecutive face vertices agrees with stored normals |
| `cw` | `FaceWindingOrder` | clockwise; cross product opposes stored normals (left-handed/DirectX style) |

### Relationships

- `Configuration` — *the `faceWindingOrder` field selects the target winding order*
- `CoordinateSystemTransformPolicy` — *winding normalization only occurs when set to `Transform`*

### Constraints

- Winding conversion is applied only when `Configuration::coordinateSystemTransformPolicy` is `Transform`. Setting `faceWindingOrder` alone under `Preserve` policy has no effect.

<!-- ink:api-end name="FaceWindingOrder" -->

<!-- ink:api name="RotationDirection" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RotationDirection`

Alias for `tdm::rot_dir`, representing the sign of rotation (positive/negative) about an axis.

### Relationships

- `RotationSign` — *a per-axis triple of `RotationDirection` values*
- `Direction` — *related axis-direction alias used for translation*

<!-- ink:api-end name="RotationDirection" -->

<!-- ink:api name="RotationSequence" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RotationSequence`

Alias for `tdm::rot_seq`, representing the order in which axis rotations are composed (e.g. XYZ).

### Relationships

- `Configuration` — *the `rotationSequence` field defaults to `RotationSequence::xyz`*

<!-- ink:api-end name="RotationSequence" -->

<!-- ink:api name="RotationSign" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RotationSign`

Alias for `tdm::rot_sign`, a per-axis triple of `RotationDirection` values specifying the sign convention for rotation on each axis.

### Construction

```cpp
Configuration config;
config.rotationSign = {RotationDirection::positive, RotationDirection::positive, RotationDirection::positive};
```

### Relationships

- `RotationDirection` — *the per-axis component type*
- `Configuration` — *the `rotationSign` field applies this convention globally*

<!-- ink:api-end name="RotationSign" -->

<!-- ink:api name="RotationUnit" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RotationUnit`

Unit of measurement (degrees or radians) used for rotation values in the rig.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `degrees` | `RotationUnit` | degrees |
| `radians` | `RotationUnit` | radians |

<!-- ink:api-end name="RotationUnit" -->

<!-- ink:api name="TranslationUnit" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `TranslationUnit`

Unit of measurement (centimeters or meters) used for translation values in the rig.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `cm` | `TranslationUnit` | centimeters |
| `m` | `TranslationUnit` | meters |

<!-- ink:api-end name="TranslationUnit" -->

<!-- ink:api name="UnknownLayerPolicy" module="dna/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `UnknownLayerPolicy`

Controls whether layers the reader/writer doesn't recognize are kept or discarded.

### Why this exists

DNA files can be produced by newer tooling that adds layers this version of the library doesn't understand. `UnknownLayerPolicy` lets callers decide whether to round-trip that unknown data untouched (`Preserve`) or drop it (`Ignore`), which matters for forward-compatibility when re-serializing a file.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Preserve` | `UnknownLayerPolicy` | keep unrecognized layer data as-is |
| `Ignore` | `UnknownLayerPolicy` | discard unrecognized layer data |

### Construction

```cpp
Configuration config;
config.unknownLayerPolicy = UnknownLayerPolicy::Preserve;
```

<!-- ink:api-end name="UnknownLayerPolicy" -->
