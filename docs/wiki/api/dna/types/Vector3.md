# API Reference — `dna/types/Vector3`

---

<!-- ink:api name="Vector3" module="dna/types/Vector3" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `Vector3`

A plain 3-component float vector (x, y, z) with the standard set of component-wise arithmetic operators.

### Why this exists

Rig math (joint translations, RBF pose targets, blend weights) needs a lightweight, allocation-free 3D vector that supports the usual arithmetic without pulling in a full math library. Operators are component-wise, not geometric (e.g. `*` is per-component multiply, not a dot or cross product), so callers should not assume vector-algebra semantics beyond addition/subtraction/scaling.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `float` | required — first component. |
| `y` | `float` | required — second component. |
| `z` | `float` | required — third component. |
| `w` (if present in variants) | — | not present on this type — Vector3 has exactly x, y, z. |

### Construction

```cpp
dna::Vector3 translation{1.0f, 0.0f, 0.0f};
dna::Vector3 offset{0.5f, 0.5f, 0.5f};
auto result = translation + offset; // component-wise add
result += 2.0f; // add scalar to every component
```

### Relationships

- `operator+`, `operator-`, `operator*`, `operator/` (free functions) — *component-wise binary arithmetic built on the compound-assignment operators.*

<!-- ink:api-end name="Vector3" -->
