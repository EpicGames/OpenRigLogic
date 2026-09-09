# API Reference — `tdm/Transforms`

---

<!-- ink:api name="change_of_basis" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> mat3<T> change_of_basis(const coord_sys& src, const coord_sys& dst)`

Build the 3x3 matrix `C` such that `v_dst = v_src * C` for row-vectors, converting between two coordinate systems' bases.

### When to use this

Use this as the shared building block for `convert_position`, `convert_direction`, `convert_scale`, and `convert_rotation` when you need the raw basis-change matrix directly, e.g. to convert several vectors without recomputing the change-of-basis matrix each time.

### Example

```cpp
coord_sys maya_up{/* ... */};
coord_sys unreal_up{/* ... */};
mat3<float> c = change_of_basis<float>(maya_up, unreal_up);
// c can now be reused across many convert_position/convert_direction calls
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `src` | `const coord_sys&` | required — the source coordinate system. |
| `dst` | `const coord_sys&` | required — the destination coordinate system. |

### Returns

`mat3<T>` — the change-of-basis matrix `C = B_src * B_dst^T`, valid because `B_dst^{-1} = B_dst^T` for orthonormal bases.

### Constraints

- Both `src` and `dst` must have orthonormal bases. The formula uses `B_dst^T` as `B_dst^{-1}`, which is only valid for orthonormal frames.

<!-- ink:api-end name="change_of_basis" -->

<!-- ink:api name="convert_direction" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> vec3<T> convert_direction(const vec3<T>& dir, const mat3<T>& c, bool renormalize = true)`

Reorient a direction vector (normal, tangent, velocity) into a different coordinate space — not a position. For positions, use `convert_position`.

### When to use this

Use this for normals, tangents, or any vector representing a direction rather than a location. Renormalization defaults to `true` because a change-of-basis matrix that is not perfectly orthonormal (or includes scale) can otherwise leave the direction non-unit length.

### Example

```cpp
coord_sys maya_up{/* ... */};
coord_sys unreal_up{/* ... */};
vec3<float> normal{0.0f, 1.0f, 0.0f};
vec3<float> converted = convert_direction(normal, maya_up, unreal_up);
// converted is re-normalized by default
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `dir` | `const vec3<T>&` | required — the direction vector to convert. |
| `c` | `const mat3<T>&` | required — the change-of-basis matrix. |
| `renormalize` | `bool` | optional, defaults to `true` — whether to re-normalize the result after conversion. |
| `src_cs`, `dst_cs` | `const coord_sys&` | required (overload) — coordinate systems used to compute `c`. |

### Returns

`vec3<T>` — the converted direction vector, normalized if `renormalize` is `true`.

<!-- ink:api-end name="convert_direction" -->

<!-- ink:api name="convert_position" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> vec3<T> convert_position(const vec3<T>& pos, const mat3<T>& c)`

Apply a change-of-basis matrix to a position (or translation) vector to move it into a different coordinate system.

### When to use this

Use this for positions and translation offsets. Both are transformed the same way by the change-of-basis matrix. For directions like normals or tangents, use `convert_direction` instead — it optionally renormalizes, which is not meaningful for positions.

### Example

```cpp
coord_sys maya_up{/* ... */};
coord_sys unreal_up{/* ... */};
vec3<float> maya_pos{10.0f, 20.0f, 30.0f};
vec3<float> unreal_pos = convert_position(maya_pos, maya_up, unreal_up);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `pos` | `const vec3<T>&` | required — the position (or translation) vector to convert. |
| `c` | `const mat3<T>&` | required — the change-of-basis matrix (see `change_of_basis`). |
| `src_cs`, `dst_cs` | `const coord_sys&` | required (overload) — source and destination coordinate systems, used to compute `c` internally. |

### Returns

`vec3<T>` — the position vector re-expressed in the destination coordinate system.

<!-- ink:api-end name="convert_position" -->

<!-- ink:api name="convert_rotation" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> rad3<T> convert_rotation(const rad3<T>& rotation, const mat3<T>& c, rot_seq src_seq, const rot_sign& src_signs, rot_seq dst_seq, const rot_sign& dst_signs)`

Convert a rotation expressed as Euler angles in one coordinate system and rotation convention into Euler angles in another, by round-tripping through a rotation matrix and applying a similarity transform.

### When to use this

Use this when migrating rig or animation rotation data between coordinate systems that also use different Euler orders or sign conventions (e.g. Maya to Unreal), since it correctly composes both the axis change and the convention change instead of naively remapping angle components.

### Example

```cpp
coord_sys maya_up{/* ... */};
coord_sys unreal_up{/* ... */};
rad3<float> maya_rotation{rad<float>{0.1f}, rad<float>{0.2f}, rad<float>{0.3f}};
rad3<float> unreal_rotation = convert_rotation(maya_rotation, maya_up, rot_seq::xyz, rot_sign{1,1,1}, unreal_up, rot_seq::zyx, rot_sign{1,1,1});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `rotation` | `const rad3<T>&` | required — source Euler angles, in radians. |
| `c` | `const mat3<T>&` | required — the change-of-basis matrix. |
| `src_seq` | `rot_seq` | required — the source rotation order. |
| `src_signs` | `const rot_sign&` | required — the source per-axis sign convention. |
| `dst_seq` | `rot_seq` | required — the destination rotation order. |
| `dst_signs` | `const rot_sign&` | required — the destination per-axis sign convention. |
| `src_cs`, `dst_cs` | `const coord_sys&` | required (overload) — coordinate systems used to compute `c`. |

### Returns

`rad3<T>` — the rotation re-expressed as Euler angles in the destination coordinate system, order, and sign convention.

### Watch out for

- Mismatching `src_signs` with the actual convention used to build the animation data produces silently wrong output. Verify the rotation direction convention against the source DCC tool's documentation before passing signs.
- Near gimbal lock in either the source or destination decomposition, one extracted angle is forced to zero by convention. The result is a valid rotation but individual angle components lose their independent meaning.

<!-- ink:api-end name="convert_rotation" -->

<!-- ink:api name="convert_scale" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> vec3<T> convert_scale(const vec3<T>& scale, const mat3<T>& c, sign_policy policy)`

Convert a scale vector between coordinate systems, permuting components by the change-of-basis matrix and applying `sign_policy` to decide whether to keep or discard each component's sign.

### When to use this

Use `sign_policy::preserve` for signed scale deltas (offsets from a neutral scale) since the axis-flip signs in the change of basis cancel out and must not be discarded. Use `sign_policy::discard` for absolute scale factors, where the result should always be non-negative.

### Example

```cpp
coord_sys maya_up{/* ... */};
coord_sys unreal_up{/* ... */};
vec3<float> scale_delta{0.1f, -0.2f, 0.0f};
vec3<float> converted = convert_scale(scale_delta, maya_up, unreal_up, sign_policy::preserve);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `scale` | `const vec3<T>&` | required — the scale vector to convert. |
| `c` | `const mat3<T>&` | required — the change-of-basis matrix. |
| `policy` | `sign_policy` | required — whether to preserve or discard component sign after conversion. |
| `src_cs`, `dst_cs` | `const coord_sys&` | required (overload) — coordinate systems used to compute `c`. |

### Returns

`vec3<T>` — the converted scale vector, signed per `policy`.

### Watch out for

- The `fabs` applied to results means negative scale inputs (mirror or flip scales) lose their sign during conversion. If your rig uses negative scale to represent reflections, handle the sign semantics manually rather than relying on this function.

<!-- ink:api-end name="convert_scale" -->

<!-- ink:api name="euler2mat" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> mat3<T> euler2mat(const rad3<T>& euler, rot_seq seq, rot_sign signs)`

Dispatch to the correct `euler_to_mat` specialization based on a runtime `rot_seq` value.

### When to use this

Use this when the rotation order is only known at runtime (e.g. read from a config or file format). If the order is known at compile time, call `euler_to_mat<T, order>` directly to avoid the switch dispatch.

### Example

```cpp
rad3<float> euler{rad<float>{0.1f}, rad<float>{0.2f}, rad<float>{0.3f}};
rot_seq order = rot_seq::zyx;  // e.g. loaded from a rig config
mat3<float> m = euler2mat(euler, order, rot_sign{1, 1, 1});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `euler` | `const rad3<T>&` | required — per-axis rotation angles, in radians. |
| `seq` | `rot_seq` | required — the rotation order to dispatch on. |
| `signs` | `rot_sign` | required — per-axis rotation direction. |

### Returns

`mat3<T>` — the composed rotation matrix. Returns a default-constructed `mat3<T>` if `seq` matches none of the known enum values.

<!-- ink:api-end name="euler2mat" -->

<!-- ink:api name="euler_to_mat" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T, rot_seq order> mat3<T> euler_to_mat(const rad3<T>& euler, rot_sign signs)`

Build a per-axis rotation matrix from Euler angles by composing `rotx`, `roty`, `rotz` and `rot_mat` for a rotation order fixed at compile time via the `order` template parameter.

### When to use this

Use this when the rotation order is known at compile time and you want to avoid the runtime dispatch overhead of `euler2mat`, which switches on a runtime `rot_seq` value.

### Example

```cpp
rad3<float> euler{rad<float>{0.1f}, rad<float>{0.2f}, rad<float>{0.3f}};
mat3<float> m = euler_to_mat<float, rot_seq::xyz>(euler, rot_sign{1, 1, 1});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `euler` | `const rad3<T>&` | required — per-axis rotation angles, in radians. |
| `signs` | `rot_sign` | required — per-axis rotation direction, forwarded to `rotx`/`roty`/`rotz`. |

### Returns

`mat3<T>` — the composed rotation matrix for the fixed `order`.

<!-- ink:api-end name="euler_to_mat" -->

<!-- ink:api name="fastasin" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> T fastasin(T value)`

Compute an approximate `asin(value)` using a 7-degree minimax polynomial, trading a small amount of accuracy for speed versus `std::asin`.

### When to use this

Use this in hot paths (e.g. per-vertex Euler angle extraction) where `std::asin` is a measurable bottleneck and the minimax approximation's error is acceptable. Prefer `std::asin` when exact results matter more than throughput.

### Example

```cpp
float approx = fastasin(-0.5f);
// approx is close to std::asin(-0.5f) but computed via polynomial approximation
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `value` | `T` | required — the input, clamped internally to [-1, 1] before the approximation is applied. |

### Returns

`T` — the approximate arcsine of `value`, in radians.

### Watch out for

- Input is clamped to [-1, 1] internally (values with `1 - |value| < 0` are treated as exactly ±1), so out-of-range inputs do not produce NaN but silently saturate.
- Only enabled for floating-point `T` via `std::enable_if`; instantiating with an integral type will fail to compile.
- SFINAE-constrained to floating-point types only via `std::enable_if`. Passing an integer type produces a compile error, not a silent conversion.
- Input values outside `[-1, 1]` are clamped silently — no assertion or exception is raised.

<!-- ink:api-end name="fastasin" -->

<!-- ink:api name="mat2euler" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> rad3<T> mat2euler(const mat3<T>& m, rot_seq seq, rot_sign signs)`

Dispatch to the correct `mat_to_euler` specialization based on a runtime `rot_seq` value.

### When to use this

Use this when converting a rotation matrix back to Euler angles and the target rotation order is only known at runtime. The rotation direction sign convention should match whatever produced the matrix, since the sign is applied after extraction, not before.

### Example

```cpp
mat3<float> m = mat3<float>::identity();
rad3<float> euler = mat2euler(m, rot_seq::zyx, rot_sign{1, 1, 1});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat3<T>&` | required — the rotation matrix to decompose. |
| `seq` | `rot_seq` | required — the rotation order to dispatch on. |
| `signs` | `rot_sign` | required — per-axis sign applied to the extracted angles. |

### Returns

`rad3<T>` — the extracted Euler angles. Returns a default-constructed `rad3<T>` if `seq` matches no known enum value.

### Watch out for

- Passing `signs` that differ from those used to build the matrix produces incorrect angles with no error signal. Always use the same `rot_sign` for both `euler2mat` and `mat2euler`.
- Near gimbal lock, the decomposition sets one angle to zero by convention. Results are still a valid rotation but the individual angles lose their independent meaning.

<!-- ink:api-end name="mat2euler" -->

<!-- ink:api name="mat_to_euler" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T, rot_seq order> struct mat_to_euler { rad3<T> operator()(const mat3<T>& m, rot_sign signs); }`

Decompose a rotation matrix into Euler angles for a rotation order fixed at compile time — the inverse of `euler_to_mat`.

### When to use this

Use this when the rotation order is known at compile time. Use `mat2euler` for a runtime-selected order.

### Example

```cpp
mat3<float> m = mat3<float>::identity();
rad3<float> euler = mat_to_euler<float, rot_seq::xyz>()(m, rot_sign{1, 1, 1});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `const mat3<T>&` | required — the rotation matrix to decompose. |
| `signs` | `rot_sign` | required — per-axis sign applied to the extracted angles. |

### Returns

`rad3<T>` — the extracted per-axis Euler angles, in radians.

### Watch out for

- Near the gimbal-lock boundary (`m(0,2)` at or beyond ±1), one axis is forced to `0` and the other two angles collapse into a single combined term, so the result is not a unique inverse of every input matrix at that boundary.
- Gimbal lock is detected per-specialization using a single matrix element comparison. When lock is detected, one angle is set to zero and another absorbs the full rotation — this is mathematically correct but produces a discontinuity that can cause animation artifacts near the lock angle.

<!-- ink:api-end name="mat_to_euler" -->

<!-- ink:api name="rot_mat" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T, rot_seq order> struct rot_mat { mat3<T> operator()(mat3<T> x, mat3<T> y, mat3<T> z); }`

Multiply three per-axis rotation matrices together in the order dictated by a `rot_seq` specialization.

### When to use this

Use this internally when implementing `euler_to_mat` for a new rotation order; each `rot_seq` value (`xyz`, `xzy`, `yxz`, `yzx`, `zxy`, `zyx`) has its own specialization that fixes the multiplication order (e.g. `x * y * z` for `xyz`, `z * y * x` for `zyx`).

### Example

```cpp
mat3<float> rx = rotx(rad<float>{0.1f}, rot_dir{1});
mat3<float> ry = roty(rad<float>{0.2f}, rot_dir{1});
mat3<float> rz = rotz(rad<float>{0.3f}, rot_dir{1});
mat3<float> combined = rot_mat<float, rot_seq::xyz>()(rx, ry, rz);
// combined == rx * ry * rz
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `x` | `mat3<T>` | required — the elementary rotation about X. |
| `y` | `mat3<T>` | required — the elementary rotation about Y. |
| `z` | `mat3<T>` | required — the elementary rotation about Z. |

### Returns

`mat3<T>` — the product of the three matrices in the order specified by the `rot_seq` template parameter.

<!-- ink:api-end name="rot_mat" -->

<!-- ink:api name="rotate" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `mat4<T> projective::rotate(const vec3<T>& axis, rad<T> angle, rot_dir dir)` (and Euler/compose overloads)

Build a 4x4 rotation transform, either from an arbitrary axis and angle (Rodrigues' rotation formula) or from Euler angles via `impl::euler2mat`, and optionally compose it onto an existing `mat4`.

### When to use this

Use the axis-angle overload when rotating about an arbitrary (non-cardinal) axis, e.g. a joint's rotation axis. Use the Euler-angle overload when working with per-axis angles and a `rot_seq`/`rot_sign` convention. Use the `mat4`-taking overloads to compose the rotation directly onto an existing transform instead of multiplying separately.

### Example

```cpp
// Rotate around an arbitrary axis
mat4<float> r = rotate(vec3<float>{0.0f, 1.0f, 0.0f}, rad<float>{1.57f}, rot_dir{1});

// Rotate by Euler angles and compose onto an existing transform
mat4<float> base = mat4<float>::identity();
mat4<float> composed = rotate(base, rad<float>{0.1f}, rad<float>{0.2f}, rad<float>{0.3f}, rot_seq::xyz, rot_sign{1, 1, 1});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `axis` | `const vec3<T>&` | required (axis-angle overload) — the rotation axis; normalized internally, need not be unit length. |
| `angle` | `rad<T>` | required (axis-angle overload) — rotation angle, in radians. |
| `dir` | `rot_dir` | required (axis-angle overload) — rotation direction sign. |
| `x`, `y`, `z` | `rad<T>` | required (Euler overload) — per-axis rotation angles, in radians. |
| `order` | `rot_seq` | required (Euler overload) — the rotation order. |
| `signs` | `rot_sign` | required (Euler overload) — per-axis rotation direction. |
| `m` | `const mat4<T>&` | optional (compose overloads) — an existing transform the rotation is multiplied onto. |

### Returns

`mat4<T>` — the resulting 4x4 rotation matrix, or the composed transform when an `m` argument is provided.

<!-- ink:api-end name="rotate" -->

<!-- ink:api name="rotx" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> mat3<T> rotx(rad<T> x, rot_dir dir)`

Build the elementary rotation matrix for a rotation about the X axis.

### When to use this

Use this as a building block when composing multi-axis Euler rotations (see `rot_mat`, `euler_to_mat`) rather than for standalone single-axis rotations, where `rotate` may be more convenient.

### Example

```cpp
mat3<float> rx = rotx(rad<float>{0.5f}, rot_dir{1});
// rx rotates around the X axis by 0.5 radians in the given direction
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `x` | `rad<T>` | required — the rotation angle about the X axis, in radians. |
| `dir` | `rot_dir` | required — the rotation direction sign multiplied into the sine term, controlling handedness/convention. |

### Returns

`mat3<T>` — a 3x3 matrix representing the X-axis rotation, starting from identity with the (1,1)/(1,2)/(2,1)/(2,2) block overwritten.

<!-- ink:api-end name="rotx" -->

<!-- ink:api name="roty" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> mat3<T> roty(rad<T> y, rot_dir dir)`

Build the elementary rotation matrix for a rotation about the Y axis.

### When to use this

Use this as a building block when composing multi-axis Euler rotations (see `rot_mat`, `euler_to_mat`) rather than for standalone single-axis rotations.

### Example

```cpp
mat3<float> ry = roty(rad<float>{0.3f}, rot_dir{1});
// ry rotates around the Y axis by 0.3 radians in the given direction
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `y` | `rad<T>` | required — the rotation angle about the Y axis, in radians. |
| `dir` | `rot_dir` | required — the rotation direction sign multiplied into the sine term. |

### Returns

`mat3<T>` — a 3x3 matrix representing the Y-axis rotation.

<!-- ink:api-end name="roty" -->

<!-- ink:api name="rotz" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> mat3<T> rotz(rad<T> z, rot_dir dir)`

Build the elementary rotation matrix for a rotation about the Z axis.

### When to use this

Use this as a building block when composing multi-axis Euler rotations (see `rot_mat`, `euler_to_mat`) rather than for standalone single-axis rotations.

### Example

```cpp
mat3<float> rz = rotz(rad<float>{0.2f}, rot_dir{1});
// rz rotates around the Z axis by 0.2 radians in the given direction
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `z` | `rad<T>` | required — the rotation angle about the Z axis, in radians. |
| `dir` | `rot_dir` | required — the rotation direction sign multiplied into the sine term. |

### Returns

`mat3<T>` — a 3x3 matrix representing the Z-axis rotation.

<!-- ink:api-end name="rotz" -->

<!-- ink:api name="scale" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `mat<L, L, T> affine::scale(const vec<L, T>& factors)` / `mat<L + 1, L + 1, T> projective::scale(const vec<L, T>& factors)`

Build a diagonal scaling matrix from per-axis factors, or apply that scale to an existing matrix.

### When to use this

Use `affine::scale` when composing an `L`x`L` linear transform directly. Use `projective::scale` when working with homogeneous `(L+1)`x`(L+1)` matrices (e.g. `mat4` for 3D), since it writes the factors into the diagonal of the upper-left linear block while leaving the homogeneous row/column at identity.

### Example

```cpp
// Affine 3x3 scale
mat3<float> s = affine::scale(vec3<float>{2.0f, 1.0f, 1.0f});

// Projective (homogeneous) scale composed onto an existing 4x4 transform
mat4<float> m = mat4<float>::identity();
mat4<float> scaled = projective::scale(m, vec3<float>{1.5f, 1.5f, 1.5f});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `factors` | `const vec<L, T>&` | required — per-axis scale factors written to the diagonal. |
| `m` | `const mat<L, L, T>&` (affine) or `const mat<L + 1, L + 1, T>&` (projective) | optional (overload-dependent) — an existing matrix to compose the scale onto via multiplication. |
| `factor` | `T` | optional (overload-dependent) — a uniform scale factor applied to all axes. |

### Returns

`mat<L, L, T>` (affine) or `mat<L + 1, L + 1, T>` (projective) — the resulting scale matrix, or the input matrix with the scale composed in.

<!-- ink:api-end name="scale" -->

<!-- ink:api name="sign_policy" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `sign_policy`

An enum controlling how `convert_scale` treats the sign of each scale component when converting between coordinate systems.

### Why this exists

A change of basis is a signed axis permutation, so converting a scale vector is ambiguous without a stated convention: an absolute scale factor (near 1) should stay non-negative, but a signed scale *delta* must keep its sign since the permutation's axis-flip signs cancel out (they appear squared). `sign_policy` makes the caller's intent explicit instead of guessing from context.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `discard` | enumerator | Treats scale as a directionless per-axis magnitude — the result is always non-negative. Correct for absolute scale factors (e.g. a neutral scale of ~1). |
| `preserve` | enumerator | Preserves each component's own sign, permuting by `\|c\|`. Correct for signed scale deltas (offsets from a neutral scale, which may be negative). |

### Relationships

- `convert_scale` — *consumes `sign_policy` to decide how to handle sign during conversion.*

<!-- ink:api-end name="sign_policy" -->

<!-- ink:api name="translate" module="tdm/Transforms" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<dim_t L, typename T> mat<L + 1, L + 1, T> translate(const vec<L, T>& position)`

Build an `(L+1)x(L+1)` homogeneous translation matrix, writing the position components into the last row of the identity matrix.

### When to use this

Use this when building a homogeneous transform (e.g. `mat4` for 3D) from a translation vector. Use the `m`-taking overload to compose a translation onto an existing transform via multiplication rather than building and multiplying separately.

### Example

```cpp
mat4<float> t = translate(vec3<float>{1.0f, 2.0f, 3.0f});

// Compose onto an existing transform
mat4<float> base = mat4<float>::identity();
mat4<float> combined = translate(base, vec3<float>{0.0f, 0.0f, 5.0f});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `position` | `const vec<L, T>&` | required — the translation offset per axis. |
| `m` | `const mat<L + 1, L + 1, T>&` | optional (compose overload) — an existing transform the translation is multiplied onto. |

### Returns

`mat<L + 1, L + 1, T>` — the homogeneous translation matrix, or the composed transform.

<!-- ink:api-end name="translate" -->
