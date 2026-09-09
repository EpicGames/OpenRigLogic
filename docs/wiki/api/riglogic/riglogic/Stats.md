# API Reference — `riglogic/riglogic/Stats`

---

<!-- ink:api name="Stats" module="riglogic/riglogic/Stats" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `Stats`

A snapshot of counts and configuration describing the size and shape of a loaded rig.

### Why this exists

Callers often need to inspect how large or how configured a loaded rig is — for logging, memory budgeting, or diagnostics — without walking every individual data structure inside RigLogic. `Stats` gathers these counts and the active calculation/floating point settings into one lightweight, read-only value.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `calculationType` | `CalculationType` | Algorithm implementation currently in use. |
| `floatingPointType` | `FloatingPointType` | Floating point precision currently in use. |
| `rbfSolverCount` | `std::uint16_t` | Number of RBF solvers in the rig. |
| `mlOperationCount` | `std::uint16_t` | Number of ML operations in the rig. |
| `psdCount` | `std::uint16_t` | Number of PSD (pose space deformation) entries in the rig. |
| `blendShapeChannelCount` | `std::uint16_t` | Number of blend shape channels in the rig. |
| `animatedMapCount` | `std::uint16_t` | Number of animated maps in the rig. |
| `jointCount` | `std::uint16_t` | Number of joints in the rig. |
| `jointDeltaValueCount` | `std::uint32_t` | Number of joint delta values stored for the rig. |

### Relationships

- `CalculationType`, `FloatingPointType` — *enum types reused from `Configuration` to describe the active settings.*

<!-- ink:api-end name="Stats" -->
