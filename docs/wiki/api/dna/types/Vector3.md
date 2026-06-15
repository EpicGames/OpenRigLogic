# API Reference — `dna/types/Vector3`

---

<!-- ink:api name="Vector3" module="dna/types/Vector3" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `dna::Vector3`

Represent a point or direction in 3D space with component-wise arithmetic.

### Why this exists

`Vector3` gives the DNA API a self-contained, dependency-free 3D vector type so callers do not need an external math library just to work with positions, translations, or normals. It is a plain-old-data struct — no virtual dispatch, no heap allocation — making it safe to copy freely across API boundaries. All four arithmetic operations are provided in both vector–vector and vector–scalar forms, covering every standard linear combination a rig evaluation would need.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `x` | `float` | required — X component. |
| `y` | `float` | required — Y component. |
| `z` | `float` | required — Z component. |

### Construction

```cpp
// Aggregate initialization — no constructor required
dna::Vector3 position{0.5f, 1.0f, 2.0f};
dna::Vector3 direction{1.0f, 0.0f, 0.0f};  // unit-X axis

// Component-wise arithmetic
dna::Vector3 offset{0.1f, 0.0f, -0.05f};
position += offset;               // modifies in place
dna::Vector3 scaled = direction * 2.5f;  // uniform scale
```

<!-- ink:api-end name="Vector3" -->
