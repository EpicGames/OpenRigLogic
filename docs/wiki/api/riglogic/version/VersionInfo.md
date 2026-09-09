# API Reference — `riglogic/version/VersionInfo`

---

<!-- ink:api name="VersionInfo" module="riglogic/version/VersionInfo" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `struct RLAPI VersionInfo`

Reports the RigLogic library version at runtime, as major/minor/patch numbers and a formatted string.

### When to use this

Use this to check which RigLogic version a linked binary was built against — for example, to log it at startup or to verify compatibility with a DNA file format before loading it.

### Method groups

| Group | Methods |
|-------|---------|
| Version | getMajorVersion, getMinorVersion, getPatchVersion, getVersionString |

### Example

```cpp
int major = rl4::VersionInfo::getMajorVersion();
int minor = rl4::VersionInfo::getMinorVersion();
rl4::StringView versionStr = rl4::VersionInfo::getVersionString();
// e.g. major == 13, minor == 2, versionStr == "13.2.9"
```

### Returns

`int` / `StringView` — the numeric version components and the combined human-readable version string, respectively.

<!-- ink:api-end name="VersionInfo" -->
