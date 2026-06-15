# API Reference — `tdm/CoordSys`

---

<!-- ink:api name="axis_vector" module="tdm/CoordSys" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> vec3<T> axis_vector(axis_dir axis)`

Map a named axis direction to its unit vector in canonical coordinates.

### When to use this

When you hold an `axis_dir` value (for example, from a `coord_sys` field) and need the corresponding unit `vec3` to project onto an axis or build a transform. `coord_sys::basis()` calls this internally; reach for it directly only when you need a single axis vector rather than the full 3×3 basis.

### Example

```cpp
// Retrieve the canonical unit vector for the "right" direction
vec3<float> right = tdm::impl::axis_vector<float>(axis_dir::right);
// right == {1.0f, 0.0f, 0.0f}

vec3<double> down = tdm::impl::axis_vector<double>(axis_dir::down);
// down == {0.0, -1.0, 0.0}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `axis` | `axis_dir` | required — the semantic axis direction to convert |

### Returns

`vec3<T>` — the unit vector for the given direction in canonical coordinates. All six `axis_dir` values (`right`, `left`, `up`, `down`, `front`, `back`) map to a distinct unit vector.

### Watch out for

- An `axis_dir` value not covered by the switch (e.g., a future enum extension or an uninitialized value) returns the zero vector `{0, 0, 0}` silently — no assertion or exception is raised. Validate `axis_dir` values before calling if the input is not a compile-time constant.

<!-- ink:api-end name="axis_vector" -->

<!-- ink:api name="coord_sys" module="tdm/CoordSys" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `coord_sys`

Describes a 3D coordinate system as three named axis directions, carrying spatial orientation semantics separately from the transformation matrices derived from it.

### Why this exists

Using a raw `mat3` or three plain vectors to describe a coordinate frame loses the semantic meaning of which physical direction each axis represents. `coord_sys` makes the axis-direction binding explicit and provides `basis()`, `handedness()`, and `valid()` to derive correct transformation data from those semantic labels. It also encodes a deliberate naming inversion: because legacy DNAs encode coordinate systems where `front` means "into screen" (opposite the mathematical right-hand rule), `handedness()` inverts its determinant-sign logic to return the physically correct chirality — callers who compute handedness manually from `basis()` will get the opposite sign.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `axis_dir` | required — the physical direction the local X axis points in canonical space |
| `y` | `axis_dir` | required — the physical direction the local Y axis points in canonical space |
| `z` | `axis_dir` | required — the physical direction the local Z axis points in canonical space |

### Construction

```cpp
// Right-handed system using legacy DNA naming: X=right, Y=up, Z=back
tdm::coord_sys cs{ axis_dir::right, axis_dir::up, axis_dir::back };

// Derive the basis matrix (row-vector convention: v_dst = v_src * C)
tdm::mat3<float> B = cs.basis<float>();

// Query handedness — returns chirality::right for this configuration
// (note: inverted sign convention vs raw determinant — see Why this exists)
tdm::chirality h = cs.handedness<float>();

// Confirm the system is orthonormal (|det| ≈ 1)
bool ok = cs.valid<float>();  // true

// Equality comparison covers all three axis fields
tdm::coord_sys other{ axis_dir::right, axis_dir::up, axis_dir::back };
bool same = (cs == other);  // true
```

### Relationships

- `axis_dir` — enum whose values populate the `x`, `y`, `z` fields
- `chirality` — return type of `handedness()`; conveys left- or right-handedness
- `axis_vector` (`tdm::impl`) — used internally by `basis()` to convert each `axis_dir` to a unit `vec3`
- `mat3` — return type of `basis()`; consumed by transform pipelines

### Watch out for

- `handedness()` returns `chirality::right` when the basis determinant is **negative**, and `chirality::left` when it is **positive** — the inverse of the standard mathematical convention. This is intentional to match the legacy DNA naming scheme. Do not replicate this inversion if you compute handedness from `basis()` directly.

<!-- ink:api-end name="coord_sys" -->
