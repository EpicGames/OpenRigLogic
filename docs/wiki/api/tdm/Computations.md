# API Reference — `tdm/Computations`

---

<!-- ink:api name="adjoint" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> mat<N, N, T> adjoint(const mat<N, N, T>& m)`

Compute the adjoint (adjugate) matrix, the transpose of the cofactor matrix.

### When to use this

Use this as a building block for computing `inverse` via the classical adjugate method (`inverse = adjoint / determinant`) — it is generally not needed directly outside that computation.

### Example

```cpp
tdm::mat3<float> m{ /* ... */ };
tdm::mat3<float> adj = tdm::impl::adjoint(m);
// adj can be divided by determinant(m) to get the inverse
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — the square matrix |

### Returns

`mat<N, N, T>` — the adjoint matrix.

<!-- ink:api-end name="adjoint" -->

<!-- ink:api name="conjugate" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> quat<T> conjugate(const quat<T>& q)`

Compute the conjugate of a quaternion by negating its vector (imaginary) components.

### When to use this

Use this as a building block for computing a quaternion's `inverse` (for a unit quaternion, the conjugate equals the inverse), or when you need to reverse the rotation direction represented by a quaternion.

### Example

```cpp
tdm::quat<float> q{0.0f, 0.0f, 0.7071f, 0.7071f};
tdm::quat<float> qc = tdm::conjugate(q);
// qc is {0.0, 0.0, -0.7071, 0.7071}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q` | `const quat<T>&` | required — the quaternion to conjugate |

### Returns

`quat<T>` — the conjugate quaternion (x, y, z negated; w unchanged).

<!-- ink:api-end name="conjugate" -->

<!-- ink:api name="cross" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> vec3<T> cross(const vec3<T>& lhs, const vec3<T>& rhs)`

Compute the cross product of two 3D vectors, producing a vector perpendicular to both.

### When to use this

Use this when you need a vector perpendicular to two others — for example, to derive a surface normal from two edge vectors, or to build an orthogonal basis.

### Example

```cpp
tdm::vec3<float> edge1{1.0f, 0.0f, 0.0f};
tdm::vec3<float> edge2{0.0f, 1.0f, 0.0f};
tdm::vec3<float> normal = tdm::cross(edge1, edge2);
// normal is {0, 0, 1}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `lhs` | `const vec3<T>&` | required — the first vector |
| `rhs` | `const vec3<T>&` | required — the second vector |

### Returns

`vec3<T>` — a vector orthogonal to both `lhs` and `rhs`, following the right-hand rule.

<!-- ink:api-end name="cross" -->

<!-- ink:api name="decompose" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> bool decompose(mat<N, N, T>& a, vec<N, dim_t>& permute)`

Perform an in-place LU decomposition of a square matrix with partial pivoting, based on the algorithm in *Numerical Recipes in C*.

### When to use this

Use this together with `substitute` to solve linear systems or compute a matrix inverse (see `lu::inverse`) more efficiently than the general adjoint/determinant method for larger matrices.

### Watch out for

- Modifies `a` in place — it is overwritten with the combined L/U factors. Pass a copy if the original matrix must be preserved.
- Returns `false` if any row is entirely zero (matrix is singular); check the return value before calling `substitute`.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `a` | `mat<N, N, T>&` | required — matrix to decompose; overwritten with LU factors |
| `permute` | `vec<N, dim_t>&` | required — receives the row permutation performed during partial pivoting |

### Returns

`bool` — `true` if decomposition succeeded; `false` if the matrix is singular.

<!-- ink:api-end name="decompose" -->

<!-- ink:api name="determinant" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> T determinant(const mat<N, N, T>& m)`

Compute the determinant of a square matrix.

### When to use this

Use this to check if a matrix is invertible (non-zero determinant) before calling `inverse`, or to measure how a transform scales area/volume, or to determine `coord_sys` handedness via its sign.

### Example

```cpp
tdm::mat3<float> m{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
float det = tdm::determinant(m);
// det is 1.0 (identity matrix)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — the square matrix |

### Returns

`T` — the determinant; zero indicates the matrix is singular (non-invertible).

<!-- ink:api-end name="determinant" -->

<!-- ink:api name="dot" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<...> T dot(...)`

Compute the dot product of two vectors or quaternions, a scalar measure of how aligned they are.

### When to use this

Use this to measure alignment between two directions (positive means same general direction, negative means opposite), to project one vector onto another, or — for quaternions — to measure similarity between two rotations before interpolating with `slerp`.

### Example

```cpp
tdm::vec3<float> a{1.0f, 0.0f, 0.0f};
tdm::vec3<float> b{0.0f, 1.0f, 0.0f};
float alignment = tdm::dot(a, b);
// alignment is 0.0 (perpendicular)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `lhs` / `q1` | `const vec<L, T>&` or `const quat<T>&` | required — the first vector or quaternion |
| `rhs` / `q2` | `const vec<L, T>&` or `const quat<T>&` | required — the second vector or quaternion |

### Returns

`T` — the scalar dot product.

<!-- ink:api-end name="dot" -->

<!-- ink:api name="inverse" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<...> inverse(...)`

Compute the inverse of a quaternion or a square matrix, undoing its transform.

### When to use this

Use this to reverse a rotation (quaternion) or reverse a linear transform (matrix) — for example, going from world space to local space when you have the local-to-world transform.

### Watch out for

- The matrix overload returns a default-constructed (zero) matrix when the determinant is exactly zero, rather than throwing — check the input for singularity before relying on the result if that distinction matters.
- The matrix overload checks whether the determinant is zero and returns a default-constructed (zero) matrix in that case. Always verify the result is non-zero before using it in a transform chain.
- The quaternion overload divides by `length2()`. Passing a zero quaternion causes division by zero.

### Example

```cpp
tdm::quat<float> rotation{0.0f, 0.0f, 0.7071f, 0.7071f};
tdm::quat<float> reversed = tdm::inverse(rotation);
// reversed undoes the effect of `rotation` when composed with it
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q` / `m` | `const quat<T>&` or `const mat<N, N, T>&` | required — the quaternion or square matrix to invert |

### Returns

Same type as input — the inverse quaternion or matrix.

<!-- ink:api-end name="inverse" -->

<!-- ink:api name="length" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<...> T length(...)`

Compute the magnitude (length) of a vector or quaternion.

### When to use this

Use this to measure the size of a vector — for example, the distance represented by a displacement vector, or to check if a quaternion is normalized (length ≈ 1).

### Example

```cpp
tdm::vec3<float> displacement{3.0f, 4.0f, 0.0f};
float dist = tdm::length(displacement);
// dist is 5.0
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `v` / `q` | `const vec<L, T>&` or `const quat<T>&` | required — the value whose length to compute; `T` must be floating-point |

### Returns

`T` — the Euclidean length/magnitude.

### Constraints

- `T` must satisfy `std::is_floating_point<T>`. Instantiation with integer element types will not compile.

<!-- ink:api-end name="length" -->

<!-- ink:api name="lerp" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> quat<T> lerp(const quat<T>& q1, const quat<T>& q2, T t)`

Linearly interpolate between two quaternions.

### When to use this

Use this for cheap, approximate rotation blending where per-frame speed variation near the endpoints is acceptable. Use `slerp` instead when you need constant angular velocity across the interpolation, such as smooth camera or character rotation animation.

### Example

```cpp
tdm::quat<float> start{0.0f, 0.0f, 0.0f, 1.0f};
tdm::quat<float> end{0.0f, 0.0f, 0.7071f, 0.7071f};
tdm::quat<float> blended = tdm::lerp(start, end, 0.5f);
// blended is roughly halfway between start and end (not renormalized)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q1` | `const quat<T>&` | required — the starting quaternion |
| `q2` | `const quat<T>&` | required — the ending quaternion |
| `t` | `T` | required — interpolation factor; `0` returns `q1`, `1` returns `q2` |

### Returns

`quat<T>` — the linearly interpolated quaternion.

### Watch out for

- The result is not normalized. Call `normalize` on the output before using it as a rotation if unit length is required.
- Does not take the shortest path automatically. If `dot(q1, q2) < 0`, negate one quaternion before calling to avoid rotating the long way around.

<!-- ink:api-end name="lerp" -->

<!-- ink:api name="minor" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> void minor(const mat<N, N, T>& input, dim_t dimensions, dim_t i, dim_t j, mat<N, N, T>& output)`

Extract the minor matrix formed by removing row `i` and column `j` from a square matrix.

### When to use this

Use this as a building block for cofactor expansion when computing a matrix's `determinant` or `adjoint` — it is not typically called directly outside those algorithms.

### Watch out for

- This is an implementation-detail helper (`impl::minor`), macro-guarded to avoid colliding with a `minor` macro defined on some platforms — not part of the stable public API surface.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `input` | `const mat<N, N, T>&` | required — the source matrix |
| `dimensions` | `dim_t` | required — the active size to operate over (may be less than `N` during recursive determinant computation) |
| `i` | `dim_t` | required — row index to exclude |
| `j` | `dim_t` | required — column index to exclude |
| `output` | `mat<N, N, T>&` | required — receives the resulting minor matrix |

<!-- ink:api-end name="minor" -->

<!-- ink:api name="negate" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<...> negate(...)`

Compute the negation of a vector, matrix, or quaternion, flipping the sign of every component.

### When to use this

Use this as a free-function form of negation when writing generic code that operates over `vec`, `mat`, or `quat` uniformly, rather than calling each type's own `.negate()` member directly.

### Example

```cpp
tdm::vec3<float> v{1.0f, -2.0f, 3.0f};
tdm::vec3<float> negated = tdm::negate(v);
// negated is {-1, 2, -3}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `v` / `m` / `q` | `vec<L, T>`, `mat<R, C, T>`, or `quat<T>` | required — the value to negate (passed by value) |

### Returns

Same type as the input — every component negated.

<!-- ink:api-end name="negate" -->

<!-- ink:api name="normalize" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<...> normalize(...)`

Scale a vector or quaternion to unit length while preserving its direction.

### When to use this

Use this before using a vector as a direction (e.g. a normal or an axis) or before treating a quaternion as a valid rotation, since many operations assume unit length.

### Example

```cpp
tdm::vec3<float> v{3.0f, 4.0f, 0.0f};
tdm::vec3<float> unit = tdm::normalize(v);
// unit is {0.6, 0.8, 0.0}, length 1.0
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `v` / `q` | `vec<L, T>` or `quat<T>` | required — the value to normalize; `T` must be floating-point |

### Returns

Same type as input — the unit-length version of the input.

### Constraints

- `T` must satisfy `std::is_floating_point<T>`. Integer element types will not compile.

### Watch out for

- Normalizing a zero vector produces undefined behavior (division by zero). Check that the vector is non-zero before calling.

<!-- ink:api-end name="normalize" -->

<!-- ink:api name="slerp" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> quat<T> slerp(const quat<T>& q1, const quat<T>& q2, T t)`

Spherically interpolate between two rotations, moving at a constant angular velocity.

### When to use this

Use this for animation and camera work where a smooth, constant-speed rotation blend matters. Use `lerp` instead when the interpolation is short or performance-critical and speed artifacts near the endpoints are acceptable.

### Watch out for

- If the dot product of the two quaternions is negative, the function negates one of them internally to take the shorter path — this avoids unwanted long-way-around rotation.
- Falls back to linear interpolation when the quaternions are nearly identical (`costheta` close to 1), to avoid a division by a near-zero `sin(theta)`.
- Automatically negates `q2` when `dot(q1, q2) < 0` to ensure the shortest-path arc is taken. This means the output quaternion may differ in sign from `q2` even at `t = 1.0`.

### Example

```cpp
tdm::quat<float> start{0.0f, 0.0f, 0.0f, 1.0f};
tdm::quat<float> end{0.0f, 0.0f, 0.7071f, 0.7071f};
tdm::quat<float> mid = tdm::slerp(start, end, 0.5f);
// mid is the rotation exactly halfway along the shortest arc from start to end
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q1` | `const quat<T>&` | required — the starting rotation |
| `q2` | `const quat<T>&` | required — the ending rotation |
| `t` | `T` | required — interpolation factor from `0` (q1) to `1` (q2) |

### Returns

`quat<T>` — the spherically interpolated quaternion.

<!-- ink:api-end name="slerp" -->

<!-- ink:api name="substitute" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> void substitute(const mat<N, N, T>& a, const vec<N, dim_t>& permute, vec<N, T>& b)`

Solve a linear system in place using the LU-decomposed matrix and permutation produced by `decompose`.

### When to use this

Use this immediately after a successful `decompose` call to solve `A x = b` for `x`, applying forward and back substitution against the LU factors.

### Watch out for

- Must be called with the `a` and `permute` outputs from a prior `decompose` call on the same original matrix — calling it with an unrelated matrix produces meaningless results.
- `b` is overwritten in place with the solution vector `x`.
- Only safe to call after a successful `decompose` call (return value `true`). Calling on a singular decomposition produces undefined results.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `a` | `const mat<N, N, T>&` | required — the LU-decomposed matrix from `decompose` |
| `permute` | `const vec<N, dim_t>&` | required — the permutation vector from `decompose` |
| `b` | `vec<N, T>&` | required — right-hand side vector on input; solution vector on output |

<!-- ink:api-end name="substitute" -->

<!-- ink:api name="trace" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t N, typename T> T trace(const mat<N, N, T>& m)`

Compute the trace of a square matrix, the sum of its diagonal elements.

### When to use this

Use this when you need a quick invariant scalar summary of a matrix — for example, as part of extracting a rotation angle from a rotation matrix.

### Example

```cpp
tdm::mat3<float> m{1.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 3.0f};
float t = tdm::trace(m);
// t is 6.0
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — the square matrix |

### Returns

`T` — sum of the diagonal elements.

<!-- ink:api-end name="trace" -->

<!-- ink:api name="transpose" module="tdm/Computations" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t R, dim_t C, typename T> mat<C, R, T> transpose(const mat<R, C, T>& m)`

Flip a matrix's rows and columns.

### When to use this

Use this when you need to convert between row-major and column-major representations of the same linear map, or when building a matrix from column vectors via `from_columns`.

### Example

```cpp
tdm::mat<3, 4, float> m{ /* ... */ };
tdm::mat<4, 3, float> mt = tdm::transpose(m);
// mt(j, i) == m(i, j) for all i, j
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<R, C, T>&` | required — the matrix to transpose |

### Returns

`mat<C, R, T>` — the transposed matrix, with rows and columns swapped.

<!-- ink:api-end name="transpose" -->
