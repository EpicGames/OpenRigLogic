# API Reference — `tdm/Ang`

---

<!-- ink:api name="ang" module="tdm/Ang" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ang`

A strongly-typed angle value tagged with its unit (degrees or radians) at compile time.

### Why this exists

`ang` exists so that angle values carry their unit as part of the type, preventing the common bug of mixing degrees and radians in the same expression. Converting between `ang<T, impl::degrees>` and `ang<T, impl::radians>` is done through an explicit converting constructor, so unit conversion becomes visible in code rather than an implicit, easy-to-miss detail.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value` | `value_type` (alias for `T`) | required — the raw numeric angle value in the unit denoted by `TUnit` |

### Construction

```cpp
tdm::ang<double, tdm::impl::degrees> heading{45.0};
tdm::ang<double, tdm::impl::radians> headingRad{heading};  // converts degrees -> radians
```

### Relationships

- `pi` — *used internally to convert between the `degrees` and `radians` unit tags*
- `degrees` / `radians` — *free-function equivalents for converting raw (untyped) values*

### Constraints

- Supports `+=`, `-=`, `*= value_type`, `/= value_type`, unary `-`, `==`, `!=`.

<!-- ink:api-end name="ang" -->

<!-- ink:api name="degrees" module="tdm/Ang" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> T degrees(T radians)`

Convert an angle expressed in radians to degrees.

### When to use this

Use this when you have a raw floating-point radian value (not wrapped in an `ang<T, TUnit>`) and need it in degrees for display, logging, or interop with APIs that expect degrees. For strongly-typed angle values, prefer converting via the `ang` constructor instead, which tracks units at compile time.

### Example

```cpp
double deg = tdm::degrees(1.5707963267948966);
// deg is approximately 90.0
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `radians` | `T` | required — angle value in radians; `T` must be a floating-point type |

### Returns

`T` — the equivalent angle expressed in degrees.

### Watch out for

- Passing an integer literal (e.g., `tdm::degrees(3)`) triggers a SFINAE substitution failure at compile time. Cast to `float` or `double` first.

<!-- ink:api-end name="degrees" -->

<!-- ink:api name="pi" module="tdm/Ang" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `constexpr double pi()`

Get the value of π as a compile-time constant for use in angle conversions and trigonometric calculations.

### When to use this

Use this instead of hardcoding a π literal so that all angle-unit conversions in the codebase (`degrees`, `radians`) stay consistent with a single source of truth.

### Example

```cpp
constexpr double circumference = 2.0 * tdm::pi() * radius;
// circumference computed using the library's canonical pi value
```

### Returns

`double` — the mathematical constant π (3.14159265358979323846).

### Constraints

- `constexpr` — always evaluated at compile time; zero runtime overhead.

<!-- ink:api-end name="pi" -->

<!-- ink:api name="radians" module="tdm/Ang" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> T radians(T degrees)`

Convert an angle expressed in degrees to radians.

### When to use this

Use this when you have a raw floating-point degree value and need it in radians for trigonometric functions (`std::sin`, `std::cos`, etc.), which expect radians. For strongly-typed angle values, prefer the `ang` constructor instead.

### Example

```cpp
double rad = tdm::radians(90.0);
// rad is approximately 1.5707963267948966
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `degrees` | `T` | required — angle value in degrees; `T` must be a floating-point type |

### Returns

`T` — the equivalent angle expressed in radians.

### Watch out for

- Like `degrees()`, integer inputs cause a compile-time SFINAE failure. Cast to a floating-point type before calling.

<!-- ink:api-end name="radians" -->
