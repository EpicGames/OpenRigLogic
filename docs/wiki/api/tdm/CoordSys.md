# API Reference — `tdm/CoordSys`

---

<!-- ink:api name="axis_vector" module="tdm/CoordSys" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> vec3<T> axis_vector(axis_dir axis)`

Convert a named axis direction (`right`, `up`, `front`, etc.) into its unit vector in the canonical coordinate system.

### When to use this

Use this internally when building a `coord_sys`'s basis vectors from its named axis directions — it is the lookup step that turns a symbolic direction into a concrete `vec3<T>`.

### Example

```cpp
tdm::vec3<float> upVec = tdm::impl::axis_vector<float>(tdm::axis_dir::up);
// upVec is {0.0f, 1.0f, 0.0f}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `axis` | `axis_dir` | required — one of `right`, `left`, `up`, `down`, `front`, `back` |

### Returns

`vec3<T>` — the canonical unit vector for the given axis direction; a zero vector for an unrecognized value.

### Watch out for

- An `axis_dir` value not covered by the switch (e.g., a future enum extension or an uninitialized value) returns the zero vector `{0, 0, 0}` silently — no assertion or exception is raised. Validate `axis_dir` values before calling if the input is not a compile-time constant.

<!-- ink:api-end name="axis_vector" -->

<!-- ink:api name="coord_sys" module="tdm/CoordSys" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `coord_sys`

Describes a 3D coordinate system by naming what each of its X, Y, and Z axes points toward (e.g. right, up, front).

### Why this exists

`coord_sys` exists because raw basis matrices don't communicate intent — a `mat3` alone doesn't tell you which axis is "up" or whether the system is right- or left-handed. By storing named axis directions instead, this type makes coordinate-system conversions between engines (which often disagree on axis conventions) explicit and self-documenting, and lets `handedness()` and `valid()` derive physically meaningful answers from the same declarative description.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `axis_dir` | required — canonical direction the local X axis points toward |
| `y` | `axis_dir` | required — canonical direction the local Y axis points toward |
| `z` | `axis_dir` | required — canonical direction the local Z axis points toward |

### Construction

```cpp
tdm::coord_sys unrealCoordSys{tdm::axis_dir::right, tdm::axis_dir::up, tdm::axis_dir::front};
bool isValid = unrealCoordSys.valid<float>();
tdm::chirality hand = unrealCoordSys.handedness<float>();
```

### Relationships

- `axis_vector` — *resolves each named axis into its canonical unit vector when building `basis()`*
- `determinant` — *used by `handedness()` and `valid()` to test orthonormality and sign*

### Watch out for

- `handedness()` returns `chirality::right` when the basis determinant is **negative**, and `chirality::left` when it is **positive** — the inverse of the standard mathematical convention. This is intentional to match the legacy DNA naming scheme. Do not replicate this inversion if you compute handedness from `basis()` directly.

<!-- ink:api-end name="coord_sys" -->
