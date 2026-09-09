# API Reference — `dna/layers/Twist`

---

<!-- ink:api name="TwistAxis" module="dna/layers/Twist" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `TwistAxis`

Identifies which local axis (X, Y, or Z) a twist or swing transformation is measured around.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `X` | enumerator | Twist/swing measured around the X axis. |
| `Y` | enumerator | Twist/swing measured around the Y axis. |
| `Z` | enumerator | Twist/swing measured around the Z axis. |

### Relationships

- `TwistSwingBehaviorReader::getTwistSetupTwistAxis` / `getSwingSetupTwistAxis` — *return this axis for a given twist/swing parameter group.*
- `RBFDistanceMethod::SwingAngle` / `TwistAngle` — *use this axis when measuring RBF input distance.*

### Constraints

- Exactly three valid values: `X`, `Y`, `Z`.
- Scoped `enum class` — implicit conversion to `int` is not allowed; use `static_cast<int>` if an integer representation is required.

<!-- ink:api-end name="TwistAxis" -->
