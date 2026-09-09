# API Reference — `dna/version/VersionInfo`

---

<!-- ink:api name="VersionInfo" module="dna/version/VersionInfo" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `struct DNAAPI VersionInfo`

Query the version of the DNA library that the current binary was built against.

### When to use this

Use this to check library compatibility at runtime — for example, verifying a loaded DNA file or plugin was produced against a compatible library version before trusting its data layout.

### Method groups

| Group | Methods |
|-------|---------|
| Version components | getMajorVersion, getMinorVersion, getPatchVersion |
| Version string | getVersionString |

### Example

```cpp
auto major = dna::VersionInfo::getMajorVersion();
auto minor = dna::VersionInfo::getMinorVersion();
auto patch = dna::VersionInfo::getPatchVersion();
auto versionStr = dna::VersionInfo::getVersionString();
// e.g. compare major/minor against the version this integration was built for
```

### Returns

`int` — the major, minor, or patch version number. `StringView` — the full version string.

<!-- ink:api-end name="VersionInfo" -->
