# API Reference — `tdm/Transforms`

---

<!-- ink:api name="change_of_basis" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> mat3<T> change_of_basis(const coord_sys& src, const coord_sys& dst)`

Compute the 3×3 change-of-basis matrix that re-expresses vectors from `src`'s frame into `dst`'s frame. Uses the row-vector convention: `v_dst = v_src * C`, where `C = B_src * B_dst^T`.

### When to use this

Call this once when you need to convert many vectors between two fixed coordinate systems — then pass the resulting matrix to `convert_position`, `convert_direction`, or `convert_scale` for each vector. This avoids recomputing the basis product on every vector. Use the `coord_sys` overloads of those conversion functions directly when converting only a few vectors; they call `change_of_basis` internally.

### Example

```cpp
// Pre-compute the reusable basis-change matrix
tdm::mat3<float> C = tdm::change_of_basis(src_cs, dst_cs);

// Reuse for many vectors
for (const auto& pos : joint_positions) {
    tdm::vec3<float> converted = tdm::convert_position(pos, C);
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `src` | `coord_sys` | required — the source coordinate system |
| `dst` | `coord_sys` | required — the destination coordinate system |

### Returns

`mat3<T>` — the change-of-basis matrix `C` such that `v_dst = v_src * C`.

### Constraints

- Both `src` and `dst` must have orthonormal bases. The formula uses `B_dst^T` as `B_dst^{-1}`, which is only valid for orthonormal frames.

<!-- ink:api-end name="change_of_basis" -->

<!-- ink:api name="convert_direction" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> vec3<T> convert_direction(const vec3<T>& dir, const mat3<T>& c, bool renormalize = true)`

Reorient a direction vector — such as a normal, tangent, or velocity — to a new coordinate frame. By default, re-normalizes the result to compensate for floating-point drift introduced by the basis-change multiplication.

### When to use this

Use this for vectors that represent orientation rather than location: mesh normals, joint orient axes, velocity directions. For positions and translations, use `convert_position`. For scale vectors, use `convert_scale`. Pass `renormalize = false` only when you are certain the input is a unit vector and the basis change is exactly orthonormal — it saves one `sqrt` call per vector.

### Example

```cpp
// Convert a mesh normal from Maya (Y-up) to Unreal (Z-up) coordinate system
tdm::mat3<float> C = tdm::change_of_basis(maya_cs, unreal_cs);
tdm::vec3<float> unreal_normal = tdm::convert_direction(maya_normal, C);
// Result is normalized

// Skip renormalization when basis change is exact
tdm::vec3<float> raw = tdm::convert_direction(maya_normal, C, false);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `dir` | `vec3<T>` | required — direction vector in the source coordinate system |
| `c` | `mat3<T>` | required (matrix overload) — precomputed change-of-basis matrix |
| `renormalize` | `bool` | optional — re-normalize the result after transformation; default `true` |
| `src_cs` | `coord_sys` | required (coord_sys overload) — source coordinate system |
| `dst_cs` | `coord_sys` | required (coord_sys overload) — destination coordinate system |

### Returns

`vec3<T>` — the direction vector expressed in the destination coordinate system, normalized if `renormalize == true`.

<!-- ink:api-end name="convert_direction" -->

<!-- ink:api name="convert_position" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> vec3<T> convert_position(const vec3<T>& pos, const mat3<T>& c)`

Transform a position or translation vector into a new coordinate frame by multiplying by the change-of-basis matrix. Both overloads apply the same transformation — one accepts a precomputed matrix, the other computes it from `coord_sys` objects on the fly.

### When to use this

Use this for joint positions and translation vectors — anything that has a location in space. For normals, tangents, or velocity directions, use `convert_direction` instead (it optionally re-normalizes and is semantically distinct). For scale vectors, use `convert_scale` which takes absolute values to strip sign flips.

### Example

```cpp
// Pre-computed matrix path (efficient for bulk conversion)
tdm::mat3<float> C = tdm::change_of_basis(src_cs, dst_cs);
tdm::vec3<float> dst_pos = tdm::convert_position(src_pos, C);

// Inline path (single conversion)
tdm::vec3<float> dst_pos2 = tdm::convert_position(src_pos, src_cs, dst_cs);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `pos` | `vec3<T>` | required — position or translation vector in the source coordinate system |
| `c` | `mat3<T>` | required (matrix overload) — precomputed change-of-basis matrix |
| `src_cs` | `coord_sys` | required (coord_sys overload) — source coordinate system |
| `dst_cs` | `coord_sys` | required (coord_sys overload) — destination coordinate system |

### Returns

`vec3<T>` — the position expressed in the destination coordinate system.

<!-- ink:api-end name="convert_position" -->

<!-- ink:api name="convert_rotation" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> rad3<T> convert_rotation(const rad3<T>& rotation, const mat3<T>& c, rot_seq src_seq, const rot_sign& src_signs, rot_seq dst_seq, const rot_sign& dst_signs)`

Convert Euler rotation angles from one coordinate system and rotation convention to another. Builds the source rotation matrix from `src_seq` and `src_signs`, applies the basis change to re-express it in the destination frame, then extracts Euler angles using `dst_seq` and `dst_signs`.

### When to use this

Use this when a joint's Euler angles need to be re-expressed for a different DCC tool or runtime that uses a different coordinate system, rotation order, or rotation direction convention. This is the full pipeline: Euler → matrix → basis change → matrix → Euler. If you only need to change rotation order within the same coordinate system, still use this function with the same `src_cs` and `dst_cs`.

### Example

```cpp
// Convert a Maya joint from XYZ/Y-up/right-handed to Unreal ZYX/Z-up conventions
tdm::mat3<float> C = tdm::change_of_basis(maya_cs, unreal_cs);
tdm::rot_sign maya_signs{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
tdm::rot_sign unreal_signs{tdm::rot_dir::positive, tdm::rot_dir::negative, tdm::rot_dir::positive};
tdm::rad3<float> unreal_rot = tdm::convert_rotation(
    maya_euler, C,
    tdm::rot_seq::xyz, maya_signs,
    tdm::rot_seq::zyx, unreal_signs
);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `rotation` | `rad3<T>` | required — input Euler angles in the source convention |
| `c` | `mat3<T>` | required (matrix overload) — precomputed change-of-basis matrix |
| `src_seq` | `rot_seq` | required — Euler ordering of the input angles |
| `src_signs` | `rot_sign` | required — per-axis rotation direction of the input convention |
| `dst_seq` | `rot_seq` | required — Euler ordering to extract for the output |
| `dst_signs` | `rot_sign` | required — per-axis rotation direction of the output convention |
| `src_cs` | `coord_sys` | required (coord_sys overload) — source coordinate system |
| `dst_cs` | `coord_sys` | required (coord_sys overload) — destination coordinate system |

### Returns

`rad3<T>` — Euler angles in the destination rotation convention and coordinate frame.

### Watch out for

- Mismatching `src_signs` with the actual convention used to build the animation data produces silently wrong output. Verify the rotation direction convention against the source DCC tool's documentation before passing signs.
- Near gimbal lock in either the source or destination decomposition, one extracted angle is forced to zero by convention. The result is a valid rotation but individual angle components lose their independent meaning.

<!-- ink:api-end name="convert_rotation" -->

<!-- ink:api name="convert_scale" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> vec3<T> convert_scale(const vec3<T>& scale, const mat3<T>& c)`

Transform a per-axis scale vector to a new coordinate frame, taking `fabs` of each component after the basis-change multiplication to strip sign flips — scale has no intrinsic direction, so negating an axis during the basis change should not invert the scale magnitude.

### When to use this

Use this when a rig or mesh has per-axis scale values that must be re-expressed in a different coordinate system. The absolute-value step is intentional and distinguishes this from `convert_position` — do not use `convert_position` for scale vectors if you need the sign-stripping behavior.

### Example

```cpp
// Convert joint scale from Maya to Unreal coordinate conventions
tdm::vec3<float> maya_scale{2.0f, 1.0f, 0.5f};
tdm::mat3<float> C = tdm::change_of_basis(maya_cs, unreal_cs);
tdm::vec3<float> unreal_scale = tdm::convert_scale(maya_scale, C);
// All components are non-negative; sign flips from axis remapping are stripped
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `scale` | `vec3<T>` | required — per-axis scale vector in the source coordinate system |
| `c` | `mat3<T>` | required (matrix overload) — precomputed change-of-basis matrix |
| `src_cs` | `coord_sys` | required (coord_sys overload) — source coordinate system |
| `dst_cs` | `coord_sys` | required (coord_sys overload) — destination coordinate system |

### Returns

`vec3<T>` — the scale vector in the destination coordinate system, with all components non-negative.

### Watch out for

- The `fabs` applied to results means negative scale inputs (mirror or flip scales) lose their sign during conversion. If your rig uses negative scale to represent reflections, handle the sign semantics manually rather than relying on this function.

<!-- ink:api-end name="convert_scale" -->

<!-- ink:api name="euler2mat" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> mat3<T> euler2mat(const rad3<T>& euler, rot_seq seq, rot_sign signs)`

Convert Euler angles to a 3×3 rotation matrix with runtime-selectable rotation ordering. Dispatches to the appropriate compile-time `euler_to_mat` specialization via a switch on `seq`.

### When to use this

Use this when the rotation order is not known at compile time — for example, when reading joint rotation orders from a DNA asset that may vary per joint. If the rotation order is always fixed for every call, prefer `euler_to_mat` to avoid the switch overhead.

### Example

```cpp
// Joint has XYZ rotation order from DNA, right-handed convention
tdm::rad3<float> euler{tdm::rad<float>{0.1f}, tdm::rad<float>{-0.3f}, tdm::rad<float>{0.05f}};
tdm::rot_sign signs{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
tdm::mat3<float> R = tdm::impl::euler2mat(euler, tdm::rot_seq::xyz, signs);
// R is now the composed Rx * Ry * Rz rotation matrix
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `euler` | `rad3<T>` | required — X, Y, Z Euler angles in radians |
| `seq` | `rot_seq` | required — Euler rotation ordering (xyz, xzy, yxz, yzx, zxy, zyx) |
| `signs` | `rot_sign` | required — per-axis rotation direction signs |

### Returns

`mat3<T>` — the composed 3×3 rotation matrix. Returns a zero-initialized matrix if `seq` does not match any known value.

<!-- ink:api-end name="euler2mat" -->

<!-- ink:api name="euler_to_mat" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T, rot_seq order> mat3<T> euler_to_mat(const rad3<T>& euler, rot_sign signs)`

Convert Euler angles to a 3×3 rotation matrix using a rotation ordering fixed at compile time. Per-axis signs from `rot_sign` negate the sine terms in each axis matrix before composition.

### When to use this

Use this (`impl` namespace) when the Euler ordering is a compile-time constant and you want zero-overhead inlining — for example, in a tight per-joint loop where the rig's rotation order never changes at runtime. For runtime-selectable ordering, use `euler2mat` instead, which dispatches through a switch.

### Example

```cpp
// XYZ ordering, right-handed signs on all three axes
tdm::rad3<float> euler{tdm::rad<float>{0.1f}, tdm::rad<float>{0.2f}, tdm::rad<float>{0.3f}};
tdm::rot_sign signs{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
tdm::mat3<float> R = tdm::impl::euler_to_mat<float, tdm::rot_seq::xyz>(euler, signs);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `euler` | `rad3<T>` | required — X, Y, Z Euler angles in radians |
| `signs` | `rot_sign` | required — per-axis rotation direction; negates sine terms per axis when negative |

### Returns

`mat3<T>` — a 3×3 rotation matrix representing the composed rotation.

<!-- ink:api-end name="euler_to_mat" -->

<!-- ink:api name="fastasin" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> T fastasin(T value)`

Compute arcsin using a 7-degree minimax polynomial approximation — faster than `std::asin` for bulk transform work. Input is silently clamped to `[-1, 1]` before evaluation.

### When to use this

Reach for this when you need `asin` at high throughput — for example, inside per-frame Euler extraction loops — and can accept minimax approximation error (on the order of 1e-7). Use `std::asin` when you need the full IEEE 754 accuracy guarantee or are computing a safety-critical result.

### Example

```cpp
// Compute the pitch angle from a unit forward vector's Y component
float fwd_y = -0.5f;
float pitch = tdm::fastasin(fwd_y);  // faster than std::asin for bulk use
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `value` | `T` (floating-point) | required — the sine value to invert; clamped to `[-1, 1]` internally |

### Returns

`T` — the arcsin of `value` in radians, in the range `[-π/2, π/2]`.

### Watch out for

- SFINAE-constrained to floating-point types only via `std::enable_if`. Passing an integer type produces a compile error, not a silent conversion.
- Input values outside `[-1, 1]` are clamped silently — no assertion or exception is raised.

<!-- ink:api-end name="fastasin" -->

<!-- ink:api name="mat2euler" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> rad3<T> mat2euler(const mat3<T>& m, rot_seq seq, rot_sign signs)`

Extract Euler angles from a 3×3 rotation matrix with runtime-selectable ordering. The `rot_sign` parameter matches the direction convention used when the matrix was originally built — pass the same signs used in `euler2mat` to get consistent round-trip results.

### When to use this

Use this to recover joint rotation angles from a composed rotation matrix — for example, after applying a coordinate-system change and needing to re-express the rotation in the destination rig's Euler convention. The `rot_sign` parameter is critical: mismatching it with the original build convention produces wrong angles without any error signal.

### Example

```cpp
// Round-trip: build matrix, then extract angles
tdm::rad3<float> input{tdm::rad<float>{0.1f}, tdm::rad<float>{-0.3f}, tdm::rad<float>{0.05f}};
tdm::rot_sign signs{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
tdm::mat3<float> R = tdm::impl::euler2mat(input, tdm::rot_seq::xyz, signs);
tdm::rad3<float> recovered = tdm::impl::mat2euler(R, tdm::rot_seq::xyz, signs);
// recovered ≈ input (within floating-point precision, except near gimbal lock)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `m` | `mat3<T>` | required — a valid rotation matrix (orthonormal) |
| `seq` | `rot_seq` | required — the Euler ordering used when `m` was built |
| `signs` | `rot_sign` | required — per-axis rotation direction; must match the signs used in `euler2mat` for correct round-trip |

### Returns

`rad3<T>` — the extracted X, Y, Z Euler angles in radians. Returns zero-initialized angles if `seq` is unrecognized.

### Watch out for

- Passing `signs` that differ from those used to build the matrix produces incorrect angles with no error signal. Always use the same `rot_sign` for both `euler2mat` and `mat2euler`.
- Near gimbal lock, the decomposition sets one angle to zero by convention. Results are still a valid rotation but the individual angles lose their independent meaning.

<!-- ink:api-end name="mat2euler" -->

<!-- ink:api name="mat_to_euler" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat_to_euler`

Policy struct that extracts Euler angles from a 3×3 rotation matrix, with the rotation ordering fixed at compile time. Each of the six specializations handles the normal case and both gimbal-lock degenerate cases.

### Why this exists

Euler extraction from a rotation matrix requires different trigonometric identities for each of the 6 orderings — the matrix elements that encode each angle change depending on the sequence. Encoding this as a policy struct allows `mat2euler` to dispatch at runtime without per-iteration branching while keeping each specialization inlinable. Each specialization also explicitly handles both gimbal-lock cases (the secondary angle hits ±90°), which a generic formula cannot do without knowing the rotation sequence.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `operator()` | `rad3<T>(const mat3<T>& m, rot_sign signs)` | required — extracts Euler angles from `m` and scales result by `signs` |

### Construction

```cpp
// Used internally via mat2euler dispatcher; direct use:
tdm::mat3<float> R = /* rotation matrix from animation */;
tdm::rot_sign signs{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
tdm::rad3<float> euler = tdm::impl::mat_to_euler<float, tdm::rot_seq::xyz>()(R, signs);
```

### Relationships

- `mat2euler` — runtime dispatcher that selects the correct `mat_to_euler` specialization
- `rot_mat` — the inverse policy: composes matrices from axis matrices
- `euler_to_mat` — the forward direction: Euler angles → rotation matrix

### Watch out for

- Gimbal lock is detected per-specialization using a single matrix element comparison. When lock is detected, one angle is set to zero and another absorbs the full rotation — this is mathematically correct but produces a discontinuity that can cause animation artifacts near the lock angle.

<!-- ink:api-end name="mat_to_euler" -->

<!-- ink:api name="rot_mat" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rot_mat`

Policy struct that composes three per-axis rotation matrices in the order dictated by the `rot_seq` template parameter — one full specialization per Euler ordering.

### Why this exists

Euler rotation order cannot be expressed as a runtime parameter when the composition is inlined — each ordering requires a distinct matrix multiplication sequence. `rot_mat` encodes that ordering as a compile-time policy, allowing `euler_to_mat` to be a zero-overhead template without a runtime switch. The six full specializations cover all intrinsic Euler orderings used by DCC tools and game engines.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `operator()` | `mat3<T>(mat3<T> x, mat3<T> y, mat3<T> z)` | required — composes the three axis matrices in the order encoded by `order` |

### Construction

```cpp
// Used internally by euler_to_mat; direct instantiation:
tdm::mat3<float> R = tdm::impl::rot_mat<float, tdm::rot_seq::xyz>()(Rx, Ry, Rz);
// Returns Rx * Ry * Rz
```

### Relationships

- `euler_to_mat` — uses `rot_mat` to compose axis matrices at compile time
- `euler2mat` — runtime dispatcher that selects the correct `rot_mat` specialization
- `rot_seq` — the ordering enum that selects which specialization is instantiated

<!-- ink:api-end name="rot_mat" -->

<!-- ink:api name="rotate" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> mat4<T> rotate(const vec3<T>& axis, rad<T> angle, rot_dir dir)`

Build a 4×4 projective rotation matrix from an axis-angle or Euler angles, or accumulate rotation into an existing `mat4`. Four overloads cover axis-angle, component Euler, packed `rad3` Euler, and accumulating (post-multiply) variants.

### When to use this

Use this (in `namespace tdm::projective`) when building homogeneous 4×4 transforms for a rendering pipeline or scene-graph composition. The axis-angle overload uses the Rodrigues formula and is appropriate for arbitrary-axis rotations. The Euler overloads embed a 3×3 from `euler2mat` into a 4×4 identity — use them when rotation order matters and you have joint angles from a rig.

### Example

```cpp
// Axis-angle: rotate 45° around world-up Y axis
tdm::vec3<float> up{0.0f, 1.0f, 0.0f};
tdm::mat4<float> R = tdm::rotate(up, tdm::rad<float>{0.7854f}, tdm::rot_dir::positive);

// Euler XYZ, accumulate into an existing model matrix
tdm::mat4<float> M = tdm::mat4<float>::identity();
tdm::rot_sign signs{tdm::rot_dir::positive, tdm::rot_dir::positive, tdm::rot_dir::positive};
M = tdm::rotate(M, tdm::rad<float>{0.1f}, tdm::rad<float>{0.2f}, tdm::rad<float>{0.0f},
                tdm::rot_seq::xyz, signs);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `axis` | `vec3<T>` | required (axis-angle overload) — rotation axis; normalized internally |
| `angle` | `rad<T>` | required (axis-angle overload) — rotation magnitude in radians |
| `dir` | `rot_dir` | required (axis-angle overload) — rotation direction |
| `x`, `y`, `z` | `rad<T>` | required (Euler overload) — per-axis angles |
| `order` | `rot_seq` | required (Euler overloads) — Euler composition ordering |
| `signs` | `rot_sign` | required (Euler overloads) — per-axis rotation direction |
| `rotation` | `rad3<T>` | required (packed Euler overload) — X, Y, Z angles |
| `m` | `mat4<T>` | optional — existing matrix; returns `m * rotate(...)` |

### Returns

`mat4<T>` — a 4×4 rotation matrix with translation column zeroed, or `m` post-multiplied by the rotation.

<!-- ink:api-end name="rotate" -->

<!-- ink:api name="rotx" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> mat3<T> rotx(rad<T> x, rot_dir dir)`

Build a 3×3 rotation matrix for a rotation about the X axis. The `rot_dir` parameter controls whether the rotation is right-handed or left-handed by negating the sine terms.

### When to use this

This is an `impl`-namespace primitive used internally by `euler_to_mat` and `euler2mat`. In application code, prefer `euler2mat` or `projective::rotate` which compose the three axis matrices and handle rotation sequence ordering. Use `rotx` directly only when you need a single-axis rotation matrix for a custom composition.

### Example

```cpp
// 45-degree rotation about X, right-handed
tdm::mat3<float> Rx = tdm::impl::rotx(tdm::rad<float>{0.7854f}, tdm::rot_dir::positive);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `x` | `rad<T>` | required — rotation angle in radians |
| `dir` | `rot_dir` | required — rotation direction; negates sine terms when negative |

### Returns

`mat3<T>` — a 3×3 rotation matrix about the X axis.

<!-- ink:api-end name="rotx" -->

<!-- ink:api name="roty" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> mat3<T> roty(rad<T> y, rot_dir dir)`

Build a 3×3 rotation matrix for a rotation about the Y axis. The `rot_dir` parameter controls rotation direction by negating the sine terms.

### When to use this

Implementation primitive in `namespace impl` — prefer `euler2mat` or `projective::rotate` in application code. Use directly only when composing a custom single-axis rotation about Y.

### Example

```cpp
// 30-degree rotation about Y, right-handed
tdm::mat3<float> Ry = tdm::impl::roty(tdm::rad<float>{0.5236f}, tdm::rot_dir::positive);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `y` | `rad<T>` | required — rotation angle in radians |
| `dir` | `rot_dir` | required — rotation direction; negates sine terms when negative |

### Returns

`mat3<T>` — a 3×3 rotation matrix about the Y axis.

<!-- ink:api-end name="roty" -->

<!-- ink:api name="rotz" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> mat3<T> rotz(rad<T> z, rot_dir dir)`

Build a 3×3 rotation matrix for a rotation about the Z axis. The `rot_dir` parameter controls rotation direction by negating the sine terms.

### When to use this

Implementation primitive in `namespace impl` — prefer `euler2mat` or `projective::rotate` in application code. Use directly only when composing a custom single-axis rotation about Z.

### Example

```cpp
// 90-degree rotation about Z, right-handed
tdm::mat3<float> Rz = tdm::impl::rotz(tdm::rad<float>{1.5708f}, tdm::rot_dir::positive);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `z` | `rad<T>` | required — rotation angle in radians |
| `dir` | `rot_dir` | required — rotation direction; negates sine terms when negative |

### Returns

`mat3<T>` — a 3×3 rotation matrix about the Z axis.

<!-- ink:api-end name="rotz" -->

<!-- ink:api name="scale" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<dim_t L, typename T> mat<L,L,T> scale(const vec<L,T>& factors)`

Build a pure affine scale matrix from per-axis factors, or apply scale to an existing matrix. Four overloads cover vector factors, scalar uniform scale, and accumulating variants.

### When to use this

Use this (in `namespace tdm::affine`) when working directly with square `mat<L,L,T>` — for example, when constructing a 3×3 scale before composing with a rotation matrix. For homogeneous 4×4 pipelines (rendering, projective transforms), use the `projective::scale` overload instead, which returns `mat<L+1,L+1,T>` with the homogeneous row preserved.

### Example

```cpp
// Non-uniform scale: stretch 2× along X, 0.5× along Y, 1× along Z
tdm::vec3<float> factors{2.0f, 0.5f, 1.0f};
tdm::mat3<float> S = tdm::affine::scale<3, float>(factors);

// Uniform scale applied to an existing rotation matrix
tdm::mat3<float> RS = tdm::affine::scale(R, 1.5f);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `factors` | `vec<L, T>` | required — per-axis scale factors |
| `m` | `mat<L,L,T>` | optional — existing matrix to post-multiply; returns `m * scale(factors)` |
| `factor` | `T` | optional — uniform scalar scale applied to all L axes |

### Returns

`mat<L,L,T>` — a diagonal scale matrix, or the result of pre-multiplying `m` by the scale.

<!-- ink:api-end name="scale" -->

<!-- ink:api name="translate" module="tdm/Transforms" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<dim_t L, typename T> mat<L+1,L+1,T> translate(const vec<L,T>& position)`

Build a homogeneous translation matrix from a position vector. For a 3D vector, returns a 4×4 matrix with the translation in the last row (row-major convention). An accumulating overload computes `m * translate(position)`.

### When to use this

Use this when constructing or composing affine transforms in homogeneous coordinates — for example, building a TRS (translate-rotate-scale) matrix for a rig joint. The translation is placed in row `L` (row-major convention), so compose as `T * R * S` or use the accumulating overload `translate(R_or_RS, position)` to keep the chain readable.

### Example

```cpp
// 3D translation: move 10 units along X
tdm::vec3<float> offset{10.0f, 0.0f, 0.0f};
tdm::mat4<float> T = tdm::translate(offset);  // 4×4 homogeneous translation

// Accumulate: apply translation on top of an existing rotation matrix
tdm::mat4<float> TR = tdm::translate(R, offset);  // R * translate(offset)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `position` | `vec<L,T>` | required — translation vector; components placed into row `L` of an identity matrix |
| `m` | `mat<L+1,L+1,T>` | optional — existing matrix; returns `m * translate(position)` |

### Returns

`mat<L+1,L+1,T>` — a homogeneous translation matrix, or `m` post-multiplied by the translation.

<!-- ink:api-end name="translate" -->
