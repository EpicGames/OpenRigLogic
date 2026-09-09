# API Reference — `riglogic`

_19 entries across 6 modules._

## By Task

### Rig evaluation

Load, configure, and evaluate a rig to produce joint, blend shape, and animation output

| API | Module | Summary |
|-----|--------|---------|
| [RigInstance](riglogic/RigInstance.md) | RigInstance | Holds the instance-specific input and output buffers for evaluating one instance of a rig, including level-of-detail and control values. |
| [RigInstance](riglogic/RigLogic.md) | RigLogic | Forward-declared here as the counterpart type that `RigLogic` creates and evaluates instances of; see the `RigInstance` entry in `riglogic/riglogic/RigInstance` for its full interface. |
| [RigLogic](riglogic/RigInstance.md) | RigInstance | Evaluates a `RigInstance` to produce joint, blend shape, and animated map output values. Pass a `RigInstance` to `RigLogic::calculate` to drive the rig for a given frame. |
| [RigLogic](riglogic/RigLogic.md) | RigLogic | Loads and optimizes DNA rig data, then calculates rig output values for any number of `RigInstance` objects based on their input control values. |

### Configuration and output types

Select CPU algorithm, floating point precision, and output representations for rotations, translations, and scales

| API | Module | Summary |
|-----|--------|---------|
| [CalculationType](riglogic/Configuration.md) | Configuration | Selects which CPU algorithm implementation RigLogic uses to evaluate a rig. |
| [Configuration](riglogic/Configuration.md) | Configuration | The set of options that determines how RigLogic loads DNA data and evaluates a rig. |
| [Configuration](riglogic/RigLogic.md) | RigLogic | An alias, inside `RigLogic`, for `rl4::Configuration` — the options struct used to create and inspect a `RigLogic` instance. |
| [FloatingPointType](riglogic/Configuration.md) | Configuration | Selects the floating point precision used in RigLogic's vectorized calculations. |
| [RotationType](riglogic/Configuration.md) | Configuration | Selects the representation RigLogic uses for rotation output values. |
| [ScaleType](riglogic/Configuration.md) | Configuration | Selects the representation RigLogic uses for scale output values. |
| [TranslationType](riglogic/Configuration.md) | Configuration | Selects the representation RigLogic uses for translation output values. |

### Memory management

Traits for memory management utilities to construct and destroy RigLogic and RigInstance objects

| API | Module | Summary |
|-----|--------|---------|
| [DefaultInstanceCreator](riglogic/RigInstance.md) | RigInstance | A `pma` trait specialization that tells the memory management utilities how to construct a `RigInstance` by default. |
| [DefaultInstanceCreator](riglogic/RigLogic.md) | RigLogic | A `pma` trait specialization that tells the memory management utilities how to construct a `RigLogic` by default. |
| [DefaultInstanceDestroyer](riglogic/RigInstance.md) | RigInstance | A `pma` trait specialization that tells the memory management utilities how to destroy a `RigInstance` by default. |
| [DefaultInstanceDestroyer](riglogic/RigLogic.md) | RigLogic | A `pma` trait specialization that tells the memory management utilities how to destroy a `RigLogic` by default. |

### Data structures and introspection

Lightweight views over arrays, rig statistics, and version information

| API | Module | Summary |
|-----|--------|---------|
| [ArrayView](types/Aliases.md) | Aliases | An alias for `dna::ArrayView<T>`, a lightweight mutable view over a contiguous array of `T`. |
| [ConstArrayView](types/Aliases.md) | Aliases | An alias for `dna::ConstArrayView<T>`, a lightweight read-only view over a contiguous array of `T`. |
| [Stats](riglogic/Stats.md) | Stats | A snapshot of counts and configuration describing the size and shape of a loaded rig. |
| [VersionInfo](version/VersionInfo.md) | VersionInfo | Reports the RigLogic library version at runtime, as major/minor/patch numbers and a formatted string. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| Configuration | [riglogic/Configuration.md](riglogic/Configuration.md) | 6 |
| RigInstance | [riglogic/RigInstance.md](riglogic/RigInstance.md) | 4 |
| RigLogic | [riglogic/RigLogic.md](riglogic/RigLogic.md) | 5 |
| Stats | [riglogic/Stats.md](riglogic/Stats.md) | 1 |
| Aliases | [types/Aliases.md](types/Aliases.md) | 2 |
| VersionInfo | [version/VersionInfo.md](version/VersionInfo.md) | 1 |
