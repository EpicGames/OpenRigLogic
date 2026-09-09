# API Reference — `tdm/Vec`

---


<!-- ink:api name="vec" module="tdm/Vec" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `vec`

Fixed-size, dimension-generic vector type used throughout `tdm` for positions, directions, and other L-dimensional float/int data.

### Why this exists

`vec<L, T>` exists so 2D, 3D, and 4D vectors (and their `float`/`int` variants) share one implementation instead of separate hand-written types, while still exposing a dimension-specific, compile-time-checked `L` via `dimensions()`. Converting between element types (e.g., `int` to `float`) is handled by a templated converting constructor/assignment rather than requiring manual casts at every call site.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `values` | `value_type[L]` | The `L` raw component values (e.g., x, y, z for `L == 3`). |

### Construction

```cpp
// Aggregate-style construction with one value per dimension
tdm::vec<3, float> position(1.0f, 2.0f, 3.0f);

// Broadcast a single scalar to all components (L > 1)
tdm::vec<3, float> origin(0.0f);

// Convert from an int vector to a float vector
tdm::vec<3, int> ipos(1, 2, 3);
tdm::vec<3, float> fpos(ipos);
```

### Relationships

- `fvec3` / `ivec3` — *common float/int instantiations of `vec` for 3 dimensions*
- `dim_t` — *type used for the dimension count `L` and for indexing*

### Watch out for

- `operator[]` calls `assert(index < dimensions())` — out-of-bounds access aborts in debug builds but has undefined behavior in release builds. Validate indices before indexing in hot paths.
- The broadcast constructor (`explicit vec(U v)`) is only enabled when `L > 1`. For `vec<1, T>`, use the variadic or pointer constructor instead.
- The cross-type copy constructor (`vec(const vec<L, U>&)`) performs element-wise copy via `std::copy` — narrowing conversions (e.g., `double` to `float`) will silently truncate.

<!-- ink:api-end name="vec" -->
