# API Reference — `dna/layers/RBFBehavior`

---

<!-- ink:api name="AutomaticRadius" module="dna/layers/RBFBehavior" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `AutomaticRadius`

Toggles whether an RBF target's radius of influence is computed automatically rather than set explicitly.

### Why this exists

Manually tuning each RBF target's radius is tedious and error-prone across large pose sets; this flag lets a rig opt into automatic radius computation instead, falling back to explicit control only where needed.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `On` | enumerator | Radius is computed automatically. |
| `Off` | enumerator | Radius must be set explicitly. |

<!-- ink:api-end name="AutomaticRadius" -->

<!-- ink:api name="RBFDistanceMethod" module="dna/layers/RBFBehavior" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RBFDistanceMethod`

Selects how the distance between an input and an RBF pose target is measured.

### Why this exists

Raw Euclidean distance is not always the right metric for RBF inputs — rotational data behaves differently depending on whether it's treated as a full quaternion or decomposed into swing/twist about a specific axis. This type lets a pose target be evaluated with the distance metric that matches the kind of data driving it (n-dimensional, quaternion, swing, or twist).

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Euclidean` | enumerator | Standard n-dimensional distance measure. |
| `Quaternion` | enumerator | Treats inputs as a quaternion. |
| `SwingAngle` | enumerator | Treats inputs as a quaternion and finds distance between rotated `TwistAxis` direction. |
| `TwistAngle` | enumerator | Treats inputs as a half quaternion and finds distance between rotations around the `TwistAxis` direction. |

### Relationships

- `TwistAxis` — *the axis used by `SwingAngle` and `TwistAngle` distance measures.*
- `RBFSolverType` — *the solver that consumes this distance measure.*

<!-- ink:api-end name="RBFDistanceMethod" -->

<!-- ink:api name="RBFFunctionType" module="dna/layers/RBFBehavior" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RBFFunctionType`

Selects the radial falloff function applied to distance when computing an RBF target's contribution weight.

### Why this exists

RBF solvers need a way to convert "distance from target" into "contribution weight," and different falloff shapes (Gaussian, exponential, linear, cubic, quintic) produce different blend characteristics. Exposing this as a type lets each RBF setup choose the falloff curve that best matches its desired responsiveness.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Gaussian` | enumerator | Bell-curve falloff. |
| `Exponential` | enumerator | Exponential decay falloff. |
| `Linear` | enumerator | Linear falloff. |
| `Cubic` | enumerator | Cubic falloff. |
| `Quintic` | enumerator | Quintic falloff. |

### Relationships

- `RBFSolverType` — *the solver algorithm that applies this falloff function.*
- `RBFDistanceMethod` — *supplies the distance value this function is applied to.*

<!-- ink:api-end name="RBFFunctionType" -->

<!-- ink:api name="RBFNormalizeMethod" module="dna/layers/RBFBehavior" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RBFNormalizeMethod`

Selects when contribution weights from an RBF solve are normalized to sum to a consistent total.

### Why this exists

The additive `RBFSolverType` can produce weights that exceed the expected 0-100% range depending on how many targets contribute, so a normalization policy is needed to keep results well-behaved. This type lets that policy be chosen per setup — only normalize when weights exceed one, or always normalize.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `OnlyNormalizeAboveOne` | enumerator | Normalize only when the summed weight exceeds one. |
| `AlwaysNormalize` | enumerator | Always normalize the summed weight. |

### Relationships

- `RBFSolverType` — *the additive solver is the primary consumer of this normalization policy.*

<!-- ink:api-end name="RBFNormalizeMethod" -->

<!-- ink:api name="RBFSolverType" module="dna/layers/RBFBehavior" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `RBFSolverType`

Selects the algorithm used to combine RBF (radial basis function) pose target contributions into a final weight.

### Why this exists

An RBF solver needs to be able to trade off speed for coverage: the additive solver sums contributions and is cheap but needs more targets and a normalization pass, while the interpolative solver blends by distance and gives smoother results with fewer targets at higher cost. Exposing this as a type lets a rig pick the right trade-off per solver instead of hard-coding one strategy.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Additive` | enumerator | Sums contributions from each target; faster, may need more targets and normalization for smooth results. |
| `Interpolative` | enumerator | Interpolates values from each target by distance; smoother with fewer targets, at higher computational cost. |

### Relationships

- `RBFFunctionType` — *the falloff function used to weight each target's contribution.*
- `RBFDistanceMethod` — *how distance between input and target is measured for this solver.*
- `RBFNormalizeMethod` — *whether/how results are normalized, most relevant to the additive solver.*

<!-- ink:api-end name="RBFSolverType" -->
