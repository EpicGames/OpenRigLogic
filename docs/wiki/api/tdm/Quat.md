# API Reference — `tdm/Quat`

---

<!-- ink:api name="euler_to_quat" module="tdm/Quat" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `quat<T> impl::euler_to_quat<T, order>::operator()(const rad3<T>& rot, rot_sign signs)`

Build a rotation quaternion from Euler angles in a specific axis order, without going through an intermediate rotation matrix.

### When to use this

Use this when converting user- or animation-authored Euler angles (e.g. XYZ, XZY, YXZ intrinsic order) directly into a `quat` for interpolation or composition. Each `rot_seq` value has its own specialization, since the half-angle sum/difference formulas differ per axis order.

### Example

```cpp
rad3<float> euler{rad<float>{0.1f}, rad<float>{0.2f}, rad<float>{0.3f}};
quat<float> q = quat<float>::from_euler<rot_seq::xyz>(euler, rot_sign{1, 1, 1});
// q now holds the equivalent rotation as a quaternion
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `rot` | `const rad3<T>&` | required — the per-axis rotation angles, in radians. |
| `signs` | `rot_sign` | required — per-axis rotation direction sign, applied before computing the half-angle sine/cosine terms. |

### Returns

`quat<T>` — the quaternion equivalent to composing the per-axis rotations in the specialization's intrinsic order (e.g. `Rx * Ry * Rz` for `rot_seq::xyz`).

### Watch out for

- Each `rot_seq` value is a separate template specialization with its own sign-flip pattern in the `x`/`y`/`z`/`w` formulas — do not assume the XYZ formula generalizes to other orders.
- The rotation order is a template parameter, not a runtime argument. Selecting the wrong `order` specialization produces a silently incorrect quaternion. Match `order` to the convention used when the Euler angles were authored.

<!-- ink:api-end name="euler_to_quat" -->

<!-- ink:api name="quat" module="tdm/Quat" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `quat<T>`

A quaternion representing a rotation in 3D space, stored as `x`, `y`, `z`, `w` components.

### Why this exists

`quat` exists because composing and interpolating rotations with raw matrices or Euler angles is error-prone — matrix composition accumulates numerical drift and Euler angles suffer gimbal lock. The default constructor produces the identity rotation (`w = 1`), and `operator*=` implements Hamilton product composition directly, so rotation chaining stays numerically well-behaved.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `value_type` | required — the i-component of the quaternion. |
| `y` | `value_type` | required — the j-component of the quaternion. |
| `z` | `value_type` | required — the k-component of the quaternion. |
| `w` | `value_type` | required — the scalar (real) component; defaults to `1` for the identity rotation. |

### Construction

```cpp
// Identity rotation
quat<float> identity;

// From explicit components
quat<float> q{0.0f, 0.0f, 0.0f, 1.0f};

// From Euler angles with an explicit rotation order and sign convention
rad3<float> euler{rad<float>{0.1f}, rad<float>{0.2f}, rad<float>{0.3f}};
quat<float> from_angles = quat<float>::from_euler<rot_seq::xyz>(euler, rot_sign{1, 1, 1});
```

### Relationships

- `euler_to_quat` — *builds a `quat` from `rad3` Euler angles for a given `rot_seq`.*
- `quat_to_euler` — *decomposes a `quat` back into `rad3` Euler angles.*

### Constraints

- The default constructor produces an identity quaternion (`w = 1`), not a zero quaternion. Adding a default-constructed `quat` to another is not the same as the additive identity.
- Arithmetic operators (`+=`, `-=`, `*=`) operate on raw component values and do not re-normalize. Repeated Hamilton products accumulate floating-point error; normalize periodically when composing many rotations.

<!-- ink:api-end name="quat" -->

<!-- ink:api name="quat_to_euler" module="tdm/Quat" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `rad3<T> impl::quat_to_euler<T, order>::operator()(const quat<T>& q, rot_sign signs)`

Recover Euler angles from a quaternion in a specific axis order — the inverse of `euler_to_quat`.

### When to use this

Use this when you need human-readable or DCC-compatible Euler angles from a quaternion produced by interpolation or composition. The `rot_seq::xyz` specialization clamps `sy` near ±1 to detect gimbal lock and falls back to a reduced two-angle solution instead of producing a NaN or unstable result.

### Example

```cpp
quat<float> q{0.0f, 0.0f, 0.0f, 1.0f};
rad3<float> euler = impl::quat_to_euler<float, rot_seq::xyz>()(q, rot_sign{1, 1, 1});
// euler now holds the equivalent XYZ intrinsic Euler angles, in radians
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q` | `const quat<T>&` | required — the quaternion to decompose. |
| `signs` | `rot_sign` | required — per-axis sign applied to the extracted angles to match the original rotation direction convention. |

### Returns

`rad3<T>` — the per-axis Euler angles, in radians, for the specialization's rotation order.

### Watch out for

- Near the gimbal-lock boundary (`|sy| >= 0.99999999999`), one axis is forced to zero and the remaining two angles are combined into a single `atan2` term — the decomposition is not unique there, so round-tripping through `euler_to_quat` may not reproduce the original input angles exactly.
- Near gimbal-lock (when the middle-axis sine component approaches ±1), the implementation sets the first output angle to zero and encodes the remaining rotation in the third axis. This is mathematically correct but may produce unexpected angle distributions when the input quaternion is near a pole.
- The `order` template parameter must match the one used during the original `euler_to_quat` conversion. A round-trip through mismatched orders does not recover the original angles.

<!-- ink:api-end name="quat_to_euler" -->
