# API Reference — `tdm/Vec`

---

<!-- ink:api name="value_type" module="tdm/Vec" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `value_type`

Member type alias that exposes the element type `T` of a `vec<L, T>` instantiation for use in generic template code.

### Why this exists

`value_type` follows the standard C++ named-requirement convention (same as `std::vector<T>::value_type`), allowing generic algorithms and traits to query the scalar element type of any `vec<L, T>` without knowing the full specialization. This makes `vec` compatible with standard type-trait patterns such as `std::is_same<typename SomeVec::value_type, float>` and template deduction helpers.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` | The scalar element type of this `vec` instantiation (e.g., `float` for `vec<3, float>`) |

### Relationships

- `vec<L, T>` — the enclosing struct that declares this alias
- `values[L]` — the underlying storage array whose element type is `value_type`

<!-- ink:api-end name="value_type" -->

<!-- ink:api name="vec" module="tdm/Vec" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<dim_t L, typename T> struct vec`

Fixed-dimension vector template for element-wise arithmetic, indexing, and functional transforms over a homogeneous array of `L` values. Use this as the foundation for typed spatial vectors (`vec<3, float>` for positions/normals, `vec<4, float>` for homogeneous coordinates) or any fixed-width numeric tuple in the `tdm` math layer.

### When to use this

Reach for `vec<L, T>` when you need a compile-time-sized numeric tuple with element access, broadcast construction, and functional transforms — and you want value semantics with copy/move naturally handled. Use it in preference to raw arrays when you need `operator[]` bounds-checking in debug builds or need to pass a typed vector by value without pointer indirection.

### Example

```cpp
// Construct a 3-component float vector from individual values
tdm::vec<3, float> position{1.0f, 2.5f, -0.5f};

// Broadcast: fill all L elements with a single scalar
tdm::vec<4, float> ones{1.0f};  // {1, 1, 1, 1}

// Construct from a raw float array (e.g., data read from a DNA stream)
float raw[3] = {0.1f, 0.2f, 0.3f};
tdm::vec<3, float> from_raw{raw};

// Element access
float x = position[0];

// Apply a functional transform in-place (clamp all components)
position.apply([](float& v, tdm::dim_t /*i*/) {
    v = std::max(0.0f, v);
});

// Convert element type: vec<3, float> -> vec<3, double>
tdm::vec<3, double> as_double = position;
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `L` | `dim_t` | required — compile-time dimension count; fixed at instantiation |
| `T` | typename | required — element type (e.g., `float`, `double`, `int`) |

### Watch out for

- `operator[]` calls `assert(index < dimensions())` — out-of-bounds access aborts in debug builds but has undefined behavior in release builds. Validate indices before indexing in hot paths.
- The broadcast constructor (`explicit vec(U v)`) is only enabled when `L > 1`. For `vec<1, T>`, use the variadic or pointer constructor instead.
- The cross-type copy constructor (`vec(const vec<L, U>&)`) performs element-wise copy via `std::copy` — narrowing conversions (e.g., `double` to `float`) will silently truncate.

<!-- ink:api-end name="vec" -->
