# API Reference — `tdm/Computations`

---

<!-- ink:api name="adjoint" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `mat<N, N, T> adjoint(const mat<N, N, T>& m)`

Compute the adjugate (classical adjoint) of a square matrix — the transpose of its cofactor matrix.

### When to use this

Use as a building block for matrix inversion via `adj(M) / det(M)`. This function is in `namespace tdm::impl` and is an internal helper; use `inverse` directly for the full inversion operation.

### Example

```cpp
tdm::mat<2, 2, float> m = {{4, 3}, {3, 2}};
tdm::mat<2, 2, float> adj = tdm::impl::adjoint(m);
// adj == {{2, -3}, {-3, 4}}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — square matrix |

### Returns

`mat<N, N, T>` — the adjugate matrix. For a 1x1 matrix, returns `mat<N,N,T>{1}`.

<!-- ink:api-end name="adjoint" -->

<!-- ink:api name="conjugate" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `quat<T> conjugate(const quat<T>& q)`

Return the conjugate of a quaternion by negating its x, y, z components while keeping w unchanged.

### When to use this

Use as a building block when computing the quaternion inverse for unit quaternions — for unit quaternions, the conjugate is equal to the inverse and is cheaper to compute. Use `inverse` instead when the quaternion may not be unit-length.

### Example

```cpp
tdm::quat<float> q = {0.0f, 0.707f, 0.0f, 0.707f};  // 90-degree rotation around Y
tdm::quat<float> qc = tdm::conjugate(q);
// qc == {0.0f, -0.707f, 0.0f, 0.707f}
// qc represents the inverse rotation (for unit quaternions)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q` | `const quat<T>&` | required — quaternion to conjugate |

### Returns

`quat<T>` — a new quaternion with `{-q.x, -q.y, -q.z, q.w}`.

<!-- ink:api-end name="conjugate" -->

<!-- ink:api name="cross" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `vec3<T> cross(const vec3<T>& lhs, const vec3<T>& rhs)`

Compute the cross product of two 3D vectors, returning a vector orthogonal to both.

### When to use this

Use when you need a vector perpendicular to a plane defined by two direction vectors — for example, computing a surface normal from two edge vectors. Only valid for `vec3`; for dot products or higher-dimensional vectors, see `dot`.

### Example

```cpp
tdm::vec3<float> edge1 = {1.0f, 0.0f, 0.0f};
tdm::vec3<float> edge2 = {0.0f, 1.0f, 0.0f};
tdm::vec3<float> normal = tdm::cross(edge1, edge2);
// normal == {0.0f, 0.0f, 1.0f}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `lhs` | `const vec3<T>&` | required — left-hand 3D vector |
| `rhs` | `const vec3<T>&` | required — right-hand 3D vector |

### Returns

`vec3<T>` — a vector perpendicular to both `lhs` and `rhs`, with magnitude equal to `|lhs| * |rhs| * sin(theta)`.

<!-- ink:api-end name="cross" -->

<!-- ink:api name="decompose" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `bool decompose(mat<N, N, T>& a, vec<N, dim_t>& permute)`

Perform LU decomposition with partial pivoting on a square matrix in-place, returning false if the matrix is singular.

### When to use this

Use as the first step before calling `substitute` to solve a linear system `Ax = b`. The LU form is cheaper to compute once and reuse for multiple right-hand sides than calling `inverse` repeatedly. This function is in `namespace tdm::lu`.

### Watch out for

- Modifies `a` in-place. The original matrix is destroyed and replaced by its LU factors. Pass a copy if you need to preserve the original.
- Returns `false` when any row is all-zero (encountered during pivoting). Check the return value before calling `substitute`.

### Example

```cpp
tdm::mat<3, 3, float> a = {{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}};
tdm::vec<3, tdm::dim_t> permute;
if (tdm::lu::decompose(a, permute)) {
    tdm::vec<3, float> b = {8.0f, -11.0f, -3.0f};
    tdm::lu::substitute(a, permute, b);
    // b now contains the solution x
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `a` | `mat<N, N, T>&` | required — square matrix; overwritten in-place with LU factors |
| `permute` | `vec<N, dim_t>&` | required — output permutation vector recording row swaps |

### Returns

`bool` — `true` if decomposition succeeded; `false` if the matrix is singular (a zero row was encountered).

<!-- ink:api-end name="decompose" -->

<!-- ink:api name="determinant" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `T determinant(const mat<N, N, T>& m)`

Compute the scalar determinant of a square matrix to check invertibility or measure transformation scale.

### When to use this

Use to test whether a matrix is invertible before calling `inverse` — a zero determinant means no inverse exists. Also useful for checking whether a transform preserves handedness (positive determinant) or reflects it (negative).

### Example

```cpp
tdm::mat<3, 3, float> m = {{2, 0, 0}, {0, 3, 0}, {0, 0, 4}};
float det = tdm::determinant(m);
// det == 24.0f  (non-zero: matrix is invertible)

tdm::mat<3, 3, float> singular = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
float det2 = tdm::determinant(singular);
// det2 == 0.0f  (matrix is singular)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — square matrix of dimension N x N |

### Returns

`T` — the scalar determinant. Zero indicates a singular (non-invertible) matrix.

<!-- ink:api-end name="determinant" -->

<!-- ink:api name="dot" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `T dot(const vec<L, T>& lhs, const vec<L, T>& rhs)` / `T dot(const quat<T>& q1, const quat<T>& q2)`

Compute the dot product of two vectors or two quaternions.

### When to use this

Use the vector overload to measure projection or check orthogonality between two vectors of the same dimension. Use the quaternion overload when computing the cosine of the angle between two orientations — for example, as a prerequisite inside `slerp` to determine interpolation direction.

### Example

```cpp
tdm::vec3<float> a = {1.0f, 0.0f, 0.0f};
tdm::vec3<float> b = {0.0f, 1.0f, 0.0f};
float result = tdm::dot(a, b);
// result == 0.0f  (orthogonal vectors)

tdm::quat<float> q1 = {0.0f, 0.0f, 0.0f, 1.0f};
tdm::quat<float> q2 = {0.0f, 0.0f, 0.0f, 1.0f};
float cosAngle = tdm::dot(q1, q2);
// cosAngle == 1.0f  (same orientation)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `lhs` / `q1` | `const vec<L, T>&` or `const quat<T>&` | required — first operand |
| `rhs` / `q2` | `const vec<L, T>&` or `const quat<T>&` | required — second operand; must be same type and dimension as first |

### Returns

`T` — scalar dot product. For unit vectors, equals the cosine of the angle between them.

<!-- ink:api-end name="dot" -->

<!-- ink:api name="inverse" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `quat<T> inverse(const quat<T>& q)` / `mat<N, N, T> inverse(const mat<N, N, T>& m)`

Compute the inverse of a quaternion or a square matrix, returning a zero-initialized result for singular matrices.

### When to use this

Use the quaternion overload to reverse an applied rotation — for example, to transform a vector from world space back to local space. Use the matrix overload when you need the exact matrix inverse; for unit quaternions, prefer `conjugate` which is equivalent but cheaper.

### Watch out for

- The matrix overload checks whether the determinant is zero and returns a default-constructed (zero) matrix in that case. Always verify the result is non-zero before using it in a transform chain.
- The quaternion overload divides by `length2()`. Passing a zero quaternion causes division by zero.

### Example

```cpp
// Quaternion inverse
tdm::quat<float> rotation = {0.0f, 0.707f, 0.0f, 0.707f};
tdm::quat<float> inv_rotation = tdm::inverse(rotation);
// Applying rotation then inv_rotation yields identity

// Matrix inverse
tdm::mat<3, 3, float> m = { /* non-singular 3x3 matrix */ };
tdm::mat<3, 3, float> m_inv = tdm::inverse(m);
if (m_inv != tdm::mat<3, 3, float>{}) {
    // use m_inv
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q` | `const quat<T>&` | required — quaternion to invert |
| `m` | `const mat<N, N, T>&` | required — square matrix to invert; must be non-singular |

### Returns

`quat<T>` or `mat<N, N, T>` — the inverse. For the matrix overload, returns a zero-initialized matrix if the determinant is zero.

<!-- ink:api-end name="inverse" -->

<!-- ink:api name="length" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `T length(const vec<L, T>& v)` / `T length(const quat<T>& q)`

Compute the Euclidean length (magnitude) of a vector or quaternion.

### When to use this

Use to measure the magnitude of a direction or displacement vector, or to verify that a quaternion is unit-length before using it as a rotation. Only available for floating-point element types; the SFINAE guard prevents instantiation with integer types.

### Example

```cpp
tdm::vec3<float> v = {3.0f, 4.0f, 0.0f};
float len = tdm::length(v);
// len == 5.0f

tdm::quat<double> q = {0.0, 0.0, 0.0, 1.0};
double qlen = tdm::length(q);
// qlen == 1.0  (unit quaternion)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `v` | `const vec<L, T>&` | required — vector; `T` must be a floating-point type |
| `q` | `const quat<T>&` | required — quaternion; `T` must be a floating-point type |

### Returns

`T` — the Euclidean norm: `sqrt(sum of squared components)`.

### Constraints

- `T` must satisfy `std::is_floating_point<T>`. Instantiation with integer element types will not compile.

<!-- ink:api-end name="length" -->

<!-- ink:api name="lerp" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `quat<T> lerp(const quat<T>& q1, const quat<T>& q2, T t)`

Linearly interpolate between two quaternions using a weighted component blend.

### When to use this

Use when speed and simplicity matter more than constant angular velocity — for example, blending nearby poses where arc distortion is negligible. Use `slerp` instead when you need smooth constant-speed rotation interpolation along the shortest arc.

### Watch out for

- The result is not normalized. Call `normalize` on the output before using it as a rotation if unit length is required.
- Does not take the shortest path automatically. If `dot(q1, q2) < 0`, negate one quaternion before calling to avoid rotating the long way around.

### Example

```cpp
tdm::quat<float> start = {0.0f, 0.0f, 0.0f, 1.0f};      // identity
tdm::quat<float> end   = {0.0f, 0.707f, 0.0f, 0.707f};  // 90 degrees around Y
tdm::quat<float> mid   = tdm::normalize(tdm::lerp(start, end, 0.5f));
// mid is approximately halfway between the two orientations
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q1` | `const quat<T>&` | required — start quaternion |
| `q2` | `const quat<T>&` | required — end quaternion |
| `t` | `T` | required — interpolation factor; `0.0` returns `q1`, `1.0` returns `q2` |

### Returns

`quat<T>` — component-wise blended quaternion. Not guaranteed to be unit-length.

<!-- ink:api-end name="lerp" -->

<!-- ink:api name="minor" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `void minor(const mat<N, N, T>& input, dim_t dimensions, dim_t i, dim_t j, mat<N, N, T>& output)`

Extract the submatrix formed by deleting row `i` and column `j` from a square matrix.

### When to use this

Use as a building block for cofactor expansion when computing determinants or the adjugate matrix. This function is in `namespace tdm::impl` and is an internal helper; prefer `determinant` and `adjoint` for direct use.

### Example

```cpp
tdm::mat<3, 3, float> m = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
tdm::mat<3, 3, float> sub;
tdm::impl::minor(m, 3, 0, 0, sub);
// sub contains the 2x2 submatrix with row 0 and col 0 deleted
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `input` | `const mat<N, N, T>&` | required — source square matrix |
| `dimensions` | `dim_t` | required — active size of the matrix (may be less than N for recursive calls) |
| `i` | `dim_t` | required — row index to exclude |
| `j` | `dim_t` | required — column index to exclude |
| `output` | `mat<N, N, T>&` | required — output matrix; active region `[0, dimensions-1)` is overwritten |

### Returns

Void — result is written to `output`.

<!-- ink:api-end name="minor" -->

<!-- ink:api name="negate" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `negate(vec<L, T>)` / `negate(mat<R, C, T>)` / `negate(quat<T>)`

Flip the sign of every component of a vector, matrix, or quaternion.

### When to use this

Use when you need to reverse the direction of a vector or negate a quaternion (e.g., to represent the same rotation reached via the antipodal path). Note that `negate(q)` and `q` represent the same 3D rotation — choose based on which hemisphere you need for interpolation continuity.

### Example

```cpp
tdm::vec3<float> v = {1.0f, -2.0f, 3.0f};
tdm::vec3<float> neg_v = tdm::negate(v);
// neg_v == {-1.0f, 2.0f, -3.0f}

tdm::quat<float> q = {0.0f, 0.707f, 0.0f, 0.707f};
tdm::quat<float> neg_q = tdm::negate(q);
// neg_q represents the same rotation as q
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `v` | `vec<L, T>` | required — vector to negate (passed by value) |
| `m` | `mat<R, C, T>` | required — matrix to negate (passed by value) |
| `q` | `quat<T>` | required — quaternion to negate (passed by value) |

### Returns

The same type as the input — a new object with all components sign-flipped.

<!-- ink:api-end name="negate" -->

<!-- ink:api name="normalize" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `vec<L, T> normalize(vec<L, T> v)` / `quat<T> normalize(quat<T> q)`

Scale a vector or quaternion to unit length.

### When to use this

Use before passing a direction vector to operations that assume unit-length input — such as computing lighting normals or feeding orientations to `slerp`. Only available for floating-point element types.

### Watch out for

- Normalizing a zero vector produces undefined behavior (division by zero). Check that the vector is non-zero before calling.

### Example

```cpp
tdm::vec3<float> dir = {3.0f, 0.0f, 4.0f};
tdm::vec3<float> unit = tdm::normalize(dir);
// unit == {0.6f, 0.0f, 0.8f}  (length == 1.0f)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `v` | `vec<L, T>` | required — vector to normalize; `T` must be floating-point |
| `q` | `quat<T>` | required — quaternion to normalize; `T` must be floating-point |

### Returns

The same type as the input — a new object scaled to length 1.

### Constraints

- `T` must satisfy `std::is_floating_point<T>`. Integer element types will not compile.

<!-- ink:api-end name="normalize" -->

<!-- ink:api name="slerp" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `quat<T> slerp(const quat<T>& q1, const quat<T>& q2, T t)`

Smoothly interpolate between two quaternion orientations at constant angular velocity.

### When to use this

Use for animation blending or camera interpolation where you need the rotation to proceed at constant speed without distortion through the midpoint. Use `lerp` instead when blending very close orientations where the difference is too small to matter — `slerp` automatically falls back to linear blending when `cos(theta)` is near 1, so there is no discontinuity at the boundary.

### Watch out for

- Automatically negates `q2` when `dot(q1, q2) < 0` to ensure the shortest-path arc is taken. This means the output quaternion may differ in sign from `q2` even at `t = 1.0`.
- Falls back to linear interpolation when the quaternions are nearly parallel (`cos(theta) > 1 - epsilon`). This prevents division by near-zero sine values.

### Example

```cpp
tdm::quat<float> start = {0.0f, 0.0f, 0.0f, 1.0f};  // identity rotation
tdm::quat<float> end   = {0.0f, 1.0f, 0.0f, 0.0f};  // 180 degrees around Y
tdm::quat<float> mid   = tdm::slerp(start, end, 0.5f);
// mid is exactly 90 degrees around Y, traveled at constant angular speed
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q1` | `const quat<T>&` | required — start orientation |
| `q2` | `const quat<T>&` | required — end orientation |
| `t` | `T` | required — interpolation factor in `[0, 1]`; `0.0` returns `q1`, `1.0` returns `q2` |

### Returns

`quat<T>` — unit quaternion interpolated along the great-circle arc from `q1` to `q2`.

<!-- ink:api-end name="slerp" -->

<!-- ink:api name="substitute" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `void substitute(const mat<N, N, T>& a, const vec<N, dim_t>& permute, vec<N, T>& b)`

Solve a linear system `Ax = b` using forward and backward substitution on an LU-decomposed matrix.

### When to use this

Call this after `tdm::lu::decompose` to solve for `x` in `Ax = b`. You can reuse the same decomposed matrix `a` and `permute` for multiple different right-hand sides `b` without re-decomposing. This function is in `namespace tdm::lu`.

### Watch out for

- Modifies `b` in-place. On return, `b` contains the solution vector `x`, not the original right-hand side.
- Only safe to call after a successful `decompose` call (return value `true`). Calling on a singular decomposition produces undefined results.

### Example

```cpp
tdm::mat<3, 3, float> a = {{2, 1, -1}, {-3, -1, 2}, {-2, 1, 2}};
tdm::vec<3, tdm::dim_t> permute;
if (tdm::lu::decompose(a, permute)) {
    tdm::vec<3, float> b = {8.0f, -11.0f, -3.0f};
    tdm::lu::substitute(a, permute, b);
    // b now holds the solution: {2.0f, 3.0f, -1.0f}
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `a` | `const mat<N, N, T>&` | required — LU-decomposed matrix from `decompose` |
| `permute` | `const vec<N, dim_t>&` | required — permutation vector from `decompose` |
| `b` | `vec<N, T>&` | required — right-hand side on input; solution vector on output |

### Returns

Void — the solution is written back into `b`.

<!-- ink:api-end name="substitute" -->

<!-- ink:api name="trace" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `T trace(const mat<N, N, T>& m)`

Sum the diagonal elements of a square matrix.

### When to use this

Use to extract the sum of eigenvalues of a matrix, or as part of algorithms that derive rotation angle from a rotation matrix (the trace of a 3x3 rotation matrix equals `1 + 2*cos(theta)`).

### Example

```cpp
tdm::mat<3, 3, float> m = {{1, 0, 0}, {0, 2, 0}, {0, 0, 3}};
float tr = tdm::trace(m);
// tr == 6.0f
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<N, N, T>&` | required — square N x N matrix |

### Returns

`T` — sum of `m(0,0) + m(1,1) + ... + m(N-1,N-1)`.

<!-- ink:api-end name="trace" -->

<!-- ink:api name="transpose" module="tdm/Computations" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `mat<C, R, T> transpose(const mat<R, C, T>& m)`

Produce the transpose of a matrix by swapping rows and columns.

### When to use this

Use when converting between row-major and column-major conventions, or when you need the adjoint direction matrix for a normal transform (the inverse-transpose of the model matrix). For square orthogonal matrices, the transpose equals the inverse — prefer this over `inverse` in that case.

### Example

```cpp
tdm::mat<2, 3, float> m = {{1, 2, 3}, {4, 5, 6}};
tdm::mat<3, 2, float> t = tdm::transpose(m);
// t == {{1, 4}, {2, 5}, {3, 6}}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat<R, C, T>&` | required — input matrix with R rows and C columns |

### Returns

`mat<C, R, T>` — new matrix with dimensions swapped; element at `[i][j]` in the input appears at `[j][i]` in the output.

<!-- ink:api-end name="transpose" -->
