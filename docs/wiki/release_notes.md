# Release Notes

<!-- This file is managed by the Ink plugin. Do not edit anchor tags manually. -->

<!-- ink:start id=3f24d88d type=release_notes conflict_score=1.0 last_confirmed=2026-06-10 created=2026-06-10 -->
## Release v13.2.5

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

<!-- ink:start id=74910689 type=release_notes conflict_score=1.0 last_confirmed=2026-07-31 created=2026-07-31 -->
## Release v13.2.7

This release spans four modules, with most of the work in `dna` (5 changes), the build root (3), and `include` (2). It adds two features and four bug fixes, all centred on coordinate system conversion fidelity. There are no breaking changes; tdm moves from 6.0.0 to 6.0.1.

### Highlights

- **dna**: Twist and swing axes are now remapped during coordinate system conversion, and RBF pose quaternions are conjugated directly rather than through an euler roundtrip
- **dna**: Layers are explicitly indexed so converted data survives raw-copy serialization of v2.1 and pre-v2.7 sources
- **include**: Six gimbal-lock sign corrections in `mat_to_euler` across four rotation sequences
- **cmake**: Registered a new exhaustive matrix/euler round-trip test suite
- **(root)**: Benchmark targets are now sanitized alongside the benchmark runner

### Features

- **dna**: Added `CoordinateSystemConverter::convertTwistAxes`, which remaps twist, swing, and RBF-solver twist axes to the dominant destination axis during coordinate system conversion
- **dna**: Added `DNA::ensureLayerIndexed` to create a layer index entry when one is absent

### Examples

**dna** — after a Maya-to-UE conversion, twist, swing, and RBF-solver axes are remapped to the dominant destination axis

```cpp
ASSERT_EQ(reader->getTwistSetupTwistAxis(0), TwistAxis::Z);
ASSERT_EQ(reader->getTwistSetupTwistAxis(1), TwistAxis::X);
ASSERT_EQ(reader->getTwistSetupTwistAxis(2), TwistAxis::Y);
ASSERT_EQ(reader->getSwingSetupTwistAxis(0), TwistAxis::Z);
ASSERT_EQ(reader->getSwingSetupTwistAxis(1), TwistAxis::X);
ASSERT_EQ(reader->getSwingSetupTwistAxis(2), TwistAxis::Y);
```

**include** — the new round-trip check: euler angles need not match, but the rotation they encode must

```cpp
float roundTripError(tdm::rot_seq seq, tdm::rot_sign signs, const tdm::frad3& euler) {
    const tdm::mat3<float> m = tdm::impl::euler2mat<float>(euler, seq, signs);
    const tdm::frad3 extracted = tdm::impl::mat2euler<float>(m, seq, signs);
    const tdm::mat3<float> reconstructed = tdm::impl::euler2mat<float>(extracted, seq, signs);
    return maxMatrixDiff(m, reconstructed);
}
```

### Bug Fixes

- **dna**: RBF pose quaternions are now conjugated directly as `q' = (det(C) * (v * C), w)` instead of via an euler roundtrip, preserving the q versus -q distinction that half-rotation RBF solvers rely on to track driver rotations beyond 180 degrees
- **dna**: v2.1 monolithic DNAs now explicitly index the descriptor, definition, behavior, and geometry layers before the format upgrade, so they are not silently dropped when serializing with `UpgradeFormatPolicy::Disallowed`
- **dna**: The DescriptorExt layer is now explicitly indexed so converted rotation and winding conventions survive raw-copy serialization from sources older than v2.7
- **include**: Corrected the `atan2` sign in six `mat_to_euler` gimbal-lock branches across rotation sequences xyz, xzy, yzx, and zxy

### Infrastructure

- **(root)**: CMake now collects benchmark targets into `RL_BENCHMARK_TARGETS` and passes them to `sanitize_targets`, so the bundled `benchmark` target is sanitized alongside the runner
- **(root)**: Added `/.clang-format-tool` to `.gitignore`
- **cmake**: Registered `tests/tdmtests/TestMatEulerConverter.cpp` in the test source list

### Other Changes

- **(root)**: `CMakeLists.txt` `RL_VERSION` raised from 13.2.5 to 13.2.7
- **include**: Version bumps: RigLogic 13.2.5 to 13.2.7, DNA 10.1.3 to 10.1.5, tdm 6.0.0 to 6.0.1

**Tags**: release, v13.2.7, include, dna, root, cmake
<!-- ink:end id=74910689 -->

<!-- ink:start id=f250a654 type=release_notes conflict_score=1.0 last_confirmed=2026-07-31 created=2026-07-31 -->
## Release v13.2.8

This release is a focused change to how scale is converted between coordinate systems, touching three modules. It contains one breaking change: `tdm::convert_scale` now requires an explicit `sign_policy` argument, moving tdm from 6.0.1 to 7.0.0. Two call sites in `dna` and `riglogic` adopt `sign_policy::preserve` so signed scale deltas keep their sign through an axis permutation.

### Highlights

- **include**: `tdm::convert_scale` gains a required `sign_policy` argument — a breaking change, reflected in the tdm 6.0.1 to 7.0.0 major bump
- **include**: New `tdm::sign_policy` enum distinguishes absolute scale magnitudes from signed scale deltas
- **dna**: Joint scale deltas now preserve their sign through coordinate system conversion
- **riglogic**: Machine-learned scale deltas now preserve their sign through coordinate system conversion

### Features

- **include**: Added the `tdm::sign_policy` enum (`discard`, `preserve`) to control whether a change of basis preserves per-component scale signs

### Examples

**include** — `sign_policy::preserve` permutes the axes but keeps each component's own sign

```cpp
const tdm::fvec3 maya_delta{-0.5f, 0.5f, -0.25f};  // Maya (mx, my, mz) -> UE (mz, mx, my)

const auto preserved = tdm::convert_scale(maya_delta, maya_cs, ue_cs, tdm::sign_policy::preserve);
ASSERT_NEAR(preserved[0], -0.25f, 0.001f);  // mz
ASSERT_NEAR(preserved[1], -0.5f, 0.001f);   // mx
ASSERT_NEAR(preserved[2], 0.5f, 0.001f);    // my
```

**include** — `sign_policy::discard` reproduces the pre-7.0.0 absolute-value behaviour

```cpp
const tdm::fvec3 ue_scale{1.0f, 3.0f, 2.0f};
const auto maya_scale = tdm::convert_scale(ue_scale, ue_cs, maya_cs, tdm::sign_policy::discard);

ASSERT_NEAR(maya_scale[0], 3.0f, 0.001f);  // uy
ASSERT_NEAR(maya_scale[1], 2.0f, 0.001f);  // uz
ASSERT_NEAR(maya_scale[2], 1.0f, 0.001f);  // ux
```

### Bug Fixes

- **dna**: Joint scale deltas now convert with `sign_policy::preserve`, so negative deltas are no longer corrupted by an absolute value
- **riglogic**: Machine-learned scale deltas now convert with `sign_policy::preserve`, so negative deltas survive the axis permutation

### Breaking Changes

- **include**: `tdm::convert_scale` now requires an explicit `sign_policy` argument on both the `mat3` and `coord_sys` overloads; tdm major version bumped from 6.0.1 to 7.0.0

### Migration Guide

**include — `tdm::convert_scale` now requires an explicit `sign_policy` argument**

Pass `tdm::sign_policy::discard` at existing call sites to keep the previous absolute-value behaviour, or `sign_policy::preserve` when the vector is a signed scale delta.

Before:
```
inline vec3<T> convert_scale(const vec3<T>& scale, const mat3<T>& c) {
```

After:
```
inline vec3<T> convert_scale(const vec3<T>& scale, const mat3<T>& c, sign_policy policy) {
```

### Other Changes

- **include**: Version bumps: RigLogic 13.2.7 to 13.2.8, DNA 10.1.5 to 10.1.6, tdm 6.0.1 to 7.0.0

**Tags**: release, v13.2.8, include, dna, riglogic
<!-- ink:end id=f250a654 -->

<!-- ink:start id=d0f5e815 type=release_notes conflict_score=1.0 last_confirmed=2026-09-09 created=2026-09-09 -->
## Release v13.2.9

This release includes changes across three modules, with all functional work in the riglogic ML CPU evaluator (three bug fixes) plus the version bump to 13.2.9. Single upstream snapshot import (UE5.8.3); no breaking changes.

### Highlights

- **riglogic**: ML CPU evaluator correctness fixes — operation indices stay aligned with the DNA when ops are skipped, and WeightedSum consumers can no longer read past a dependency's output buffer
- **(root)**: RigLogic version bumped from 13.2.7 to 13.2.9

### Bug Fixes

- **riglogic**: Skipped ML operations (non-MLP type or layer-less networks) are now kept as inert, layer-less placeholder entries so op indices remain aligned with the DNA — LOD lists and cross-op dependency indices reference original op indices; `execute()` skips layer-less ops so placeholders contribute nothing at runtime
- **riglogic**: When a WeightedSum op reads more elements from a dependency buffer than the dependency's true output width, the dependency's buffer is now grown so the read stays in bounds, and MLP dependencies mark the over-read tail via the new `tailZeroCount` field (zeroed padding floats)
- **riglogic**: Masked-op default-value scatter now uses the per-LOD output count (`outputCountsPerLOD[lod]`) as its limit instead of the full `defaultValues` size

### Infrastructure

- **(root)**: Bumped `RL_VERSION` from 13.2.7 to 13.2.9 in CMakeLists.txt
- **include/riglogic**: Updated version constants and `RL_VERSION_STRING` to 13.2.9 in Version.h

**Tags**: release, v13.2.9, riglogic, ml, cpu, cmake, include
<!-- ink:end id=d0f5e815 -->

<!-- ink:start id=f3d074e9 type=release_notes conflict_score=1.0 last_confirmed=2026-09-30 created=2026-09-30 -->
## Release v13.5.0

This release spans twelve modules, with the bulk of the work in riglogic (118 files: serialization validation infrastructure), dna (44 files: malformed-input rejection), and trio (21 files: stream API additions and a stdio-based FileStream). It adds four features and four bug fixes centred on hardening dump/restore and DNA loading against malformed data, plus a library-wide rework of the DLL export macros; no breaking changes. RigLogic version moves from 13.2.9 to 13.5.0.

### Highlights

- **riglogic**: Restored rigs are now validated — per-module validators (joints, storage, controls, blend shapes, conditional tables, animated maps, ML behavior) check dumped data offsets and sizes against rig metadata before use
- **dna**: The binary reader rejects malformed DNA with `InvalidDataError` instead of clamping it into a rig that silently differs from the file
- **trimd**: One binary can now host both AVX-only and AVX2+FMA SIMD kernels, selected by runtime CPU feature detection
- **(root)**: New `RL_BUILD_WITH_AVX2` / `RL_BUILD_WITH_AVX512F` build options; RigLogic version bumped to 13.5.0

### Features

- **riglogic**: Added `SerializationContext` (passed via the archive's user-data pointer during `dump()`/`restore()`) and per-module validators so each `load()` validates its offsets and sizes against the rig dimensions before use
- **riglogic**: Added `BoundedInputArchive`, a bounds-checked input archive now used by all evaluator `load()` paths in place of the raw `terse::BinaryInputArchive`
- **trimd**: Parameterized the SIMD wrappers on a compile-time `FPModel` tag (`Precise`/`Fast`) so the same kernel template produces distinct coexisting symbols — an AVX-only arm and an AVX2+FMA arm — with the runtime dispatcher picking by detected CPU features; `F256` remains as the backwards-compatible `Precise` alias
- **trio**: Added the `Mappable` stream concept and a default no-op `BoundedIOStream::flush()` (buffering streams override it); exposed `flush` through the Python bindings

### Bug Fixes

- **dna**: `DenormalizedData::populate` and the new stream `Validator` reject malformed sources — mismatched parallel parameter arrays, joint-group LOD row counts past the group's rows, joint indices past the joint count, and ML joint outputs with no recognized attribute width — reporting `InvalidDataError` instead of clamping
- **terse**: Bool deserialization validates the wire value (a byte greater than 1 marks the archive malformed, reported via `isOk()`), and `is_batchable` now admits enums while excluding `bool`
- **python**: Fixed the SWIG runtime type-table name — the previous quoted, dot-containing capsule name broke `PyCapsule_Import` type lookups (null-deref on bad arguments); now a plain version-suffixed identifier
- **python**: `setMetaData(key, None)` is accepted again, matching the `DescriptorWriter` contract where a null value deletes the key

### Infrastructure

- **include**: Reworked the DLL export/visibility macros across all public headers (`RLAPI_TYPE`/`RLAPI_MEMBER`, `DNAAPI_TYPE`/`DNAAPI_MEMBER`, and template instantiation declaration/definition halves) to attach the correct attribute per export mechanism — visibility vs `dllexport`/`dllimport` — per compiler and ABI
- **(root)**: Bumped `RL_VERSION` from 13.2.9 to 13.5.0; added `RL_AUTODETECT_AVX2`/`RL_BUILD_WITH_AVX2` and AVX-512F options; registered the new validator sources and test suites in the build

### Other Changes

- **trio**: Reimplemented `FileStream` over C stdio with explicit read/write positioning (replacing `std::fstream`), shared-open semantics on Windows (`_wfsopen`/`_SH_DENYNO`), and `fstat`-based size queries where available

**Tags**: release, v13.5.0, riglogic, dna, trio, trimd, terse, python
<!-- ink:end id=f3d074e9 -->
