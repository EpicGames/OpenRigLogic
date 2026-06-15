# API Reference — `tdm/Quat`

---

<!-- ink:api name="euler_to_quat" module="tdm/Quat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `euler_to_quat<T, order>(const rad3<T>& rot, rot_sign signs) -> quat<T>`

Convert a set of Euler angles (in radians) to a unit quaternion, using a compile-time rotation order and per-axis direction signs.

### When to use this

Use when you need to build a quaternion from user-facing Euler angle inputs and the rotation order is fixed at compile time. The `order` template parameter selects the intrinsic rotation sequence (e.g., `rot_seq::xyz` for Rx·Ry·Rz), and `signs` adjusts axis handedness without additional wrapper logic. Use `quat::from_euler` for the same operation from a `quat` call site without instantiating the functor directly.

### Example

```cpp
using namespace tdm;

// Convert XYZ Euler angles (in radians) to a quaternion
rad3<float> rot{rad<float>{0.1f}, rad<float>{0.5f}, rad<float>{0.2f}};
rot_sign signs{1, 1, 1};  // standard positive-axis convention

quat<float> q = impl::euler_to_quat<float, rot_seq::xyz>{}(rot, signs);
// q now holds the equivalent unit quaternion for the given Euler rotation
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `rot` | `const rad3<T>&` | required — three Euler angles in radians, indexed [0]=X, [1]=Y, [2]=Z |
| `signs` | `rot_sign` | required — per-axis sign multipliers applied before conversion; controls rotation direction convention |

### Returns

`quat<T>` — a unit quaternion representing the combined intrinsic rotation in the compile-time `order`.

### Watch out for

- The rotation order is a template parameter, not a runtime argument. Selecting the wrong `order` specialization produces a silently incorrect quaternion. Match `order` to the convention used when the Euler angles were authored.
- `signs` are applied to the input angles before the half-angle cosine/sine computation. A sign of `0` zeroes that axis entirely; use `{1, 1, 1}` for standard conventions.

<!-- ink:api-end name="euler_to_quat" -->

<!-- ink:api name="quat" module="tdm/Quat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `quat<T>`

A unit quaternion representing a 3D orientation, with full arithmetic operator support and Euler-angle construction.

### Why this exists

Raw rotation matrices and Euler angles both carry representation problems at runtime: matrices accumulate floating-point drift under repeated multiplication, and Euler angles suffer from gimbal lock and order ambiguity. `quat<T>` provides a normalized quaternion type that sidesteps both issues while remaining composable via the Hamilton product (`operator*=`). The `from_euler` factory and explicit constructor handle the conversion burden so callers never need to invoke `euler_to_quat` directly.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `value_type` | required — X component of the vector part |
| `y` | `value_type` | required — Y component of the vector part |
| `z` | `value_type` | required — Z component of the vector part |
| `w` | `value_type` | required — scalar (real) part; initialized to `1.0` by the default constructor (identity rotation) |

### Construction

```cpp
using namespace tdm;

// Identity quaternion (no rotation)
quat<float> identity{};

// Explicit XYZW components
quat<float> q{0.0f, 0.0f, 0.707f, 0.707f};  // 90° around Z

// From Euler angles with rotation order and axis signs
rad3<float> rot{rad<float>{0.0f}, rad<float>{0.0f}, rad<float>{1.5708f}};
rot_sign signs{1, 1, 1};
quat<float> q2 = quat<float>::from_euler<rot_seq::xyz>(rot, signs);
```

### Relationships

- `euler_to_quat<T, order>` — internal functor used by `from_euler` and the explicit Euler constructor
- `quat_to_euler<T, order>` — inverse: decomposes a `quat` back to `rad3<T>`
- `rad3<T>` — the Euler-angle input type for construction and decomposition
- `rot_seq` — compile-time enum selecting the intrinsic rotation order

### Constraints

- The default constructor produces an identity quaternion (`w = 1`), not a zero quaternion. Adding a default-constructed `quat` to another is not the same as the additive identity.
- Arithmetic operators (`+=`, `-=`, `*=`) operate on raw component values and do not re-normalize. Repeated Hamilton products accumulate floating-point error; normalize periodically when composing many rotations.

<!-- ink:api-end name="quat" -->

<!-- ink:api name="quat_to_euler" module="tdm/Quat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `quat_to_euler<T, order>::operator()(const quat<T>& q, rot_sign signs) -> rad3<T>`

Decompose a quaternion back into Euler angles (in radians) for a given intrinsic rotation order and axis-sign convention.

### When to use this

Use when you need to present rotation values in Euler form after computation in quaternion space — for example, writing joint angles back to a rig or UI. The `order` template parameter must match the order originally used to construct the quaternion; mismatched orders produce angles that, while mathematically valid, do not reproduce the original rotation when reapplied.

### Example

```cpp
using namespace tdm;

quat<float> q{0.0f, 0.5f, 0.0f, 0.866f};  // approx 60° around Y
rot_sign signs{1, 1, 1};

rad3<float> angles = impl::quat_to_euler<float, rot_seq::xyz>{}(q, signs);
// angles[0] = X rotation, angles[1] = Y rotation, angles[2] = Z rotation
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `q` | `const quat<T>&` | required — the input quaternion to decompose |
| `signs` | `rot_sign` | required — per-axis sign multipliers applied to the output angles to match the target convention |

### Returns

`rad3<T>` — three Euler angles in radians, ordered by axis index (0=X, 1=Y, 2=Z), scaled by `signs`.

### Watch out for

- Near gimbal-lock (when the middle-axis sine component approaches ±1), the implementation sets the first output angle to zero and encodes the remaining rotation in the third axis. This is mathematically correct but may produce unexpected angle distributions when the input quaternion is near a pole.
- The `order` template parameter must match the one used during the original `euler_to_quat` conversion. A round-trip through mismatched orders does not recover the original angles.

<!-- ink:api-end name="quat_to_euler" -->

<!-- ink:api name="value_type" module="tdm/Quat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `quat<T>::value_type`

Type alias that exposes the scalar component type of a `quat<T>` specialization.

### Why this exists

Following the C++ named requirement convention for numeric types, `value_type` lets generic code query the underlying scalar type of a `quat` without repeating the template argument. Use `quat<T>::value_type` in template contexts where you have a `quat` type but not the original `T`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` | required — aliases the template parameter `T` of the enclosing `quat<T>` |

### Relationships

- `quat<T>` — the enclosing type that declares this alias

<!-- ink:api-end name="value_type" -->
