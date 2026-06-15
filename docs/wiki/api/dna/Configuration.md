# API Reference — `dna/Configuration`

---

<!-- ink:api name="Configuration" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Configuration`

Aggregate load-time options for a DNA reader — controls which layers load, which LODs are included, and how coordinates are converted.

### Why this exists

DNA assets can contain many optional data layers and multiple levels of detail. Without a configuration object, every call site would need to pass a growing list of independent parameters, and cross-field constraints (e.g., `lods` overrides `maxLOD`/`minLOD`) would be invisible. `Configuration` gathers all read-time options into one zero-argument-constructible struct whose defaults (load all layers, all LODs, preserve transforms) are safe for the common case. Pass a customised instance when you need partial loads or coordinate conversion.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `layer` | `DataLayer` | optional — which data layers to load; defaults to `DataLayer::All` |
| `unknownLayerPolicy` | `UnknownLayerPolicy` | optional — whether to preserve or discard unrecognised layer data; defaults to `Preserve` |
| `lods` | `ConstArrayView<uint16_t>` | optional — explicit list of LOD indices to load; when non-empty, overrides `maxLOD`/`minLOD`; all values must be less than `getLODCount()` |
| `maxLOD` | `uint16_t` | optional — highest-detail LOD index to load (0 = most detailed); defaults to `0`; ignored when `lods` is non-empty; must be less than `getLODCount()` |
| `minLOD` | `uint16_t` | optional — lowest-detail LOD index to load; defaults to `UINT16_MAX` (load all); ignored when `lods` is non-empty; must be less than `getLODCount()` |
| `coordinateSystemTransformPolicy` | `CoordinateSystemTransformPolicy` | optional — whether to convert coordinates to `coordinateSystem` at load time; defaults to `Preserve` |
| `coordinateSystem` | `CoordinateSystem` | optional — target coordinate frame; only active when policy is `Transform` |
| `rotationSequence` | `RotationSequence` | optional — Euler rotation order applied globally during conversion; defaults to `xyz` |
| `rotationSign` | `RotationSign` | optional — per-axis rotation sign during conversion; defaults to all-positive |
| `faceWindingOrder` | `FaceWindingOrder` | optional — target face winding; normalised to `ccw` during conversion; defaults to `ccw` |

### Construction

```cpp
// Default: load everything, no coordinate conversion
dna::Configuration cfg;

// Load only descriptor and joint definition data for a preview
dna::Configuration preview;
preview.layer = dna::DataLayer::Definition;

// Load specific LODs 0 and 2 only
std::array<std::uint16_t, 2> lodList = {0, 2};
dna::Configuration lodCfg;
lodCfg.lods = dna::ConstArrayView<std::uint16_t>{lodList.data(), lodList.size()};

// Load all layers, convert to a right-handed Y-up coordinate system
dna::Configuration engineCfg;
engineCfg.coordinateSystemTransformPolicy = dna::CoordinateSystemTransformPolicy::Transform;
engineCfg.coordinateSystem = dna::CoordinateSystem{/* tdm::coord_sys for target engine */};
```

### Constraints

- When `lods` is non-empty, the values set for `maxLOD` and `minLOD` are ignored entirely.
- All values in `lods` must be less than the value returned by `getLODCount()` on the reader; violation is undefined behaviour.
- `maxLOD` and `minLOD` must each be less than `getLODCount()` when `lods` is empty.
- Coordinate conversion (rotation, winding, system) applies only when `coordinateSystemTransformPolicy == Transform`.

### Relationships

- `DataLayer` — selects which data layers to load
- `UnknownLayerPolicy` — controls handling of unrecognised layers
- `CoordinateSystemTransformPolicy` / `CoordinateSystem` — coordinate conversion settings
- `FaceWindingOrder` — face winding normalisation during conversion

<!-- ink:api-end name="Configuration" -->

<!-- ink:api name="CoordinateSystem" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `CoordinateSystem`

Complete spatial coordinate frame type from TDM, aliased into the `dna` namespace.

### Why this exists

`CoordinateSystem` is `tdm::coord_sys` re-exported under `dna::`. It encapsulates the full description of a spatial frame — axis directions, handedness, and up convention — as a typed object rather than a loose collection of booleans or an implicit enum. Using a typed coordinate frame prevents silent mismatches where a right-handed Y-up frame is accidentally interpreted as a left-handed Z-up frame. Set `Configuration::coordinateSystem` to the target frame, then set `coordinateSystemTransformPolicy = Transform` to activate conversion at load time.

### Relationships

- `Direction` — axis direction values used to build a `CoordinateSystem`
- `Configuration` — the `coordinateSystem` field holds the target frame
- `CoordinateSystemTransformPolicy` — must be `Transform` for the frame to be applied

<!-- ink:api-end name="CoordinateSystem" -->

<!-- ink:api name="CoordinateSystemTransformPolicy" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `CoordinateSystemTransformPolicy`

Decide whether the reader converts loaded data into a target coordinate system or passes it through unchanged.

### Why this exists

DNA assets are authored in a specific coordinate frame, but downstream engines (Unreal, Maya, custom renderers) may use different conventions. `Transform` enables in-place conversion at load time so the rest of the pipeline can assume a consistent frame. `Preserve` is the safe default for read-back fidelity — use it when the consuming code handles its own coordinate transforms, or when round-tripping a file back to disk without modification.

### Fields

| Name | Description |
|------|-------------|
| `Preserve` | Pass coordinate data through without any conversion. Default. |
| `Transform` | Convert to the coordinate system specified in `Configuration::coordinateSystem` at load time, unless the data is already in that system. |

### Relationships

- `Configuration` — set via `coordinateSystemTransformPolicy`; works with `coordinateSystem`, `rotationSequence`, `rotationSign`, and `faceWindingOrder` fields
- `CoordinateSystem` — the target frame used when policy is `Transform`
- `FaceWindingOrder` — face winding is normalised to `ccw` when policy is `Transform`

<!-- ink:api-end name="CoordinateSystemTransformPolicy" -->

<!-- ink:api name="DataLayer" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DataLayer`

Select which data layers to load from a DNA asset — pay only for the data your pipeline needs.

### Why this exists

Loading an entire DNA asset when only descriptor or joint data is needed wastes memory and I/O time. `DataLayer` is a bitmask enum that lets callers request exactly the layers they need, while encoding implicit load-order dependencies directly in the enumerator values — `Behavior = 4 | Definition` makes it self-evident that loading behaviour also loads definition. The `operator|` overload lets you compose custom subsets without casting.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Descriptor` | `uint32_t = 1` | optional — base identity and metadata only; no joint or geometry data |
| `Definition` | `uint32_t = 2 \| Descriptor` | optional — joint definitions; implicitly loads `Descriptor` |
| `Behavior` | `uint32_t = 4 \| Definition` | optional — joint behaviour data; implicitly loads `Definition` |
| `Geometry` | `uint32_t = 8 \| Definition` | optional — full mesh geometry including blend shapes; implicitly loads `Definition` |
| `GeometryWithoutBlendShapes` | `uint32_t = 16 \| Definition` | optional — mesh geometry without blend shapes; implicitly loads `Definition` |
| `MachineLearnedBehavior` | `uint32_t = 32 \| Definition` | optional — ML-driven behaviour layer; implicitly loads `Definition` |
| `RBFBehavior` | `uint32_t = 64 \| Behavior` | optional — RBF behaviour layer; implicitly loads `Behavior` (and thus `Definition`) |
| `JointBehaviorMetadata` | `uint32_t = 128 \| Definition` | optional — per-joint behaviour metadata; implicitly loads `Definition` |
| `TwistSwingBehavior` | `uint32_t = 256 \| Definition` | optional — twist/swing constraint behaviour; implicitly loads `Definition` |
| `All` | `uint32_t` | optional — composite of `RBFBehavior`, `Geometry`, `MachineLearnedBehavior`, `JointBehaviorMetadata`, and `TwistSwingBehavior`; the default for `Configuration::layer` |

### Construction

```cpp
// Load only joint behaviour (implicitly includes Definition + Descriptor)
dna::Configuration cfg;
cfg.layer = dna::DataLayer::Behavior;

// Load behaviour and geometry together via operator|
cfg.layer = dna::DataLayer::Behavior | dna::DataLayer::Geometry;

// Load everything (default)
cfg.layer = dna::DataLayer::All;
```

### Relationships

- `Configuration` — the `layer` field holds a `DataLayer` value
- `UnknownLayerPolicy` — companion enum controlling what happens to unrecognised layer data

### Constraints

- Each enumerator encodes its own dependencies via bitwise OR; requesting a higher-level layer automatically satisfies lower-level requirements at the reader level.
- `All` does not include `GeometryWithoutBlendShapes` — that value is mutually exclusive with `Geometry`.

<!-- ink:api-end name="DataLayer" -->

<!-- ink:api name="Direction" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Direction`

Axis direction type from the TDM math library, aliased into the `dna` namespace.

### Why this exists

`Direction` is `tdm::axis_dir` re-exported under `dna::` so client code configuring coordinate systems does not need to import TDM headers directly. It represents the signed direction of a single spatial axis (e.g., positive X, negative Y) and is used to build a `CoordinateSystem` frame.

### Relationships

- `CoordinateSystem` (`tdm::coord_sys`) — composed from `Direction` values per axis
- `Configuration` — `coordinateSystem` field accepts a `CoordinateSystem` built from `Direction` values

<!-- ink:api-end name="Direction" -->

<!-- ink:api name="FaceWindingOrder" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `FaceWindingOrder`

Declare the vertex winding convention for geometry faces — counter-clockwise (OpenGL/Maya) or clockwise (DirectX/left-handed).

### Why this exists

DNA geometry is authored with CCW winding (the cross product of consecutive face vertices points along the outward surface normal). Engines using a left-handed coordinate system (DirectX, Unreal by default) expect CW winding, and submitting CCW faces to them causes inverted normals and back-face culling errors. Setting `faceWindingOrder = FaceWindingOrder::cw` in `Configuration` together with `CoordinateSystemTransformPolicy::Transform` instructs the reader to reverse winding at load time so the rest of the pipeline sees correctly oriented faces.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `ccw` | `uint8_t = 0` | Counter-clockwise winding. Authored default; cross product of consecutive vertices agrees with stored normals. |
| `cw` | `uint8_t = 1` | Clockwise winding. Left-handed / DirectX style; cross product opposes stored normals. |

### Constraints

- Winding conversion is applied only when `Configuration::coordinateSystemTransformPolicy` is `Transform`. Setting `faceWindingOrder` alone under `Preserve` policy has no effect.

### Relationships

- `Configuration` — set via the `faceWindingOrder` field
- `CoordinateSystemTransformPolicy` — must be `Transform` for winding conversion to take effect

<!-- ink:api-end name="FaceWindingOrder" -->

<!-- ink:api name="RotationDirection" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RotationDirection`

Per-axis rotation direction type from TDM, aliased into the `dna` namespace.

### Why this exists

`RotationDirection` is `tdm::rot_dir` re-exported under `dna::` to avoid requiring TDM imports in user code. It encodes the sign of rotation for a single axis — `positive` (right-hand rule) or `negative` (left-hand / inverted). Three `RotationDirection` values compose `RotationSign`, which is held by `Configuration::rotationSign`.

### Relationships

- `RotationSign` — three `RotationDirection` values (one per axis) form a `RotationSign`
- `Configuration` — `rotationSign` field uses this type

<!-- ink:api-end name="RotationDirection" -->

<!-- ink:api name="RotationSequence" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RotationSequence`

Euler rotation application order (e.g., XYZ, ZYX), aliased from TDM into the `dna` namespace.

### Why this exists

Euler angle decomposition is order-dependent — the same three angles produce different orientations under XYZ vs ZYX ordering. `RotationSequence` makes the intended order explicit rather than leaving it as an undocumented convention. The default in `Configuration` is `RotationSequence::xyz`, matching common DCC tool convention.

### Relationships

- `Configuration` — set via `rotationSequence`; applied globally to all joint rotations during coordinate conversion
- `RotationSign` — companion type controlling per-axis sign

<!-- ink:api-end name="RotationSequence" -->

<!-- ink:api name="RotationSign" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RotationSign`

Per-axis rotation sign aggregate from TDM, aliased into the `dna` namespace.

### Why this exists

`RotationSign` is `tdm::rot_sign` re-exported under `dna::`. It bundles three `RotationDirection` values — one for each spatial axis — into a single object, making the handedness of each axis explicit rather than encoded in a sign-bit convention. The default in `Configuration` sets all axes to `RotationDirection::positive`.

### Construction

```cpp
// Default: all axes positive (right-hand convention)
dna::RotationSign sign = {dna::RotationDirection::positive,
                          dna::RotationDirection::positive,
                          dna::RotationDirection::positive};

// Invert Y axis (common when converting to a left-handed Y-up frame)
dna::RotationSign signInvertY = {dna::RotationDirection::positive,
                                  dna::RotationDirection::negative,
                                  dna::RotationDirection::positive};
```

### Relationships

- `RotationDirection` — the element type; three of these compose a `RotationSign`
- `Configuration` — held by `rotationSign`; applied during coordinate conversion

<!-- ink:api-end name="RotationSign" -->

<!-- ink:api name="RotationUnit" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RotationUnit`

Specify the unit for rotation data — degrees or radians.

### Fields

| Name | Description |
|------|-------------|
| `degrees` | Rotation expressed in degrees. Common in DCC tools and human-readable exports. |
| `radians` | Rotation expressed in radians. Common for real-time math libraries. |

### Relationships

- `Configuration` — paired with `rotationSequence` and `rotationSign` when `CoordinateSystemTransformPolicy::Transform` is active

<!-- ink:api-end name="RotationUnit" -->

<!-- ink:api name="TranslationUnit" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `TranslationUnit`

Specify the unit of measurement for positional data — centimetres or metres.

### Fields

| Name | Description |
|------|-------------|
| `cm` | Centimetres. Common for DCC tools (Maya default scene unit). |
| `m` | Metres. Common for real-time engines (Unreal default world unit). |

### Relationships

- `Configuration` — used alongside `CoordinateSystemTransformPolicy` when converting spatial data between DCC and engine conventions

<!-- ink:api-end name="TranslationUnit" -->

<!-- ink:api name="UnknownLayerPolicy" module="dna/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `UnknownLayerPolicy`

Control whether unrecognised DNA layers are kept intact or silently dropped during load.

### Why this exists

DNA assets may contain layer data introduced by a newer writer than the reader understands. Silently discarding that data is safe for read-only inspection but catastrophic in a round-trip pipeline where the file is re-saved — it causes permanent data loss. `Preserve` (the default) prevents that by retaining opaque layer blobs verbatim. `Ignore` is the opt-in choice for pipelines that will never re-write the file and want a smaller in-memory footprint.

### Fields

| Name | Description |
|------|-------------|
| `Preserve` | Keep all unrecognised layer data in memory unchanged. Use this in any pipeline that may re-save the DNA file. |
| `Ignore` | Discard unrecognised layer data on load. Safe only for read-only consumers. |

### Relationships

- `Configuration` — set via the `unknownLayerPolicy` field
- `DataLayer` — companion enum that selects which known layers to load

<!-- ink:api-end name="UnknownLayerPolicy" -->
