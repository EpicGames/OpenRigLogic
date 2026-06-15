# API Reference — `tdm/Mat`

---

<!-- ink:api name="applied" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `mat<R, C, T> applied(const mat<R, C, T>& lhs, F func)`

Apply a functor to every row of a matrix and return the modified copy, leaving the original unchanged.

### When to use this

Use when you need a functional-style transform over a matrix's rows without mutating the source. `applied` is the non-mutating counterpart to `mat::apply` — reach for it in expressions and pipelines where you want a new matrix value rather than in-place modification of an existing one.

### Example

```cpp
// Negate every row of a 3×3 matrix
tdm::mat<3, 3, float> src = tdm::mat<3, 3, float>::diagonal(1.0f);
tdm::mat<3, 3, float> negated = tdm::applied(src, [](tdm::vec<3, float>& row, tdm::dim_t /*i*/) {
    row = row * -1.0f;
});
// src is unchanged; negated has all elements negated
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `lhs` | `const mat<R, C, T>&` | required — the source matrix to copy and transform. |
| `func` | `F` | required — a callable with signature `void(row_type&, dim_t row_index)` applied to each row in order. |

### Returns

`mat<R, C, T>` — a new matrix that is a copy of `lhs` with `func` applied to each row.

<!-- ink:api-end name="applied" -->

<!-- ink:api name="column_type" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>::column_type`

Type alias for a single column of a `mat<R, C, T>` matrix — a `vec<R, T>` vector with R elements.

### Why this exists

Names the column vector type with R components, which is distinct from `row_type` in non-square matrices. The `rows()` static method returns `column_type::dimensions()` (R), making explicit that the row count equals the column vector height. Generic code receiving a column vector from matrix operations can use this alias to determine the expected type without inspecting the R template parameter directly.

### Relationships

- `row_type` — `vec<C, value_type>`: the corresponding row vector type.
- `from_columns` — factory method that takes `C` arguments of this type.

<!-- ink:api-end name="column_type" -->

<!-- ink:api name="inverse" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `mat<N, N, T> inverse(const mat<N, N, T>& m)`

Compute the inverse of a square matrix — the matrix M⁻¹ such that M × M⁻¹ = I.

### When to use this

Use when you need to reverse a transformation: undo a world-to-camera transform, solve a linear system, or recover original coordinates from a transformed space. For orthogonal matrices (rotation-only, no scale), prefer `transpose` — it produces the same result with no division and no risk of numerical instability.

### Example

```cpp
// Invert a 4×4 transform matrix
tdm::mat<4, 4, float> world_to_camera = /* camera transform */;
tdm::mat<4, 4, float> camera_to_world = tdm::inverse(world_to_camera);
// Apply camera_to_world to bring a camera-space point back to world space
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — the square matrix to invert. Must be N×N (enforced by template). |

### Returns

`mat<N, N, T>` — the matrix inverse of the input.

### Constraints

- Only callable on square matrices (`N×N`). Rectangular matrices do not have an overload — the template parameter forces this at compile time.
- Behavior is undefined (or numerically degenerate) for singular matrices (determinant = 0). Check invertibility with a determinant check before calling if the input is not guaranteed to be non-singular.

<!-- ink:api-end name="inverse" -->

<!-- ink:api name="is_all_scalar" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `is_all_scalar<T, Us...>`

Internal trait that checks whether every type in a variadic parameter pack is a scalar type. Lives in `tdm::impl` — not part of the public API surface.

### Why this exists

The `mat` constructors need a compile-time guard that permits variadic scalar packs (e.g., `mat(1.0f, 0.0f, 0.0f, …)`) only when all arguments share the same scalar type. A plain `std::is_scalar` check cannot span a heterogeneous pack, so this recursive trait accumulates the check across all types in the pack before enabling the constructor via `std::enable_if`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| (base) | `std::true_type` or `std::false_type` | Inherits `true_type` when all types in the pack are scalar and homogeneous; `false_type` otherwise. |

### Constraints

- Resides in `tdm::impl` — treat as internal. Do not specialize or depend on it directly.
- Empty pack specialization (`is_all_scalar<>`) always yields `false_type` — zero arguments are not considered all-scalar.

<!-- ink:api-end name="is_all_scalar" -->

<!-- ink:api name="mat" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>`

A fixed-size R×C matrix of scalar type T with full construction, factory, and arithmetic support.

### Why this exists

`mat<R, C, T>` encodes matrix dimensions as compile-time template parameters, eliminating entire classes of shape-mismatch bugs that a runtime matrix type cannot catch. The template structure ensures that `transpose`, `inverse`, and multiplication are only callable when dimensions are compatible — the compiler rejects invalid operations before the program runs. This is the primary linear-algebra primitive in the `tdm` library; `vec<N, T>` is a specialization of its row and column types.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` | The scalar element type. |
| `row_type` | `vec<C, T>` | Type of a single row (a C-dimensional vector). |
| `column_type` | `vec<R, T>` | Type of a single column (an R-dimensional vector). |

### Construction

```cpp
// Zero-initialize a 3×3 float matrix
tdm::mat<3, 3, float> identity{};

// Fill all elements with a scalar
tdm::mat<3, 3, float> scaled(2.0f);

// Element-by-element (row-major order)
tdm::mat<2, 2, float> m(1.0f, 0.0f,
                         0.0f, 1.0f);

// From row vectors
auto rot = tdm::mat<3, 3, float>::from_rows(
    tdm::vec<3, float>{1.0f, 0.0f, 0.0f},
    tdm::vec<3, float>{0.0f, 0.866f, -0.5f},
    tdm::vec<3, float>{0.0f, 0.5f,  0.866f}
);

// From column vectors
auto basis = tdm::mat<3, 3, float>::from_columns(xAxis, yAxis, zAxis);

// Diagonal (square matrices only)
auto uniform_scale = tdm::mat<4, 4, float>::diagonal(2.0f);
```

### Relationships

- `vec<N, T>` — row and column element type; `row_type` and `column_type` are both `vec` instances.
- `transpose` — free function that produces `mat<C, R, T>` from `mat<R, C, T>`.
- `inverse` — free function for square matrix inversion.
- `applied` — free function that applies a functor to a copy of the matrix.

### Constraints

- `diagonal()` and `inverse()` are only available for square matrices (`R == C`). The compiler enforces this via `std::enable_if<H == W>`.
- The variadic scalar constructor requires exactly `R * C` arguments all of the same scalar type. Passing the wrong count or mixed types is a compile error.

<!-- ink:api-end name="mat" -->

<!-- ink:api name="row_type" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>::row_type`

Type alias for a single row of a `mat<R, C, T>` matrix — a `vec<C, T>` vector with C elements.

### Why this exists

Names the concrete row vector type so that generic algorithms and `from_rows` factory callers can construct row arguments at the correct width without repeating the column dimension. The internal `values[R]` storage array is an array of `row_type`, so iterating rows operates directly on this type.

### Relationships

- `column_type` — `vec<R, value_type>`: the corresponding column vector type.
- `from_rows` — factory method that takes `R` arguments of this type.

<!-- ink:api-end name="row_type" -->

<!-- ink:api name="transpose" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `mat<C, R, T> transpose(const mat<R, C, T>& m)`

Produce the transpose of a matrix — swap rows and columns to get a `mat<C, R, T>` from a `mat<R, C, T>` input.

### When to use this

Use when you need to convert between row-major and column-major representations, solve a system Aᵀx = b, or construct a matrix from column vectors using `from_columns` (which calls `transpose` internally). For square matrices, `transpose` also undoes the effect of `inverse` for orthogonal matrices without the cost of a full inversion.

### Example

```cpp
// Transpose a 3×2 matrix to get a 2×3 result
tdm::mat<3, 2, float> m = tdm::mat<3, 2, float>::from_rows(
    tdm::vec<2, float>{1.0f, 2.0f},
    tdm::vec<2, float>{3.0f, 4.0f},
    tdm::vec<2, float>{5.0f, 6.0f}
);
tdm::mat<2, 3, float> mt = tdm::transpose(m);
// mt rows are now: {1, 3, 5} and {2, 4, 6}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<R, C, T>&` | required — the source matrix to transpose. |

### Returns

`mat<C, R, T>` — a new matrix whose row `i` equals column `i` of the input.

<!-- ink:api-end name="transpose" -->

<!-- ink:api name="value_type" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>::value_type`

Type alias for the scalar element type `T` of a `mat<R, C, T>` matrix.

### Why this exists

Follows the C++ standard container convention so that generic code can write `typename Mat::value_type` to recover the element type without knowing the concrete instantiation. This enables template utilities to operate uniformly on `mat`, `vec`, and standard containers.

### Relationships

- `row_type` — `vec<C, value_type>`: each row stores elements of this type.
- `column_type` — `vec<R, value_type>`: each column stores elements of this type.

<!-- ink:api-end name="value_type" -->
