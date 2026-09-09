# API Reference — `tdm/Mat`

---

<!-- ink:api name="applied" module="tdm/Mat" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t R, dim_t C, typename T, typename F> mat<R, C, T> applied(const mat<R, C, T>& lhs, F func)`

Apply a function to every element of a matrix, returning a new matrix with the results — the source matrix is left unchanged.

### When to use this

Use this when you need an element-wise transform (e.g. clamping, rounding, or a custom per-component operation) without mutating the original matrix. If mutating in place is acceptable, call `apply` directly on the matrix instead.

### Example

```cpp
mat3<float> m = mat3<float>::identity();
mat3<float> doubled = applied(m, [](float& value, dim_t /*unused*/) { value *= 2.0f; });
// doubled now holds the element-wise result; m is untouched
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `lhs` | `const mat<R, C, T>&` | required — the source matrix; copied before the function is applied. |
| `func` | `F` | required — a callable invoked per element (and, depending on `apply`'s contract, per row) to transform the value in place on the copy. |

### Returns

`mat<R, C, T>` — a new matrix holding the result of applying `func` to every element of `lhs`.

<!-- ink:api-end name="applied" -->


<!-- ink:api name="inverse" module="tdm/Mat" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> mat<N, N, T> inverse(const mat<N, N, T>& m)`

Forward declaration of the matrix inverse operation.

### When to use this

This forward declaration lets `mat`-related code reference `tdm::inverse` before its full definition is visible. Call the fully-defined `inverse` (documented under `tdm/Computations`) for actual use.

### Constraints

- Only callable on square matrices (`N×N`). Rectangular matrices do not have an overload — the template parameter forces this at compile time.
- Behavior is undefined (or numerically degenerate) for singular matrices (determinant = 0). Check invertibility with a determinant check before calling if the input is not guaranteed to be non-singular.

<!-- ink:api-end name="inverse" -->

<!-- ink:api name="is_all_scalar" module="tdm/Mat" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `is_all_scalar`

A compile-time trait that checks whether a parameter pack consists entirely of the same scalar type.

### Why this exists

`is_all_scalar` exists to validate, at compile time, that the variadic scalar constructor of `mat` (`mat(Us... scalars)`) is only enabled when every argument is a scalar of a single consistent type — preventing accidental construction from a mismatched or non-scalar argument list without a confusing template error deep in `mat`'s internals.

### Fields

| Name | Type | Description |
|------|------|-------------|
| (trait, no data members) | — | Inherits from `std::true_type` or `std::false_type` depending on whether the pack `Us...` is all the same scalar type `T` |

### Constraints

- Resides in `tdm::impl` — treat as internal. Do not specialize or depend on it directly.
- Empty pack specialization (`is_all_scalar<>`) always yields `false_type` — zero arguments are not considered all-scalar.

<!-- ink:api-end name="is_all_scalar" -->

<!-- ink:api name="mat" module="tdm/Mat" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `mat`

A fixed-size, generic `R`×`C` matrix template parameterized on element type, used throughout `tdm` for linear transforms.

### Why this exists

`mat` exists to give the library a single, generically-sized matrix type instead of separate hand-written 2x2/3x3/4x4 types, while still supporting row-based storage, scalar/list construction, and conversion between element types. Storing data as an array of `row_type` vectors (rather than a flat array) lets row access and per-row `apply` operations stay type-safe and simple.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `values` | `row_type[R]` (private) | Row-major storage; each element is a `vec<C, T>` representing one row |

### Construction

```cpp
tdm::mat<3, 3, float> identity = tdm::mat<3, 3, float>::diagonal(1.0f);
tdm::mat<2, 2, float> m{1.0f, 2.0f, 3.0f, 4.0f};  // row-major scalar list
tdm::vec2<float> row0{1.0f, 0.0f}, row1{0.0f, 1.0f};
tdm::mat<2, 2, float> fromRows = tdm::mat<2, 2, float>::from_rows(row0, row1);
```

### Relationships

- `transpose` — *used by `from_columns` to build a matrix from column vectors instead of rows*
- `is_all_scalar` — *gates the variadic scalar constructor to a single consistent scalar type*
- `inverse` — *forward-declared here, defined in `tdm/Computations` for inverting square instances*

### Constraints

- `diagonal()` and `inverse()` are only available for square matrices (`R == C`). The compiler enforces this via `std::enable_if<H == W>`.
- The variadic scalar constructor requires exactly `R * C` arguments all of the same scalar type. Passing the wrong count or mixed types is a compile error.

<!-- ink:api-end name="mat" -->


<!-- ink:api name="transpose" module="tdm/Mat" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t R, dim_t C, typename T> mat<C, R, T> transpose(const mat<R, C, T>& m)`

Forward declaration of the matrix transpose operation, used internally by `mat::from_columns`.

### When to use this

This forward declaration exists so `mat` can call `tdm::transpose` from within `from_columns` before the full definition (in `Computations.h`) is visible. Call the fully-defined `transpose` (documented under `tdm/Computations`) rather than relying on this declaration directly.

<!-- ink:api-end name="transpose" -->
