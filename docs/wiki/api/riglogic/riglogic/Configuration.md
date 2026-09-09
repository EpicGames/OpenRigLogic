# API Reference — `riglogic/riglogic/Configuration`

---

<!-- ink:api name="CalculationType" module="riglogic/riglogic/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `CalculationType`

Selects which CPU algorithm implementation RigLogic uses to evaluate a rig.

### Why this exists

Different hardware supports different vectorization instruction sets, and RigLogic needs a way to pick (or auto-select) the fastest available path without the caller having to know the build's capabilities. `AnyVector` lets RigLogic pick whichever vectorized path is available, while the explicit values let a caller pin a specific implementation for testing or platform-specific tuning.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Scalar` | enum value | Scalar CPU algorithm; always available. |
| `SSE` | enum value | Vectorized CPU algorithm using SSE instructions. |
| `AVX` | enum value | Vectorized CPU algorithm using AVX instructions; requires RigLogic built with AVX support, otherwise falls back to `Scalar`. |
| `NEON` | enum value | Vectorized CPU algorithm using NEON instructions; requires RigLogic built with NEON support, otherwise falls back to `Scalar`. |
| `AnyVector` | enum value | Picks any available vectorization automatically. |

### Relationships

- `Configuration` — *stores a `CalculationType` in its `calculationType` field.*

### Watch out for

- `AVX` and `NEON` silently fall back to `Scalar` when the library was not built with the corresponding support — no error or warning is raised. Confirm the build configuration before relying on a specific SIMD path in performance-critical code.

<!-- ink:api-end name="CalculationType" -->

<!-- ink:api name="Configuration" module="riglogic/riglogic/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `Configuration`

The set of options that determines how RigLogic loads DNA data and evaluates a rig.

### Why this exists

Loading and evaluating a rig involves several independent choices — which algorithm to run, which floating point precision to use, which behavior submodules to load, and which output conventions to emit — that would otherwise require a long list of separate constructor parameters. `Configuration` groups all of these into a single value with sensible defaults, so callers only need to override the fields that matter to them. Loading fewer submodules (e.g. setting `loadRBFBehavior` to `false`) reduces the memory allocated for a `RigLogic` instance.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `calculationType` | `CalculationType` | optional. Algorithm implementation to use; defaults to `AnyVector`. |
| `floatingPointType` | `FloatingPointType` | optional. Precision for vectorized calculations; defaults to `HalfFloat`. |
| `loadJoints` | `bool` | optional. Whether to load joint behavior; defaults to `true`. |
| `loadBlendShapes` | `bool` | optional. Whether to load blend shape behavior; defaults to `true`. |
| `loadAnimatedMaps` | `bool` | optional. Whether to load animated map behavior; defaults to `true`. |
| `loadMachineLearnedBehavior` | `bool` | optional. Whether to load ML behavior; defaults to `true`. |
| `loadRBFBehavior` | `bool` | optional. Whether to load RBF behavior; defaults to `true`. |
| `loadTwistSwingBehavior` | `bool` | optional. Whether to load twist/swing behavior; defaults to `true`. |
| `translationType` | `TranslationType` | optional. Output representation for translation; defaults to `Vector`. |
| `rotationType` | `RotationType` | optional. Output representation for rotation; defaults to `EulerAngles`. |
| `scaleType` | `ScaleType` | optional. Output representation for scale; defaults to `Vector`. |
| `translationPruningThreshold` | `float` | optional. Threshold below which translation deltas are pruned; defaults to `0.0f` (comment suggests `0.0001f` is reasonably safe to try). |
| `rotationPruningThreshold` | `float` | optional. Threshold below which rotation deltas are pruned; defaults to `0.0f` (comment suggests `0.1f` is reasonably safe to try). |
| `scalePruningThreshold` | `float` | optional. Threshold below which scale deltas are pruned; defaults to `0.0f` (comment suggests `0.001f` is reasonably safe to try). |

### Construction

```cpp
rl4::Configuration config;
config.calculationType = rl4::CalculationType::AVX;
config.loadRBFBehavior = false;
auto* rigLogic = rl4::RigLogic::create(reader, config);
```

### Relationships

- `RigLogic` — *`RigLogic::create` takes a `Configuration` and `RigLogic::getConfiguration` returns the one it was built with.*
- `CalculationType`, `FloatingPointType`, `TranslationType`, `RotationType`, `ScaleType` — *enum types held by this struct's fields.*

### Watch out for

- Pruning thresholds are `0.0f` by default (disabled). Any non-zero value silently discards transform components below that magnitude — start at `0.0f` and increase incrementally while checking for visible artefacts on subtle joint motions before committing to a non-zero value.
- Setting `loadJoints = false` while leaving `loadBlendShapes = true` is valid, but some rigs assume joint data is present; verify with your specific DNA asset before shipping a partial-load configuration.

<!-- ink:api-end name="Configuration" -->

<!-- ink:api name="FloatingPointType" module="riglogic/riglogic/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `FloatingPointType`

Selects the floating point precision used in RigLogic's vectorized calculations.

### Why this exists

Vectorized rig evaluation can trade precision for memory and bandwidth. `FloatingPointType` makes that trade-off an explicit, per-`Configuration` choice rather than a compile-time constant, so callers can pick `HalfFloat` for reduced memory footprint or `Float` for full precision.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Float` | enum value | Full-precision floating point calculations. |
| `HalfFloat` | enum value | Reduced-precision (half float) calculations; the default in `Configuration`. |

### Relationships

- `Configuration` — *stores a `FloatingPointType` in its `floatingPointType` field.*

<!-- ink:api-end name="FloatingPointType" -->

<!-- ink:api name="RotationType" module="riglogic/riglogic/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RotationType`

Selects the representation RigLogic uses for rotation output values.

### Why this exists

Rig consumers may expect rotations either as Euler angles or as quaternions depending on the downstream engine or rig rig binding. `RotationType` lets `Configuration` pick the representation without RigLogic needing separate evaluation code paths per convention.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `EulerAngles` | enum value = 3 | 3-component Euler angle representation of rotation; the default in `Configuration`. |
| `Quaternions` | enum value = 4 | 4-component quaternion representation of rotation. |

### Relationships

- `Configuration` — *stores a `RotationType` in its `rotationType` field.*

<!-- ink:api-end name="RotationType" -->

<!-- ink:api name="ScaleType" module="riglogic/riglogic/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ScaleType`

Selects the representation RigLogic uses for scale output values.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enum value = 3 | 3-component vector representation of scale. |

### Relationships

- `Configuration` — *stores a `ScaleType` in its `scaleType` field.*

<!-- ink:api-end name="ScaleType" -->

<!-- ink:api name="TranslationType" module="riglogic/riglogic/Configuration" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `TranslationType`

Selects the representation RigLogic uses for translation output values.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enum value = 3 | 3-component vector representation of translation. |

### Relationships

- `Configuration` — *stores a `TranslationType` in its `translationType` field.*

<!-- ink:api-end name="TranslationType" -->
