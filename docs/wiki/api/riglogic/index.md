# API Reference — `riglogic`

_21 entries across 6 modules._

## By Task

### Main RigLogic API

Runtime evaluation and instance management.

| API | Module | Summary |
|-----|--------|---------|
| [RigLogic](riglogic/RigLogic.md) | RigLogic | Main RigLogic evaluator for rig control computation |
| [RigInstance](riglogic/RigInstance.md) | RigInstance | Per-character instance state for RigLogic evaluation |
| [Configuration](riglogic/Configuration.md) | Configuration | RigLogic configuration and evaluation options |
| [Stats](riglogic/Stats.md) | Stats | Statistics and performance metrics |

### Configuration options

Runtime behavior customization.

| API | Module | Summary |
|-----|--------|---------|
| [CalculationType](riglogic/Configuration.md) | Configuration | Computation method (SSE, AVX, scalar) |
| [FloatingPointType](riglogic/Configuration.md) | Configuration | Floating-point precision (float, half) |
| [RotationType](riglogic/Configuration.md) | Configuration | Rotation representation (quaternion, Euler angles) |
| [ScaleType](riglogic/Configuration.md) | Configuration | Scale representation mode |

### Version information

| API | Module | Summary |
|-----|--------|---------|
| [VersionInfo](version/VersionInfo.md) | VersionInfo | RigLogic runtime version information |

### Type aliases

| API | Module | Summary |
|-----|--------|---------|
| [Aliases](types/Aliases.md) | Aliases | Core type aliases and definitions |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| Aliases | [types/Aliases.md](types/Aliases.md) | 2 |
| Configuration | [riglogic/Configuration.md](riglogic/Configuration.md) | 6 |
| RigInstance | [riglogic/RigInstance.md](riglogic/RigInstance.md) | 5 |
| RigLogic | [riglogic/RigLogic.md](riglogic/RigLogic.md) | 6 |
| Stats | [riglogic/Stats.md](riglogic/Stats.md) | 1 |
| VersionInfo | [version/VersionInfo.md](version/VersionInfo.md) | 1 |
