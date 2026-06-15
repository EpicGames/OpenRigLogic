# API Reference — `dna/layers/Twist`

---

<!-- ink:api name="TwistAxis" module="dna/layers/Twist" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `TwistAxis`

Select the axis along which a twist deformation is applied.

### Why this exists

A raw integer or string axis specifier would allow out-of-range values that are only caught at runtime. `TwistAxis` restricts callers to exactly three valid choices — X, Y, or Z — enforcing correctness at compile time. As a scoped `enum class`, it prevents silent implicit conversion to `int`, making axis-passing code self-documenting and type-safe.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `X` | enumerator | The X axis. |
| `Y` | enumerator | The Y axis. |
| `Z` | enumerator | The Z axis. |

### Construction

```cpp
// Apply twist along the Y axis
dna::TwistAxis axis = dna::TwistAxis::Y;
```

### Constraints

- Exactly three valid values: `X`, `Y`, `Z`.
- Scoped `enum class` — implicit conversion to `int` is not allowed; use `static_cast<int>` if an integer representation is required.

<!-- ink:api-end name="TwistAxis" -->
