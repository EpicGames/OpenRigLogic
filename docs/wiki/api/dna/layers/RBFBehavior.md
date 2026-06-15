# API Reference — `dna/layers/RBFBehavior`

---

<!-- ink:api name="AutomaticRadius" module="dna/layers/RBFBehavior" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `AutomaticRadius`

Enable or disable automatic radius computation for RBF targets.

### Why this exists

When set to `On`, the solver derives a per-target influence radius from the distribution of target poses, removing the need to hand-tune radius values. `Off` gives callers manual control over radius, which is necessary when auto-computed radii produce undesired blending behavior in sparse or non-uniform target layouts.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `On` | enumerator | Solver automatically computes per-target radius from target distribution. |
| `Off` | enumerator | Radius is specified manually by the caller. |

### Relationships

- `RBFSolverType` — automatic radius applies to both solver types

<!-- ink:api-end name="AutomaticRadius" -->

<!-- ink:api name="RBFDistanceMethod" module="dna/layers/RBFBehavior" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RBFDistanceMethod`

Specify how the RBF solver measures distance between input poses and target poses.

### Why this exists

Rotation inputs cannot be meaningfully compared with standard Euclidean distance — doing so produces artifacts because quaternion space is curved. This enum encodes the geometric assumption the solver makes about its inputs, preventing silent misuse of a Euclidean metric on rotation data. The `SwingAngle` and `TwistAngle` variants decompose rotation distance along anatomically meaningful axes, enabling per-axis RBF solvers used in character rigging workflows.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Euclidean` | enumerator | Standard n-dimensional distance measure. Use when inputs are not rotations. |
| `Quaternion` | enumerator | Treats inputs as quaternions and uses geodesic distance in quaternion space. |
| `SwingAngle` | enumerator | Treats inputs as quaternions; measures the angle between rotated `TwistAxis` directions. Isolates swing component of rotation. |
| `TwistAngle` | enumerator | Treats inputs as half-quaternions; measures distance between rotations around the `TwistAxis`. Isolates twist component. |

### Relationships

- `RBFSolverType` — distance method is an input to the solver's weight computation
- `Twist` (`dna/layers/Twist.h`) — defines the `TwistAxis` used by `SwingAngle` and `TwistAngle`

<!-- ink:api-end name="RBFDistanceMethod" -->

<!-- ink:api name="RBFFunctionType" module="dna/layers/RBFBehavior" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RBFFunctionType`

Choose the radial basis function kernel applied when computing influence weights from target distances.

### Why this exists

Different RBF kernels produce qualitatively different weight falloff curves, and the right choice depends on the density and distribution of targets. `Gaussian` and `Exponential` decay smoothly toward zero, which suits densely packed targets. `Linear`, `Cubic`, and `Quintic` are polynomial kernels that avoid the sharp near-zero tails of exponentials and can generalize better with sparse targets.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Gaussian` | enumerator | Smooth exponential falloff; `exp(-r²)` shape. |
| `Exponential` | enumerator | Exponential decay; steeper falloff than Gaussian. |
| `Linear` | enumerator | Linear polynomial kernel; fastest to evaluate. |
| `Cubic` | enumerator | Cubic polynomial kernel; smoother than linear. |
| `Quintic` | enumerator | Fifth-degree polynomial kernel; smoothest polynomial option. |

### Relationships

- `RBFSolverType` — the solver type determines how the chosen kernel's outputs are combined
- `RBFDistanceMethod` — the distance metric feeds into the kernel's `r` parameter

<!-- ink:api-end name="RBFFunctionType" -->

<!-- ink:api name="RBFNormalizeMethod" module="dna/layers/RBFBehavior" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RBFNormalizeMethod`

Control when the RBF solver normalizes blended weights to sum to 1.0.

### Why this exists

Additive RBF solvers can produce summed weights greater than 1.0 when targets overlap, and normalization is needed to keep blended outputs in a valid range. Applying normalization unconditionally when weights already sum to ≤1 can unnecessarily rescale valid outputs. Separating `OnlyNormalizeAboveOne` from `AlwaysNormalize` gives callers explicit control over that tradeoff; this enum is primarily relevant when `RBFSolverType::Additive` is selected, as interpolative solvers produce bounded weights without requiring normalization.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `OnlyNormalizeAboveOne` | enumerator | Normalizes weights only when their sum exceeds 1.0. Avoids rescaling already-valid weight distributions. |
| `AlwaysNormalize` | enumerator | Normalizes weights unconditionally on every evaluation. |

### Relationships

- `RBFSolverType` — normalization is most relevant when `Additive` is selected; `Interpolative` produces bounded weights without it

<!-- ink:api-end name="RBFNormalizeMethod" -->

<!-- ink:api name="RBFSolverType" module="dna/layers/RBFBehavior" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `RBFSolverType`

Select the algorithm an RBF solver uses to combine target contributions — additive summation or distance-weighted interpolation.

### Why this exists

Additive and interpolative RBF solvers make different tradeoffs between speed, coverage, and output quality. Encoding the choice as a typed enum makes call sites self-documenting and prevents invalid solver identifiers from being passed. The critical operational difference is that `Additive` requires a normalization pass (see `RBFNormalizeMethod`) for smooth results, while `Interpolative` guarantees weight values within 0%–100% with no normalization step needed.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Additive` | enumerator | Sums contributions from each target. Faster but may require more targets for full coverage; requires normalization for smooth results. |
| `Interpolative` | enumerator | Interpolates values by distance from each target. Produces smoother results with fewer targets than `Additive`, but at higher computational cost. Weights are bounded within 0%–100% without normalization. |

### Relationships

- `RBFNormalizeMethod` — controls whether and when normalization is applied; most relevant when `Additive` is selected
- `AutomaticRadius` — radius tuning applies to both solver types

<!-- ink:api-end name="RBFSolverType" -->
