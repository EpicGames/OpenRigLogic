# API Reference — `riglogic/riglogic/Stats`

---

<!-- ink:api name="Stats" module="riglogic/riglogic/Stats" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Stats`

Snapshot of the resolved computation configuration and resource counts for a loaded RigLogic instance.

### Why this exists

A caller cannot determine which solvers are active or how many deformer elements were loaded from the DNA file alone — that resolution happens at rig-load time. `Stats` provides a single post-load query point for the active computation mode, floating-point precision, and per-category element counts. Use it for validation, diagnostic logging, or conditional code paths that branch on which solver types (RBF, ML, PSD) are present in the loaded rig.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `calculationType` | `CalculationType` | required — the computation backend selected for evaluation (e.g., scalar, SSE, AVX). |
| `floatingPointType` | `FloatingPointType` | required — floating-point precision used during evaluation (e.g., `float32`, `float64`). |
| `rbfSolverCount` | `std::uint16_t` | required — number of active RBF solvers in the loaded rig. |
| `mlOperationCount` | `std::uint16_t` | required — number of machine-learning operations active in the rig. |
| `psdCount` | `std::uint16_t` | required — number of pose-space deformation entries. |
| `blendShapeChannelCount` | `std::uint16_t` | required — number of blend shape channels driven by the rig. |
| `animatedMapCount` | `std::uint16_t` | required — number of animated map entries. |
| `jointCount` | `std::uint16_t` | required — number of joints driven by the rig. |
| `jointDeltaValueCount` | `std::uint32_t` | required — total number of joint delta values; uses `uint32_t` because this count can exceed the 65535 limit of the other counters. |

### Relationships

- `CalculationType` — enum or typedef in `rl4` namespace; controls which computation backend is selected.
- `FloatingPointType` — enum or typedef in `rl4` namespace; controls numeric precision during evaluation.

<!-- ink:api-end name="Stats" -->
