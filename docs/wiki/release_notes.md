# Release Notes

<!-- This file is managed by the Ink plugin. Do not edit anchor tags manually. -->

<!-- ink:start id=3f24d88d type=release_notes conflict_score=1.0 last_confirmed=2026-06-10 created=2026-06-10 -->
## Release notes for v13.2.5 (2026-06-10)

This release includes changes across 16 modules, with the heaviest activity in `include/dna` (5 items), `include/riglogic` (4 items), and `src/riglogic` (4 items). It introduces 11 new features including a consolidated `dna::Configuration` struct, an extended ML behavior API, and new TDM math types. This release contains 6 breaking changes that require call-site updates.

### Highlights

- **include/dna**: New `dna::Configuration` struct consolidates all DNA reader parameters; `BinaryStreamReader::create()` now accepts a single config struct; `MachineLearnedBehaviorExt` API added for structured ML model introspection
- **include/riglogic**: ML API renamed (`calculateMLControls`), neural-network index APIs replaced by structured ML type/operation-set model, `AnyVector`/`HalfFloat` set as new defaults, raw control buffer now mutable
- **include/tdm**: New strongly-typed angle type (`tdm::Ang`), comprehensive math computation utilities, and typed coordinate system types added
- **cmake/install**: New `install_application()` macro and `Uninstall` target added for executable install support
- **src/riglogic**: Internal ML behavior storage restructured to flat `meshRegionCounts` vector aligned with the new ML operation set model

### Features

- **cmake/install**: Added `install_application()` CMake macro and `ConfigApplication.cmake.in` for installing executable targets alongside libraries, plus a new `Uninstall` target
- **include/dna**: Added `dna::Configuration` struct consolidating all DNA reader parameters (layer, LOD range, coordinate system transform policy, rotation sequence/sign, translation unit, face winding order)
- **include/dna**: Added `BinaryStreamReader::create(stream, config, memRes)` single-struct factory as the new primary creation path
- **include/dna**: Added `MachineLearnedBehaviorExt` API: `MachineLearnedBehaviorOperationType` and `MachineLearnedBehaviorParameterKey` enums for fine-grained ML model introspection
- **include/riglogic**: Added `RigLogic::getMLTypeCount()` and `getMLOperationSetCount()` for structured ML model introspection replacing flat neural-network index lists
- **include/riglogic**: Added `FloatingPointType::HalfFloat` to `rl4::Configuration` with half-float as the new default floating-point representation
- **include/riglogic**: Added `CalculationType::AnyVector` as a new default that selects the best available SIMD path at runtime
- **include/riglogic**: `RigInstance::getRawControlValues()` now returns a mutable `ArrayView<float>` enabling direct zero-copy buffer writes
- **include/tdm**: Added `tdm::Ang<T,TUnit>` strongly-typed angle value with degree/radian specialisations, conversion constructors, and `_fdeg`/`_frad` user-defined literals
- **include/tdm**: Added `tdm::Computations.h`: free-function math utilities including `cross`, `dot`, `lerp`, `slerp`, `normalize`, `conjugate`, `inverse`, `transpose`, and `determinant`
- **include/tdm**: Added `tdm::CoordSys` typed coordinate system and axis direction types (`Direction`, `RotationDirection`, `RotationSequence`, `RotationSign`)

### Examples

**examples** — Creating a BinaryStreamReader using the new dna::Configuration struct

```cpp
dna::Configuration dnacfg = {};
dnacfg.layer = rl4::DataLayer::All;
dnacfg.unknownLayerPolicy = rl4::UnknownLayerPolicy::Ignore;
dnacfg.maxLOD = 2;
auto reader = rl4::makeScoped<rl4::BinaryStreamReader>(stream.get(), dnacfg, &memRes);
```

**examples** — Writing controls directly into the mutable raw control buffer returned by getRawControlValues()

```cpp
rl4::ArrayView<float> rawControlBuffer = rigInstance->getRawControlValues();
for (std::uint16_t ctrlIndex = 0u; ctrlIndex < rigInstance->getRawControlCount(); ++ctrlIndex) {
    if (ctrlIndex == controlToManipulate) {
        const float val = std::fabs(std::sin(frame / 1000.0f));
        rawControlBuffer[ctrlIndex] = val;
    }
}
```

### Breaking Changes

- **include/dna**: `BinaryStreamReader::create()` multi-parameter overloads removed; all call sites must migrate to `dna::Configuration` struct
- **include/riglogic**: `rl4::RotationOrder` enum and `Configuration::rotationOrder` field removed; rotation order is now expressed via `dna::Configuration::rotationSequence` and `rotationSign`
- **include/riglogic**: `calculateMachineLearnedBehaviorControls()` renamed to `calculateMLControls()` on `RigLogic`
- **include/riglogic**: `RigLogic::getNeuralNetworkIndicesForLOD()` and `getNeuralNetworkCount()` removed; replaced by `getMLTypeCount()`/`getMLOperationSetCount()`
- **include/riglogic**: `RigInstance::getNeuralNetworkMask()`/`setNeuralNetworkMask()` removed along with `getNeuralNetworkCount()` on `RigInstance`
- **include/riglogic**: `rl4::Configuration` default `calculationType` changed from `SSE` to `AnyVector`; new `floatingPointType` field defaults to `HalfFloat` — may change numerical output on existing callers

### Migration Guide

**include/dna — BinaryStreamReader::create() multi-parameter overloads removed; replaced by single dna::Configuration struct**

Replace positional arguments with a `dna::Configuration` struct: set `cfg.layer`, `cfg.unknownLayerPolicy`, `cfg.maxLOD`, `cfg.minLOD`, then pass `cfg` as the second argument.

Before:
```
static BinaryStreamReader* create(BoundedIOStream* stream, DataLayer layer = DataLayer::All, UnknownLayerPolicy policy = UnknownLayerPolicy::Preserve, std::uint16_t maxLOD = 0u, MemoryResource* memRes = nullptr);
```

After:
```
static BinaryStreamReader* create(BoundedIOStream* stream, const Configuration& config = {}, MemoryResource* memRes = nullptr);
```

**include/riglogic — RotationOrder enum and Configuration::rotationOrder field removed**

Remove any assignment to `config.rotationOrder`. Specify rotation sequence via `dna::Configuration::rotationSequence` (e.g. `rotationSequence = RotationSequence::xyz`) and `rotationSign` if needed.

Before:
```
    RotationOrder rotationOrder = RotationOrder::XYZ;
```

**include/riglogic — calculateMachineLearnedBehaviorControls() renamed to calculateMLControls()**

Rename all call sites from `calculateMachineLearnedBehaviorControls()` to `calculateMLControls()`.

Before:
```
virtual void calculateMachineLearnedBehaviorControls(RigInstance* instance) const = 0;
```

After:
```
virtual void calculateMLControls(RigInstance* instance) const = 0;
```

**include/riglogic — getNeuralNetworkIndicesForLOD() and getNeuralNetworkCount() removed from RigLogic**

Replace with `getMLTypeCount()` / `getMLOperationSetCount(mlTypeIndex)` / `getMLOperationCount(mlTypeIndex, mlOperationSetIndex)` for structured ML model traversal.

Before:
```
virtual ConstArrayView<std::uint32_t> getNeuralNetworkIndicesForLOD(std::uint16_t lod) const = 0;
```

**include/riglogic — RigInstance getNeuralNetworkMask/setNeuralNetworkMask removed**

Remove usage; neural network masking API has been removed. Use the new ML operation set structure for selective evaluation.

Before:
```
virtual float getNeuralNetworkMask(std::uint16_t neuralNetIndex) const = 0;
```

**include/riglogic — rl4::Configuration defaults changed: calculationType SSE→AnyVector, new floatingPointType defaulting to HalfFloat**

If full-precision output is required, explicitly set `config.floatingPointType = FloatingPointType::Float32`. If SSE-only computation is required, set `config.calculationType = CalculationType::SSE`.

Before:
```
    CalculationType calculationType = CalculationType::SSE;
```

After:
```
    CalculationType calculationType = CalculationType::AnyVector;
    FloatingPointType floatingPointType = FloatingPointType::HalfFloat;
```

### Infrastructure

- **(root)**: Code formatting migrated from uncrustify (`.uncrustify.cfg` removed) to clang-format (`.clang-format` added)
- **cmake/install**: `cmake/install/Config.cmake.in` renamed to `ConfigLibrary.cmake.in` to disambiguate from the new `ConfigApplication.cmake.in`
- **docs/uml**: Removed UML architecture diagram and all sequence diagrams from `docs/uml/`
- **python/dna**: Python SWIG bindings updated to reflect new `dna::Configuration` API and `MachineLearnedBehaviorExt` types

### Other Changes

- **(other)**: Minor changes across benchmarks, cmake dependency scripts, and sanitizer configuration
- **include/pma**: Reformatted access specifiers across all pma and trio headers (indentation style change only, no behavioral change)
- **src/riglogic**: ML behavior internal storage changed from `neuralNetworkIndicesPerMeshRegion` (`Vector<Matrix<uint16_t>>`) to flat `meshRegionCounts` (`Vector<uint16_t>`)

**Tags**: release, v13.2.5, dna, riglogic, tdm, pma, trio, cmake
<!-- ink:end id=3f24d88d -->
