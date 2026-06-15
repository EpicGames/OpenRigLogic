# API Reference — `riglogic/version/VersionInfo`

---

<!-- ink:api name="VersionInfo" module="riglogic/version/VersionInfo" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `struct VersionInfo`

Query the compiled RigLogic library version at runtime. Use this to verify the loaded library matches the expected version before making version-dependent API calls.

### When to use this

Reach for `VersionInfo` when logging the library version at startup, asserting a minimum version in a plugin loader, or diagnosing mismatches between a compiled host and a dynamically loaded RigLogic build. All four accessors are static — no instance is needed.

### Example

```cpp
#include "riglogic/version/VersionInfo.h"
#include <cassert>

// Assert minimum required version at plugin load time
assert(rl4::VersionInfo::getMajorVersion() == 4);
assert(rl4::VersionInfo::getMinorVersion() >= 1);

// Log the full version string for diagnostics
const auto version = rl4::VersionInfo::getVersionString();
// version == "4.1.0" (or equivalent for the compiled library)
```

### Returns

| Method | Return type | Description |
|--------|-------------|-------------|
| `getMajorVersion()` | `int` | Major version component of the compiled library. |
| `getMinorVersion()` | `int` | Minor version component of the compiled library. |
| `getPatchVersion()` | `int` | Patch version component of the compiled library. |
| `getVersionString()` | `StringView` | Full version string (e.g., `"4.1.0"`). Non-owning view — do not store past the library's lifetime. |

<!-- ink:api-end name="VersionInfo" -->
