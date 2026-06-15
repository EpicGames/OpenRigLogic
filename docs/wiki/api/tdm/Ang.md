# API Reference — `tdm/Ang`

---

<!-- ink:api name="ang" module="tdm/Ang" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ang<T, TUnit>`

A strongly-typed angle parameterized by numeric scalar type and unit tag. Prevents degrees and radians from being silently interchanged.

### Why this exists

Raw floating-point angles lose unit semantics — a function taking `double` cannot enforce that you haven't mixed a radian value with a degree value. `ang<T, TUnit>` encodes the unit into the type so the compiler rejects implicit cross-unit assignments. Explicit converting constructors (radians ↔ degrees) make every unit conversion visible and auditable at the call site.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value` | `value_type` | required — the raw angle magnitude in the unit specified by `TUnit` |

### Construction

```cpp
// Default-initialize to zero
ang<float, impl::radians> zero{};

// Construct from a scalar in the native unit
ang<double, impl::degrees> rotation{90.0};

// Explicitly convert radians → degrees
ang<float, impl::radians> r{tdm::radians(45.0f)};
ang<float, impl::degrees> d{r}; // explicit converting constructor
```

### Relationships

- `impl::degrees` — unit tag for degree-valued angles
- `impl::radians` — unit tag for radian-valued angles
- `tdm::pi()` — used internally by the converting constructors
- `value_type` — the member alias for `T`, the scalar type

### Constraints

- Cross-unit construction is marked `explicit` — no implicit conversion between `ang<T, impl::degrees>` and `ang<T, impl::radians>`.
- Supports `+=`, `-=`, `*= value_type`, `/= value_type`, unary `-`, `==`, `!=`.

<!-- ink:api-end name="ang" -->

<!-- ink:api name="degrees" module="tdm/Ang" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> T degrees(T radians)`

Convert a radian angle to degrees. The conversion factor 180/π is computed at compile time.

### When to use this

Use when you receive an angle from a math or physics function (which conventionally works in radians) and need a human-readable degree value or must pass the result to an API that expects degrees. For the inverse, use `tdm::radians()`.

### Example

```cpp
// Convert a half-turn from radians to degrees
float deg = tdm::degrees(static_cast<float>(tdm::pi())); // 180.0f

// Construct a degree-unit ang from a radian value
double rad_val = tdm::pi() / 4.0; // 45° in radians
double deg_val = tdm::degrees(rad_val); // 45.0
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `radians` | `T` | required — angle in radians; `T` must be a floating-point type (`float`, `double`, `long double`) |

### Returns

`T` — the equivalent angle in degrees, same type as the input.

### Watch out for

- Passing an integer literal (e.g., `tdm::degrees(3)`) triggers a SFINAE substitution failure at compile time. Cast to `float` or `double` first.

<!-- ink:api-end name="degrees" -->

<!-- ink:api name="pi" module="tdm/Ang" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `inline constexpr double pi()`

Return the value of π with full double precision. Use this instead of `M_PI` or a magic literal to ensure consistent precision across all TDM math operations.

### When to use this

Use inside `constexpr` expressions where you need π directly — for example, in custom angle formulas not covered by `degrees()` or `radians()`. Prefer `tdm::degrees()` and `tdm::radians()` for the common radian/degree conversions; they call `pi()` internally with the same constant.

### Example

```cpp
// Convert 60° to radians manually using pi()
constexpr double angle_rad = 60.0 * tdm::pi() / 180.0;

// Or use the convenience helper (preferred)
constexpr double angle_rad2 = tdm::radians(60.0);
```

### Returns

`double` — the value 3.14159265358979323846, evaluated at compile time.

### Constraints

- `constexpr` — always evaluated at compile time; zero runtime overhead.

<!-- ink:api-end name="pi" -->

<!-- ink:api name="radians" module="tdm/Ang" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> T radians(T degrees)`

Convert a degree angle to radians. The conversion factor π/180 is computed at compile time.

### When to use this

Use when you have an angle from user input, a configuration file, or an artist-facing tool (which typically works in degrees) and need to pass it to a trigonometric function or construct an `ang<T, impl::radians>`. For the inverse, use `tdm::degrees()`.

### Example

```cpp
// Convert 90 degrees to radians
double rad = tdm::radians(90.0); // ~1.5708

// Construct a radian ang from a degree literal
ang<float, impl::radians> joint_angle{tdm::radians(45.0f)}; // π/4
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `degrees` | `T` | required — angle in degrees; `T` must be a floating-point type (`float`, `double`, `long double`) |

### Returns

`T` — the equivalent angle in radians, same type as the input.

### Watch out for

- Like `degrees()`, integer inputs cause a compile-time SFINAE failure. Cast to a floating-point type before calling.

<!-- ink:api-end name="radians" -->

<!-- ink:api name="value_type" module="tdm/Ang" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `value_type`

The numeric scalar type used to store the angle magnitude inside an `ang<T, TUnit>` instance.

### Why this exists

`value_type` exposes the template parameter `T` as a named member type alias following the standard library convention (`std::vector<T>::value_type`, etc.). Generic code that operates on `ang` specializations can use `ang<T, TUnit>::value_type` to inspect or rebind the underlying scalar type without hard-coding the template argument.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | alias for `T` | required — the scalar floating-point type (e.g., `float`, `double`) used to store the angle's numeric magnitude |

### Relationships

- `ang<T, TUnit>` — the containing struct that declares this alias
- `value` — the `ang` member whose type is `value_type`

<!-- ink:api-end name="value_type" -->
