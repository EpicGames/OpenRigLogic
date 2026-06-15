# API Reference — `dna/version/VersionInfo`

---

<!-- ink:api name="VersionInfo" module="dna/version/VersionInfo" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `VersionInfo`

Query the compiled version of the DNA library at runtime.

### Why this exists

The DNA library exposes its version through a single grouped struct rather than free functions or preprocessor macros, keeping version symbols consistently namespaced under `dna::`. All accessors are static, so no instance is required — call them directly without constructing a `VersionInfo` object. This makes version-guard checks straightforward in both C++ consumers and language bindings that import the `dna` namespace.

### Construction

```cpp
// No instantiation needed — all methods are static.

// Numeric components for programmatic version checks:
int major = dna::VersionInfo::getMajorVersion();
int minor = dna::VersionInfo::getMinorVersion();
int patch = dna::VersionInfo::getPatchVersion();

// Human-readable string for logging or diagnostics:
dna::StringView version = dna::VersionInfo::getVersionString();
```

### Relationships

- `StringView` — returned by `getVersionString()`; defined in `dna/types/Aliases.h`

<!-- ink:api-end name="VersionInfo" -->
