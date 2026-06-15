# API Reference — `riglogic/riglogic/Configuration`

---

<!-- ink:api name="CalculationType" module="riglogic/riglogic/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `CalculationType`

Choose the CPU instruction set RigLogic uses for character deformation calculations. `AnyVector` (the default) picks the best available at runtime.

### Why this exists

Selecting an explicit implementation type lets a project trade portability for throughput — `Scalar` runs everywhere; `SSE`, `AVX`, and `NEON` exploit SIMD lanes for faster joint evaluation. Without this enum a caller would need to manage platform `#ifdef` blocks manually. The `AnyVector` sentinel delegates that decision to the runtime, covering most use-cases without build-time specialisation.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Scalar` | enumerator | Scalar CPU algorithm. Runs on all platforms. |
| `SSE` | enumerator | Vectorized SSE CPU algorithm. |
| `AVX` | enumerator | Vectorized AVX CPU algorithm. Falls back to `Scalar` if RigLogic was not built with AVX support. |
| `NEON` | enumerator | Vectorized NEON CPU algorithm. Falls back to `Scalar` if RigLogic was not built with NEON support. |
| `AnyVector` | enumerator | Pick any available vectorization at runtime. Recommended default. |

### Watch out for

- `AVX` and `NEON` silently fall back to `Scalar` when the library was not built with the corresponding support — no error or warning is raised. Confirm the build configuration before relying on a specific SIMD path in performance-critical code.

<!-- ink:api-end name="CalculationType" -->

<!-- ink:api name="Configuration" module="riglogic/riglogic/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Configuration`

Aggregate settings struct that controls which RigLogic subsystems are loaded and how deformation calculations are performed.

### Why this exists

Grouping all construction-time knobs into a single plain struct lets a caller configure RigLogic in one place and pass it by value, without managing a builder object or a long parameter list. Every field carries a sensible default, so `Configuration{}` is a fully valid "load everything, use best available hardware" configuration that requires no further setup for most projects.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `calculationType` | `CalculationType` | optional — CPU instruction set for deformation math. Default: `AnyVector`. |
| `floatingPointType` | `FloatingPointType` | optional — Floating-point precision for vectorized calculations. Default: `HalfFloat`. |
| `loadJoints` | `bool` | optional — Whether to load joint deformation data. Default: `true`. |
| `loadBlendShapes` | `bool` | optional — Whether to load blend shape data. Default: `true`. |
| `loadAnimatedMaps` | `bool` | optional — Whether to load animated map data. Default: `true`. |
| `loadMachineLearnedBehavior` | `bool` | optional — Whether to load ML behavior data. Default: `true`. |
| `loadRBFBehavior` | `bool` | optional — Whether to load RBF behavior data. Default: `true`. |
| `loadTwistSwingBehavior` | `bool` | optional — Whether to load twist/swing behavior data. Default: `true`. |
| `translationType` | `TranslationType` | optional — Joint translation representation. Default: `Vector`. |
| `rotationType` | `RotationType` | optional — Joint rotation representation. Default: `EulerAngles`. |
| `scaleType` | `ScaleType` | optional — Joint scale representation. Default: `Vector`. |
| `translationPruningThreshold` | `float` | optional — Discard translation values below this magnitude. Default: `0.0f` (disabled). Safe trial value: `0.0001f`. |
| `rotationPruningThreshold` | `float` | optional — Discard rotation values below this magnitude. Default: `0.0f` (disabled). Safe trial value: `0.1f`. |
| `scalePruningThreshold` | `float` | optional — Discard scale values below this magnitude. Default: `0.0f` (disabled). Safe trial value: `0.001f`. |

### Construction

```cpp
// Use all defaults: load every subsystem, AnyVector SIMD, HalfFloat precision
rl4::Configuration cfg{};

// Load only joints; use scalar math for a deterministic reference path
rl4::Configuration cfg{
    .calculationType = rl4::CalculationType::Scalar,
    .loadBlendShapes = false,
    .loadAnimatedMaps = false,
    .loadMachineLearnedBehavior = false,
    .loadRBFBehavior = false,
    .loadTwistSwingBehavior = false
};

// Enable gentle pruning to reduce output buffer writes
rl4::Configuration cfg{
    .translationPruningThreshold = 0.0001f,
    .rotationPruningThreshold = 0.1f,
    .scalePruningThreshold = 0.001f
};
```

### Relationships

- `CalculationType` — selects the SIMD path used during evaluation
- `FloatingPointType` — selects weight buffer precision
- `RotationType` — controls whether output rotations are Euler or quaternion
- `TranslationType` / `ScaleType` — control output component layout for translation and scale

### Watch out for

- Pruning thresholds are `0.0f` by default (disabled). Any non-zero value silently discards transform components below that magnitude — start at `0.0f` and increase incrementally while checking for visible artefacts on subtle joint motions before committing to a non-zero value.
- Setting `loadJoints = false` while leaving `loadBlendShapes = true` is valid, but some rigs assume joint data is present; verify with your specific DNA asset before shipping a partial-load configuration.

<!-- ink:api-end name="Configuration" -->

<!-- ink:api name="FloatingPointType" module="riglogic/riglogic/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `FloatingPointType`

Select the floating-point precision used in vectorized RigLogic calculations.

### Why this exists

Using `HalfFloat` halves the memory bandwidth of weight buffers, which is often the bottleneck in vectorized joint evaluation. `Float` trades that memory saving for higher precision — useful when weight values are very small or accumulate across many joints in a way that causes noticeable drift.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Float` | enumerator | Standard 32-bit single-precision float. Higher precision, higher memory cost. |
| `HalfFloat` | enumerator | 16-bit half-precision float. Lower memory bandwidth; default in `Configuration`. |

<!-- ink:api-end name="FloatingPointType" -->

<!-- ink:api name="RotationType" module="riglogic/riglogic/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RotationType`

Select whether joint rotations are expressed as Euler angles or quaternions.

### Why this exists

Euler angles match the representation used by most DCC tools and the existing DNA file format, making them the default. Quaternions avoid gimbal lock and are preferable when the downstream consumer performs further rotation math — such as blend trees or IK solvers — where composing Euler-angle rotations accumulates floating-point error. The enumerator values (`3` and `4`) encode the component count, used internally to stride rotation output buffers.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `EulerAngles` | enumerator | 3-component Euler angle representation. Default. Compatible with most DCC tools and the DNA format. |
| `Quaternions` | enumerator | 4-component quaternion representation. Preferred for downstream rotation composition (IK, blend trees). |

<!-- ink:api-end name="RotationType" -->

<!-- ink:api name="ScaleType" module="riglogic/riglogic/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ScaleType`

Specifies the representation used for joint scale data in RigLogic.

### Why this exists

Mirrors `TranslationType` — a named enum rather than a bare component count keeps `Configuration` readable and provides a future extension point for non-uniform or non-vector scale representations. Currently only `Vector` (3 components: X, Y, Z scale) is defined.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enumerator | 3-component XYZ scale vector. Value `3` encodes the component count used internally to stride scale buffers. |

<!-- ink:api-end name="ScaleType" -->

<!-- ink:api name="TranslationType" module="riglogic/riglogic/Configuration" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `TranslationType`

Specifies the representation used for joint translation data in RigLogic.

### Why this exists

Using a named enum rather than a bare component count makes `Configuration` self-documenting and provides an explicit extension point for future translation representations without breaking existing code that switches on this value. Currently only `Vector` (3 components: X, Y, Z) is defined.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Vector` | enumerator | 3-component XYZ vector. Value `3` encodes the component count used internally to stride translation buffers. |

<!-- ink:api-end name="TranslationType" -->
