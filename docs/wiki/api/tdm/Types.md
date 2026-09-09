# API Reference — `tdm/Types`

---

<!-- ink:api name="ang" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ang`

Generic angle type parameterized by numeric type and angle unit (radians or degrees).

### Why this exists

`ang` exists to make the angle unit part of the type system instead of a raw float that
could silently be radians in one place and degrees in another. Its second template
parameter, `TUnit` (`impl::radians` or `impl::degrees`), enforces that a value's unit is
explicit at the type level, preventing unit-mismatch bugs at call sites.

### Relationships

- `deg`, `rad` — *the degree- and radian-unit specializations of this template*
- `impl::radians`, `impl::degrees` — *the tag types used as `TUnit`*

<!-- ink:api-end name="ang" -->

<!-- ink:api name="axis_dir" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `axis_dir`

Enumerates the six named spatial directions (left, right, up, down, front, back) used to describe how a coordinate system's axes map onto real-world directions.

### Why this exists

Different engines and tools disagree on which axis points "up" or "forward." `axis_dir` gives a coordinate system definition (`coord_sys`) a way to state, per axis, which real-world direction it points in, so conversions between coordinate conventions (e.g., Maya vs. Unreal) can be done mechanically instead of by convention memorized in code comments.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `left`, `right`, `up`, `down`, `front`, `back` | enumerators | Named spatial directions an axis can be mapped to. |

### Relationships

- `coord_sys` — *coordinate system definition built from `axis_dir` mappings*
- `chirality` — *handedness implied by a combination of `axis_dir` values*

<!-- ink:api-end name="axis_dir" -->

<!-- ink:api name="chirality" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `chirality`

Enumerates the handedness (left- or right-handed) of a coordinate system.

### Why this exists

Coordinate-system handedness affects the sign of cross products and rotation direction, and silently assuming the wrong handedness is a common source of mirrored or inverted transforms. `chirality` makes this an explicit, checkable property rather than an implicit convention.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `left` | enumerator (`-1`) | Left-handed coordinate system. |
| `right` | enumerator (`1`) | Right-handed coordinate system. |

### Relationships

- `coord_sys` — *coordinate system definition that carries a `chirality` value*
- `axis_dir` — *axis directions whose combination determines a system's chirality*

<!-- ink:api-end name="chirality" -->

<!-- ink:api name="coord_sys" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `coord_sys`

Forward-declared type describing a coordinate system's axis directions, handedness, and rotation conventions as a single value.

### Why this exists

`coord_sys` exists to bundle the pieces that define a coordinate convention — `axis_dir` mappings, `chirality`, and `rot_sign` — into one describable value, rather than requiring code to reason about several independent enums whenever it needs to convert data between two coordinate conventions (e.g., different DCC tools or game engines).

### Relationships

- `axis_dir` — *per-axis direction mapping used to build a `coord_sys`*
- `chirality` — *handedness component of a `coord_sys`*
- `rot_sign` — *rotation sign convention component of a `coord_sys`*

<!-- ink:api-end name="coord_sys" -->

<!-- ink:api name="deg" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `deg`

Alias for an angle value held in degrees, `ang<T, impl::degrees>`.

### Construction

```cpp
fdeg heading{ 90.0f };
```

### Relationships

- `ang` — *the underlying generic angle template this alias specializes*
- `rad` — *the radian-unit counterpart of this alias*
- `deg3`, `fdeg`, `fdeg3` — *vector and float-specialized forms built on this alias*

<!-- ink:api-end name="deg" -->

<!-- ink:api name="deg3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `deg3`

A 3-component vector of degree-unit angles, used for Euler rotations expressed in degrees.

### Construction

```cpp
fdeg3 euler{ fdeg{0.0f}, fdeg{45.0f}, fdeg{90.0f} };
```

### Relationships

- `deg` — *the scalar angle type this vector is built from*
- `rad3` — *the radian-unit counterpart of this vector alias*
- `fdeg3` — *the float-specialized instantiation of this alias*

<!-- ink:api-end name="deg3" -->

<!-- ink:api name="degrees" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::degrees`

Internal tag type used to mark an `ang<T, TUnit>` instantiation as holding a degree value.

### Why this exists

`degrees` carries no data — it exists purely so the compiler can distinguish `deg<T>`
from `rad<T>` at the type level, the counterpart to `impl::radians`.

### Relationships

- `ang` — *the template this tag type parameterizes as `TUnit`*
- `deg` — *the public alias built by pairing `ang<T, impl::degrees>`*
- `radians` — *the sibling tag type for radian-unit values*

<!-- ink:api-end name="degrees" -->

<!-- ink:api name="dim_t" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `dim_t`

An alias for `std::size_t`, used as the dimension/index type for `vec` and `mat` templates.

### Why this exists

`dim_t` exists so that vector and matrix dimensions (`L`, `R`, `C`) and indices are expressed with one consistent, semantically named type across the library instead of scattering raw `std::size_t` or `int` through template parameters. This keeps `vec<L, T>` and `mat<R, C, T>` signatures self-documenting.

### Relationships

- `vec` — *templated on `dim_t L` for its component count.*
- `mat` — *templated on `dim_t R, dim_t C` for its row/column counts.*

<!-- ink:api-end name="dim_t" -->

<!-- ink:api name="fdeg" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fdeg`

Single-precision degree-based angle alias, used wherever rotations are expressed in degrees instead of radians.

### Why this exists

`fdeg` exists to keep unit semantics explicit at the type level — `deg<float>` self-documents that a value is an angle in degrees rather than an unadorned `float`, preventing accidental mixing of degree and radian values in arithmetic. It's the float-precision instantiation of the generic `deg<T>` template, mirroring the split between `fvec`/`fmat` and their generic counterparts elsewhere in `tdm::Types`.

### Relationships

- `deg<T>` — *generic template `fdeg` instantiates for `float`*
- `frad` — *radian counterpart; convert between the two rather than mixing units*
- `fdeg3` — *3-component vector of `fdeg` angles (e.g., Euler angles)*

<!-- ink:api-end name="fdeg" -->

<!-- ink:api name="fdeg3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fdeg3`

A 3-component vector of single-precision degree angles, typically used to represent Euler-angle rotations (pitch/yaw/roll) in degrees.

### Why this exists

`fdeg3` exists so a triple of Euler angles carries its unit (degrees) and precision (float) in the type itself, rather than being a bare `vec3<float>` that could be mistaken for radians or a position. It is `vec3<deg<float>>` under the hood, reusing the generic `vec3` container instead of a bespoke struct.

### Relationships

- `deg3<T>` — *generic template `fdeg3` instantiates for `float`*
- `frad3` — *radian counterpart for the same 3-component rotation representation*
- `fdeg` — *scalar angle type that composes into `fdeg3`*

<!-- ink:api-end name="fdeg3" -->

<!-- ink:api name="fmat" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fmat`

Generic alias template for a floating-point matrix of given row and column counts.

### Why this exists

`fmat` exists so floating-point matrices of different shapes share one alias template
rather than being independently defined, specializing `mat<R, C, T>` with `T = float`
for transforms and other continuous-valued matrices.

### Relationships

- `mat` — *the underlying generic template `fmat` specializes with `T = float`*
- `fmat2`, `fmat3`, `fmat4` — *fixed-shape aliases built from this template*
- `imat` — *the integer counterpart of this alias family*

<!-- ink:api-end name="fmat" -->

<!-- ink:api name="fmat2" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fmat2`

A 2x2 floating-point matrix.

### Relationships

- `fmat` — *the shape-parameterized template this alias fixes to 2x2*
- `fmat3`, `fmat4` — *sibling fixed-shape float matrix aliases*

<!-- ink:api-end name="fmat2" -->

<!-- ink:api name="fmat3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fmat3`

A 3x3 floating-point matrix.

### Relationships

- `fmat` — *the shape-parameterized template this alias fixes to 3x3*
- `fmat2`, `fmat4` — *sibling fixed-shape float matrix aliases*
- `coord_sys` — *a coordinate frame type commonly represented with a `fmat3`-like orientation*

<!-- ink:api-end name="fmat3" -->

<!-- ink:api name="fmat4" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fmat4`

A 4x4 floating-point matrix, the standard type for affine transforms.

### Relationships

- `fmat` — *the shape-parameterized template this alias fixes to 4x4*
- `fmat2`, `fmat3` — *sibling fixed-shape float matrix aliases*

<!-- ink:api-end name="fmat4" -->

<!-- ink:api name="fquat" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fquat`

Single-precision quaternion type, the standard rotation representation used throughout `tdm` for 3D orientation.

### Why this exists

`fquat` is the float-precision instantiation of the generic `quat<T>` template. Using a dedicated alias keeps rotation code readable and consistent with the rest of the library's `f`-prefixed float-precision aliases (`fvec`, `fmat`, `fdeg`).

### Relationships

- `quat<T>` — *generic template `fquat` instantiates for `float`*
- `fmat` — *matrix representation that a quaternion can be converted to*

<!-- ink:api-end name="fquat" -->

<!-- ink:api name="frad" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `frad`

Single-precision radian-based angle alias, used wherever rotations are expressed in radians instead of degrees.

### Why this exists

`frad` exists for the same reason as `fdeg` — to make the angle unit explicit at the type level and prevent radian/degree mixups in arithmetic. It is the float-precision instantiation of the generic `rad<T>` template.

### Relationships

- `rad<T>` — *generic template `frad` instantiates for `float`*
- `fdeg` — *degree counterpart; convert rather than mix units*
- `frad3` — *3-component vector of `frad` angles*

<!-- ink:api-end name="frad" -->

<!-- ink:api name="frad3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `frad3`

A 3-component vector of single-precision radian angles, typically used to represent Euler-angle rotations in radians.

### Why this exists

`frad3` mirrors `fdeg3` for radians — it is `vec3<rad<float>>`, keeping the unit and precision explicit in the type rather than relying on a bare `vec3<float>`.

### Relationships

- `rad3<T>` — *generic template `frad3` instantiates for `float`*
- `fdeg3` — *degree counterpart for the same rotation representation*
- `frad` — *scalar angle type that composes into `frad3`*

<!-- ink:api-end name="frad3" -->

<!-- ink:api name="fvec" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fvec`

Generic alias template for a floating-point vector of a given dimension.

### Why this exists

`fvec` exists so floating-point vector types of different lengths share one template
rather than being independently defined, mirroring `ivec` but with `T = float` for
positions, directions, and other continuous values.

### Relationships

- `vec` — *the underlying generic template `fvec` specializes with `T = float`*
- `fvec2`, `fvec3`, `fvec4` — *fixed-length aliases built from this template*
- `ivec` — *the integer counterpart of this alias family*

<!-- ink:api-end name="fvec" -->

<!-- ink:api name="fvec2" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fvec2`

A 2-component floating-point vector.

### Construction

```cpp
fvec2 uv{ 0.5f, 0.25f };
```

### Relationships

- `fvec` — *the length-parameterized template this alias fixes to length 2*
- `fvec3`, `fvec4` — *sibling fixed-length float vector aliases*

<!-- ink:api-end name="fvec2" -->

<!-- ink:api name="fvec3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fvec3`

A 3-component floating-point vector.

### Construction

```cpp
fvec3 position{ 0.0f, 1.5f, -3.2f };
```

### Relationships

- `fvec` — *the length-parameterized template this alias fixes to length 3*
- `fvec2`, `fvec4` — *sibling fixed-length float vector aliases*
- `rad3`, `deg3` — *angle-typed vec3 specializations used for Euler rotations*

<!-- ink:api-end name="fvec3" -->

<!-- ink:api name="fvec4" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `fvec4`

A 4-component floating-point vector.

### Construction

```cpp
fvec4 color{ 1.0f, 0.0f, 0.0f, 1.0f };
```

### Relationships

- `fvec` — *the length-parameterized template this alias fixes to length 4*
- `fvec2`, `fvec3` — *sibling fixed-length float vector aliases*

<!-- ink:api-end name="fvec4" -->

<!-- ink:api name="imat" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `imat`

Generic alias template for an integer matrix of given row and column counts.

### Why this exists

`imat` exists so integer matrices of different shapes share one alias template
rather than being independently defined, specializing `mat<R, C, T>` with `T = int`.

### Relationships

- `mat` — *the underlying generic template `imat` specializes with `T = int`*
- `imat2`, `imat3`, `imat4` — *fixed-shape aliases built from this template*
- `fmat` — *the floating-point counterpart of this alias family*

<!-- ink:api-end name="imat" -->

<!-- ink:api name="imat2" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `imat2`

A 2x2 integer matrix.

### Relationships

- `imat` — *the shape-parameterized template this alias fixes to 2x2*
- `imat3`, `imat4` — *sibling fixed-shape integer matrix aliases*

<!-- ink:api-end name="imat2" -->

<!-- ink:api name="imat3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `imat3`

A 3x3 integer matrix.

### Relationships

- `imat` — *the shape-parameterized template this alias fixes to 3x3*
- `imat2`, `imat4` — *sibling fixed-shape integer matrix aliases*

<!-- ink:api-end name="imat3" -->

<!-- ink:api name="imat4" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `imat4`

A 4x4 integer matrix.

### Relationships

- `imat` — *the shape-parameterized template this alias fixes to 4x4*
- `imat2`, `imat3` — *sibling fixed-shape integer matrix aliases*

<!-- ink:api-end name="imat4" -->

<!-- ink:api name="ivec" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ivec`

Generic alias template for an integer vector of a given dimension.

### Why this exists

`ivec` exists so integer vector types of different lengths (2, 3, 4) share one template
rather than being independently defined. It separates the "integer" element type from the
generic `vec<L, T>` template used for all vector element types.

### Relationships

- `vec` — *the underlying generic template `ivec` specializes with `T = int`*
- `ivec2`, `ivec3`, `ivec4` — *fixed-length aliases built from this template*
- `fvec` — *the floating-point counterpart of this alias family*

<!-- ink:api-end name="ivec" -->

<!-- ink:api name="ivec2" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ivec2`

A 2-component integer vector.

### Construction

```cpp
ivec2 pixel_coord{ 12, 34 };
```

### Relationships

- `ivec` — *the length-parameterized template this alias fixes to length 2*
- `ivec3`, `ivec4` — *sibling fixed-length integer vector aliases*

<!-- ink:api-end name="ivec2" -->

<!-- ink:api name="ivec3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ivec3`

A 3-component integer vector.

### Construction

```cpp
ivec3 voxel_coord{ 1, 2, 3 };
```

### Relationships

- `ivec` — *the length-parameterized template this alias fixes to length 3*
- `ivec2`, `ivec4` — *sibling fixed-length integer vector aliases*

<!-- ink:api-end name="ivec3" -->

<!-- ink:api name="ivec4" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ivec4`

A 4-component integer vector.

### Construction

```cpp
ivec4 rect{ 0, 0, 128, 128 };
```

### Relationships

- `ivec` — *the length-parameterized template this alias fixes to length 4*
- `ivec2`, `ivec3` — *sibling fixed-length integer vector aliases*

<!-- ink:api-end name="ivec4" -->

<!-- ink:api name="mat" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `mat`

Generic matrix template parameterized by row count, column count, and element type.

### Why this exists

`mat` exists as the single generic definition backing every fixed-size matrix alias
(`mat2`, `mat3`, `mat4`, and their `int`/`float` specializations `imat*`/`fmat*`).
Keeping row/column/element-type as template parameters avoids duplicating matrix
logic per size or type.

### Relationships

- `mat2`, `mat3`, `mat4` — *square-matrix aliases built from this template*
- `imat`, `fmat` — *element-type-specialized alias templates built on `mat`*

<!-- ink:api-end name="mat" -->

<!-- ink:api name="mat2" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `mat2`

A generic 2x2 matrix, parameterized only by element type `T`.

### Relationships

- `mat` — *the underlying template this alias fixes to a 2x2 shape*
- `mat3`, `mat4` — *sibling square matrix aliases*
- `imat2`, `fmat2` — *fully-specialized int/float versions of this alias*

<!-- ink:api-end name="mat2" -->

<!-- ink:api name="mat3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `mat3`

A generic 3x3 matrix, parameterized only by element type `T`.

### Relationships

- `mat` — *the underlying template this alias fixes to a 3x3 shape*
- `mat2`, `mat4` — *sibling square matrix aliases*
- `imat3`, `fmat3` — *fully-specialized int/float versions of this alias*

<!-- ink:api-end name="mat3" -->

<!-- ink:api name="mat4" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `mat4`

A generic 4x4 matrix, parameterized only by element type `T`.

### Relationships

- `mat` — *the underlying template this alias fixes to a 4x4 shape*
- `mat2`, `mat3` — *sibling square matrix aliases*
- `imat4`, `fmat4` — *fully-specialized int/float versions of this alias, commonly used for transforms*

<!-- ink:api-end name="mat4" -->

<!-- ink:api name="quat" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `quat`

Generic quaternion template used to represent 3D rotations without the gimbal-lock and interpolation problems of Euler angles.

### Why this exists

`quat` exists because Euler-angle triples (`deg3`/`rad3`) are unsuitable for interpolation and composition — quaternions give stable, commutative-friendly rotation composition and smooth interpolation (e.g., slerp). It is declared here as a forward declaration; the float specialization `fquat` is the commonly used instantiation.

### Relationships

- `fquat` — *float-precision instantiation of this template*
- `fdeg3` / `frad3` — *Euler-angle representations that quaternions are often converted from/to*

<!-- ink:api-end name="quat" -->

<!-- ink:api name="rad" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `rad`

Alias for an angle value held in radians, `ang<T, impl::radians>`.

### Construction

```cpp
frad rotation{ 1.5708f };
```

### Relationships

- `ang` — *the underlying generic angle template this alias specializes*
- `deg` — *the degree-unit counterpart of this alias*
- `rad3`, `frad`, `frad3` — *vector and float-specialized forms built on this alias*

<!-- ink:api-end name="rad" -->

<!-- ink:api name="rad3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `rad3`

A 3-component vector of radian-unit angles, used for Euler rotations expressed in radians.

### Construction

```cpp
frad3 euler{ frad{0.0f}, frad{0.785f}, frad{1.571f} };
```

### Relationships

- `rad` — *the scalar angle type this vector is built from*
- `deg3` — *the degree-unit counterpart of this vector alias*
- `frad3` — *the float-specialized instantiation of this alias*

<!-- ink:api-end name="rad3" -->

<!-- ink:api name="radians" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::radians`

Internal tag type used to mark an `ang<T, TUnit>` instantiation as holding a radian value.

### Why this exists

`radians` carries no data — it exists purely so the compiler can distinguish `rad<T>`
from `deg<T>` at the type level. This is what lets `ang` catch unit mismatches (e.g.
passing degrees where radians are expected) as compile errors rather than runtime bugs.

### Relationships

- `ang` — *the template this tag type parameterizes as `TUnit`*
- `rad` — *the public alias built by pairing `ang<T, impl::radians>`*
- `degrees` — *the sibling tag type for degree-unit values*

<!-- ink:api-end name="radians" -->

<!-- ink:api name="rot_dir" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `rot_dir`

Enumerates whether a rotation is applied in the positive or negative direction around an axis.

### Why this exists

Rotation sign convention (clockwise vs. counterclockwise) is another implicit assumption that varies between coordinate systems and tools; `rot_dir` makes it explicit per-axis rather than assumed globally.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `negative` | enumerator (`-1`) | Rotation applied in the negative direction. |
| `positive` | enumerator (`1`) | Rotation applied in the positive direction. |

### Relationships

- `rot_sign` — *bundles a `rot_dir` value per axis (x, y, z)*
- `rot_seq` — *specifies the axis order that `rot_dir` signs apply to*

<!-- ink:api-end name="rot_dir" -->

<!-- ink:api name="rot_seq" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `rot_seq`

Enumerates the six orderings in which Euler-angle rotations around the X, Y, and Z axes can be composed.

### Why this exists

Euler-angle rotations are not commutative — the result differs depending on the order the axis rotations are applied in. `rot_seq` makes that order an explicit, checkable parameter instead of an implicit convention baked into calling code, so conversions between Euler angles and other representations (quaternions, matrices) can be done unambiguously.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `xyz`, `xzy`, `yxz`, `yzx`, `zxy`, `zyx` | enumerators | The six possible axis-rotation application orders. |

### Relationships

- `fdeg3` / `frad3` — *Euler-angle triples whose composition order this enum specifies*
- `rot_dir` — *direction convention applied alongside the rotation sequence*

<!-- ink:api-end name="rot_seq" -->

<!-- ink:api name="rot_sign" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `rot_sign`

Bundles the rotation direction convention for all three axes (x, y, z) of a coordinate system.

### Why this exists

Rather than tracking three separate `rot_dir` values, `rot_sign` groups them into one value representing a coordinate system's complete sign convention, which can then be passed around and compared as a unit alongside `rot_seq` and `chirality`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `rot_dir` | Rotation direction convention for the X axis. |
| `y` | `rot_dir` | Rotation direction convention for the Y axis. |
| `z` | `rot_dir` | Rotation direction convention for the Z axis. |

### Relationships

- `rot_dir` — *the per-axis value this struct bundles*
- `coord_sys` — *coordinate system definition that likely carries a `rot_sign`*

<!-- ink:api-end name="rot_sign" -->

<!-- ink:api name="vec" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `vec<dim_t L, typename T>`

The base fixed-size vector template, parameterized on component count `L` and scalar type `T`.

### Why this exists

`vec` exists to give fixed-size numeric arrays (positions, directions, colors) a single generic implementation instead of writing separate `vec2`/`vec3`/`vec4` structs by hand. `vec2`, `vec3`, and `vec4` are just convenience aliases (`vec<2, T>`, `vec<3, T>`, `vec<4, T>`) over this one template, so all dimensions share the same operations.

### Relationships

- `vec2` — *alias for `vec<2, T>`.*
- `vec3` — *alias for `vec<3, T>`.*
- `vec4` — *alias for `vec<4, T>`.*
- `dim_t` — *the type used for the `L` template parameter.*

<!-- ink:api-end name="vec" -->

<!-- ink:api name="vec2" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `vec2<T> = vec<2, T>`

A 2-component vector, for values like 2D coordinates or UVs.

### Relationships

- `vec` — *the underlying generic template `vec2` aliases.*
- `ivec2`, `fvec2` — *common `vec2` instantiations for `int` and `float`.*

<!-- ink:api-end name="vec2" -->

<!-- ink:api name="vec3" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `vec3<T> = vec<3, T>`

A 3-component vector, used throughout `tdm` for positions, directions, and scale factors.

### Relationships

- `vec` — *the underlying generic template `vec3` aliases.*
- `ivec3`, `fvec3` — *common `vec3` instantiations for `int` and `float`.*
- `rad3`, `deg3` — *3-component vectors of angle types, built on `vec3`.*

<!-- ink:api-end name="vec3" -->

<!-- ink:api name="vec4" module="tdm/Types" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `vec4<T> = vec<4, T>`

A 4-component vector, used for homogeneous coordinates (e.g. `[x, y, z, w]`) and quaternion-adjacent storage.

### Relationships

- `vec` — *the underlying generic template `vec4` aliases.*
- `ivec4`, `fvec4` — *common `vec4` instantiations for `int` and `float`.*

<!-- ink:api-end name="vec4" -->
