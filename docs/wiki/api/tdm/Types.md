# API Reference — `tdm/Types`

---

<!-- ink:api name="ang" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ang<T, TUnit>`

Type-safe angle template that carries its unit (degrees or radians) as a phantom type parameter.

### Why this exists

A bare `float` representing an angle cannot communicate whether it is in degrees or radians — a common source of hard-to-find bugs in 3D math code. `ang<T, TUnit>` encodes the unit at the type level, making unit mismatches a compile error rather than a runtime artifact. The concrete aliases `deg<T>` and `rad<T>` are the primary consumer-facing types; `ang` is the generic base.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | typename | required — numeric element type (typically `float`) |
| `TUnit` | typename | required — unit tag type; use `impl::degrees` or `impl::radians` |

### Relationships

- `deg` — alias that fixes `TUnit` to `impl::degrees`
- `rad` — alias that fixes `TUnit` to `impl::radians`
- `impl::degrees`, `impl::radians` — phantom tag structs for the unit parameter

<!-- ink:api-end name="ang" -->

<!-- ink:api name="axis_dir" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `axis_dir`

Enum of the six cardinal axis directions used to specify which world direction each coordinate axis points.

### Why this exists

Different tools assign different world-space directions to their local X, Y, and Z axes (e.g., Y-up vs. Z-up, left-forward vs. right-forward). Using `axis_dir` to describe the orientation of each axis — rather than an integer index or string — makes the coordinate system configuration explicit and type-safe, and feeds directly into `coord_sys` construction.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `left` | enumerator | axis points in the world-left direction |
| `right` | enumerator | axis points in the world-right direction |
| `up` | enumerator | axis points in the world-up direction |
| `down` | enumerator | axis points in the world-down direction |
| `front` | enumerator | axis points in the world-forward direction |
| `back` | enumerator | axis points in the world-backward direction |

### Relationships

- `coord_sys` — uses `axis_dir` values to define its orientation

<!-- ink:api-end name="axis_dir" -->

<!-- ink:api name="chirality" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `chirality`

Enum that distinguishes left-handed from right-handed coordinate systems.

### Why this exists

Different DCCs and game engines use different handedness conventions. Passing a bare `int` or `bool` for handedness is error-prone and unreadable. `chirality` makes the convention explicit at every call site, and its `left = -1` / `right = 1` numeric values let it participate directly in sign-flip calculations without a branch.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `left` | `int(-1)` | left-handed coordinate system |
| `right` | `int(1)` | right-handed coordinate system |

### Relationships

- `coord_sys` — coordinate system descriptor that consumes `chirality`

<!-- ink:api-end name="chirality" -->

<!-- ink:api name="coord_sys" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `coord_sys`

Forward-declared struct that represents a complete spatial coordinate system convention.

### Why this exists

Using a raw `mat4` or a set of loose enumerators to describe a coordinate frame loses semantic meaning — you can no longer distinguish a rigid-body orientation from an arbitrary affine transform, and the handedness, axis directions, and rotation sign conventions are all implicit. `coord_sys` makes these conventions explicit and prevents transforms that would violate the frame's invariants. The struct is forward-declared here; its full definition (fields and construction) is in the implementing source file.

### Relationships

- `chirality` — handedness component of a coordinate system
- `axis_dir` — per-axis direction assignments
- `rot_sign` — per-axis rotation sign convention
- `rot_seq` — Euler rotation order used with this system

<!-- ink:api-end name="coord_sys" -->

<!-- ink:api name="deg" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `deg<T>`

Degree-unit angle alias — an `ang` whose unit is statically known to be degrees.

### Why this exists

Use `deg<float>` (or the shorthand `fdeg`) when passing degree values to APIs that distinguish angle units at the type level. Passing a `deg` where a `rad` is expected is a compile error, not a silent unit bug.

### Relationships

- `ang` — underlying template (`ang<T, impl::degrees>`)
- `fdeg` — concrete float alias (`deg<float>`)
- `deg3` — 3-component vector of `deg<T>` values
- `rad` — complementary radians alias

<!-- ink:api-end name="deg" -->

<!-- ink:api name="deg3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `deg3<T>`

3-component vector of degree-unit angles — the standard type for Euler angle triples in degrees.

### Why this exists

Representing Euler angles as a plain `vec3<float>` loses the unit information. `deg3<T>` combines `vec3` layout with the `deg` unit tag so that a degree-valued rotation triple cannot be silently passed to a function expecting radians.

### Relationships

- `vec3` — underlying layout template (`vec3<deg<T>>`)
- `deg` — element type
- `fdeg3` — concrete float alias
- `rad3` — radians counterpart

<!-- ink:api-end name="deg3" -->

<!-- ink:api name="degrees" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `impl::degrees`

Phantom tag type that marks an `ang` value as being in degrees.

### Why this exists

`impl::degrees` is not meant to be instantiated or stored directly. It exists solely to distinguish `ang<T, impl::degrees>` from `ang<T, impl::radians>` at compile time. Using a named tag struct instead of a boolean or enum allows the type system to reject invalid unit combinations at the instantiation site.

### Relationships

- `ang` — uses this as the `TUnit` phantom parameter
- `deg` — top-level alias that binds `TUnit` to `impl::degrees`
- `impl::radians` — paired radians tag

<!-- ink:api-end name="degrees" -->

<!-- ink:api name="dim_t" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `dim_t`

Unsigned integer type used to express vector and matrix dimensions throughout the tdm library.

### Why this exists

Template parameters for `vec<L, T>` and `mat<R, C, T>` need a semantic type for dimension counts — not a raw `int` or `size_t`. Using `dim_t` makes dimension arguments visually distinct from element-type arguments in template parameter lists and prevents accidental mixing with signed integers in dimension arithmetic.

### Relationships

- `vec` — uses `dim_t` as its dimension template parameter `L`
- `mat` — uses `dim_t` for both row count `R` and column count `C`

<!-- ink:api-end name="dim_t" -->

<!-- ink:api name="fdeg" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fdeg`

Float degree angle — the most common concrete type for a single angle value in degrees.

### Relationships

- `deg` — parent alias (`deg<float>`)
- `frad` — radians counterpart
- `fdeg3` — 3-component version

<!-- ink:api-end name="fdeg" -->

<!-- ink:api name="fdeg3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fdeg3`

3-component float degree angle vector — the standard Euler triple in degrees for rig math.

### Relationships

- `deg3` — parent alias (`deg3<float>`)
- `frad3` — radians counterpart
- `fdeg` — single-angle component type

<!-- ink:api-end name="fdeg3" -->

<!-- ink:api name="fmat" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fmat<R, C>`

Float matrix alias template — a `mat` of `float` components at any dimension.

### Why this exists

Pins the element type to `float`, leaving row and column counts as the only free parameters. Use `fmat<R, C>` in generic code that accepts float-element matrices at non-square or non-standard dimensions, without separately templating on the element type.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `R` | `dim_t` | required — number of rows |
| `C` | `dim_t` | required — number of columns |

### Relationships

- `mat` — underlying template (`mat<R, C, float>`)
- `fmat2`, `fmat3`, `fmat4` — pre-instantiated square shorthands

<!-- ink:api-end name="fmat" -->

<!-- ink:api name="fmat2" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fmat2`

2×2 float matrix (`mat<2, 2, float>`).

### Relationships

- `fmat` — parent alias template
- `imat2` — integer counterpart

<!-- ink:api-end name="fmat2" -->

<!-- ink:api name="fmat3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fmat3`

3×3 float matrix (`mat<3, 3, float>`).

### Relationships

- `fmat` — parent alias template
- `imat3` — integer counterpart

<!-- ink:api-end name="fmat3" -->

<!-- ink:api name="fmat4" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fmat4`

4×4 float matrix (`mat<4, 4, float>`).

### Relationships

- `fmat` — parent alias template
- `imat4` — integer counterpart

<!-- ink:api-end name="fmat4" -->

<!-- ink:api name="fquat" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fquat`

Float quaternion — the standard concrete type for 3D rotation in rig math.

### Relationships

- `quat` — parent template (`quat<float>`)

<!-- ink:api-end name="fquat" -->

<!-- ink:api name="frad" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `frad`

Float radian angle — the most common concrete type for a single angle value in radians.

### Relationships

- `rad` — parent alias (`rad<float>`)
- `fdeg` — degrees counterpart
- `frad3` — 3-component version

<!-- ink:api-end name="frad" -->

<!-- ink:api name="frad3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `frad3`

3-component float radian angle vector — the standard Euler triple in radians for rig math.

### Relationships

- `rad3` — parent alias (`rad3<float>`)
- `fdeg3` — degrees counterpart
- `frad` — single-angle component type

<!-- ink:api-end name="frad3" -->

<!-- ink:api name="fvec" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fvec<L>`

Float vector alias template — a `vec` of `float` components at any dimension.

### Why this exists

Pins the element type to `float`, leaving dimension as the only free parameter. Use `fvec<L>` in generic code that must accept any float vector regardless of dimensionality, without templating on the element type separately.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `L` | `dim_t` | required — number of float components |

### Relationships

- `vec` — underlying template (`vec<L, float>`)
- `fvec2`, `fvec3`, `fvec4` — pre-instantiated shorthands

<!-- ink:api-end name="fvec" -->

<!-- ink:api name="fvec2" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fvec2`

2-component float vector (`vec<2, float>`).

### Relationships

- `fvec` — parent alias template
- `ivec2` — integer counterpart

<!-- ink:api-end name="fvec2" -->

<!-- ink:api name="fvec3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fvec3`

3-component float vector (`vec<3, float>`).

### Relationships

- `fvec` — parent alias template
- `ivec3` — integer counterpart
- `deg3`, `rad3` — angle-vector types that wrap `vec3<deg<T>>` / `vec3<rad<T>>`

<!-- ink:api-end name="fvec3" -->

<!-- ink:api name="fvec4" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `fvec4`

4-component float vector (`vec<4, float>`).

### Relationships

- `fvec` — parent alias template
- `ivec4` — integer counterpart

<!-- ink:api-end name="fvec4" -->

<!-- ink:api name="imat" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `imat<R, C>`

Integer matrix alias template — a `mat` of `int` components at any dimension.

### Why this exists

Pins the element type to `int`, leaving row and column counts as free parameters. Use `imat<R, C>` in generic code that operates on integer-element matrices of arbitrary size, mirroring the role `ivec<L>` plays for vectors.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `R` | `dim_t` | required — number of rows |
| `C` | `dim_t` | required — number of columns |

### Relationships

- `mat` — underlying template (`mat<R, C, int>`)
- `imat2`, `imat3`, `imat4` — pre-instantiated square shorthands

<!-- ink:api-end name="imat" -->

<!-- ink:api name="imat2" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `imat2`

2×2 integer matrix (`mat<2, 2, int>`).

### Relationships

- `imat` — parent alias template
- `fmat2` — float counterpart

<!-- ink:api-end name="imat2" -->

<!-- ink:api name="imat3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `imat3`

3×3 integer matrix (`mat<3, 3, int>`).

### Relationships

- `imat` — parent alias template
- `fmat3` — float counterpart

<!-- ink:api-end name="imat3" -->

<!-- ink:api name="imat4" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `imat4`

4×4 integer matrix (`mat<4, 4, int>`).

### Relationships

- `imat` — parent alias template
- `fmat4` — float counterpart

<!-- ink:api-end name="imat4" -->

<!-- ink:api name="ivec" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ivec<L>`

Integer vector alias template — a `vec` of `int` components at any dimension.

### Why this exists

Provides a single alias template that pins the element type to `int`, leaving the dimension as the only parameter. Use `ivec<L>` when you need integer-component vectors at a dimension not covered by `ivec2`/`ivec3`/`ivec4`, or as the common base in generic code that accepts any integer-dimension vector.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `L` | `dim_t` | required — number of integer components |

### Relationships

- `vec` — underlying template (`vec<L, int>`)
- `ivec2`, `ivec3`, `ivec4` — pre-instantiated shorthands

<!-- ink:api-end name="ivec" -->

<!-- ink:api name="ivec2" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ivec2`

2-component integer vector (`vec<2, int>`).

### Relationships

- `ivec` — parent alias template
- `fvec2` — float counterpart

<!-- ink:api-end name="ivec2" -->

<!-- ink:api name="ivec3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ivec3`

3-component integer vector (`vec<3, int>`).

### Relationships

- `ivec` — parent alias template
- `fvec3` — float counterpart

<!-- ink:api-end name="ivec3" -->

<!-- ink:api name="ivec4" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ivec4`

4-component integer vector (`vec<4, int>`).

### Relationships

- `ivec` — parent alias template
- `fvec4` — float counterpart

<!-- ink:api-end name="ivec4" -->

<!-- ink:api name="mat" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>`

Generic fixed-size matrix template parameterized by row count, column count, and element type.

### Why this exists

Provides a single struct template that backs all concrete matrix aliases (`mat2`, `mat3`, `imat`, `fmat`, etc.) in the tdm library, mirroring the design of `vec`. Separating the generic definition from its aliases allows dimension and type combinations to be added without new struct definitions.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `R` | `dim_t` | required — number of rows |
| `C` | `dim_t` | required — number of columns |
| `T` | typename | required — element type (e.g., `float`, `int`) |

### Relationships

- `dim_t` — type of the dimension parameters
- `mat2`, `mat3`, `mat4` — square-matrix aliases
- `imat`, `fmat` — element-type aliases

<!-- ink:api-end name="mat" -->

<!-- ink:api name="mat2" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat2<T>`

Convenience alias for a 2×2 matrix of any element type.

### Relationships

- `mat` — underlying template (`mat<2, 2, T>`)
- `fmat2` — float specialization
- `imat2` — integer specialization

<!-- ink:api-end name="mat2" -->

<!-- ink:api name="mat3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat3<T>`

Convenience alias for a 3×3 matrix of any element type.

### Relationships

- `mat` — underlying template (`mat<3, 3, T>`)
- `fmat3` — float specialization
- `imat3` — integer specialization

<!-- ink:api-end name="mat3" -->

<!-- ink:api name="mat4" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat4<T>`

Convenience alias for a 4×4 matrix of any element type.

### Relationships

- `mat` — underlying template (`mat<4, 4, T>`)
- `fmat4` — float specialization
- `imat4` — integer specialization

<!-- ink:api-end name="mat4" -->

<!-- ink:api name="quat" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `quat<T>`

Generic quaternion template for representing 3D rotations without gimbal lock.

### Why this exists

Euler angle triples (`deg3`, `rad3`) cannot represent all rotations unambiguously and exhibit gimbal lock at certain configurations. `quat<T>` provides the type foundation for quaternion-based rotation, which avoids both issues and composes rotations efficiently via multiplication.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | typename | required — element type (typically `float`) |

### Relationships

- `fquat` — concrete float alias
- `deg3`, `rad3` — Euler angle types that `quat` operations may convert from/to

<!-- ink:api-end name="quat" -->

<!-- ink:api name="rad" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rad<T>`

Radian-unit angle alias — an `ang` whose unit is statically known to be radians.

### Why this exists

Use `rad<float>` (or the shorthand `frad`) when passing radian values to APIs that enforce angle units at the type level. The compiler rejects mixing `rad` and `deg` arguments, eliminating the most common trigonometry bug in math libraries.

### Relationships

- `ang` — underlying template (`ang<T, impl::radians>`)
- `frad` — concrete float alias (`rad<float>`)
- `rad3` — 3-component vector of `rad<T>` values
- `deg` — complementary degrees alias

<!-- ink:api-end name="rad" -->

<!-- ink:api name="rad3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rad3<T>`

3-component vector of radian-unit angles — the standard type for Euler angle triples in radians.

### Why this exists

Mirrors `deg3` for the radians unit. Trigonometric and rotation functions in `tdm` that accept Euler triples will typically take `rad3<float>` rather than a bare `fvec3`, making the unit requirement visible in the function signature.

### Relationships

- `vec3` — underlying layout template (`vec3<rad<T>>`)
- `rad` — element type
- `frad3` — concrete float alias
- `deg3` — degrees counterpart

<!-- ink:api-end name="rad3" -->

<!-- ink:api name="radians" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `impl::radians`

Phantom tag type that marks an `ang` value as being in radians.

### Why this exists

`impl::radians` is not meant to be instantiated or stored directly. It exists solely to distinguish `ang<T, impl::radians>` from `ang<T, impl::degrees>` at compile time, preventing accidental mixing of angle units in function signatures and template specializations.

### Relationships

- `ang` — uses this as the `TUnit` phantom parameter
- `rad` — top-level alias that binds `TUnit` to `impl::radians`
- `impl::degrees` — paired degrees tag

<!-- ink:api-end name="radians" -->

<!-- ink:api name="rot_dir" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rot_dir`

Enum that encodes the sign (direction) of a rotation about an axis.

### Why this exists

Coordinate system conventions differ not only in handedness and axis orientation but also in which direction a positive rotation angle moves. Using a `rot_dir` enum instead of a bare `int` or sign constant makes the intended convention readable at the call site and allows `rot_sign` to bundle per-axis directions into a single struct.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `negative` | `int(-1)` | positive angle values rotate in the negative (clockwise-when-looking-along-axis) direction |
| `positive` | `int(1)` | positive angle values rotate in the positive (counterclockwise-when-looking-along-axis) direction |

### Relationships

- `rot_sign` — struct that holds one `rot_dir` per axis

<!-- ink:api-end name="rot_dir" -->

<!-- ink:api name="rot_seq" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rot_seq`

Enum of the six Euler rotation application sequences (e.g., XYZ, ZYX).

### Why this exists

Euler angles are ambiguous without knowing the order in which the three axis rotations are applied — the same `(rx, ry, rz)` triple produces a different orientation for `xyz` versus `zyx`. Encoding the sequence as an explicit enum value forces callers to be unambiguous about the convention and allows conversion functions to dispatch correctly.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `xyz` | enumerator | apply X, then Y, then Z rotation |
| `xzy` | enumerator | apply X, then Z, then Y rotation |
| `yxz` | enumerator | apply Y, then X, then Z rotation |
| `yzx` | enumerator | apply Y, then Z, then X rotation |
| `zxy` | enumerator | apply Z, then X, then Y rotation |
| `zyx` | enumerator | apply Z, then Y, then X rotation |

### Relationships

- `coord_sys` — coordinate system descriptor that may reference a rotation sequence
- `deg3`, `rad3` — Euler angle triples that are meaningful only with a `rot_seq`

<!-- ink:api-end name="rot_seq" -->

<!-- ink:api name="rot_sign" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rot_sign`

Aggregate of per-axis rotation directions that fully describes the sign convention of a coordinate system.

### Why this exists

A coordinate system's rotation convention requires specifying not just handedness but the direction of positive rotation about each individual axis. `rot_sign` groups the three per-axis `rot_dir` values into a single named type so the full sign convention can be passed and stored as a unit rather than three separate parameters.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `rot_dir` | required — positive rotation direction about the X axis |
| `y` | `rot_dir` | required — positive rotation direction about the Y axis |
| `z` | `rot_dir` | required — positive rotation direction about the Z axis |

### Relationships

- `rot_dir` — element type for each field
- `coord_sys` — coordinate system descriptor that incorporates `rot_sign`

<!-- ink:api-end name="rot_sign" -->

<!-- ink:api name="vec" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `vec<L, T>`

Generic fixed-length vector template parameterized by dimension count and element type.

### Why this exists

Provides a single struct template that backs all concrete vector aliases (`vec2`, `vec3`, `ivec`, `fvec`, etc.) in the tdm library. Keeping the generic struct separate from its concrete aliases means new element types and dimension combinations can be introduced without adding new struct definitions — only new `using` declarations.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `L` | `dim_t` | required — number of components |
| `T` | typename | required — element type (e.g., `float`, `int`) |

### Relationships

- `dim_t` — type of the dimension parameter `L`
- `vec2`, `vec3`, `vec4` — concrete dimension aliases for this template
- `ivec`, `fvec` — concrete element-type aliases for this template

<!-- ink:api-end name="vec" -->

<!-- ink:api name="vec2" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `vec2<T>`

Convenience alias for a 2-component vector of any element type.

### Relationships

- `vec` — underlying generic template (`vec<2, T>`)
- `fvec2` — float specialization
- `ivec2` — integer specialization

<!-- ink:api-end name="vec2" -->

<!-- ink:api name="vec3" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `vec3<T>`

Convenience alias for a 3-component vector of any element type.

### Relationships

- `vec` — underlying generic template (`vec<3, T>`)
- `fvec3` — float specialization
- `ivec3` — integer specialization
- `deg3`, `rad3` — angle-vector composites built on `vec3`

<!-- ink:api-end name="vec3" -->

<!-- ink:api name="vec4" module="tdm/Types" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `vec4<T>`

Convenience alias for a 4-component vector of any element type.

### Relationships

- `vec` — underlying generic template (`vec<4, T>`)
- `fvec4` — float specialization
- `ivec4` — integer specialization

<!-- ink:api-end name="vec4" -->
