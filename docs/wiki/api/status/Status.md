# API Reference — `status/Status`

---

<!-- ink:api name="HookFunction" module="status/Status" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `HookFunction`

Intercept and remap status messages or interpolation arguments before they are used by the status system.

### Why this exists

Status messages are often generated deep inside a call stack with no easy way to remap them at the point of origin. `HookFunction` decouples message production from message presentation by letting callers substitute a different string — or the same string — before it reaches the consumer. This makes runtime localization, log filtering, and sensitive-value redaction possible without modifying the status machinery itself.

### Fields

| Signature component | Type | Description |
|---|---|---|
| parameter 1 | `StatusCode` | The status code associated with the current message or argument. |
| parameter 2 | `std::size_t` | Index of the interpolation argument (0-based); 0 when the call carries the primary status message. |
| parameter 3 | `const char*` | The original status message or interpolation argument string. |
| return value | `const char*` | required — the replacement string to use instead of the original. Return the original pointer to pass through unchanged. |

### Construction

```cpp
// Define a hook that inspects every status message
const char* myHook(StatusCode code, std::size_t argIndex, const char* original) {
    // Return the original unchanged, or substitute a localized/redacted string
    return original;
}

// Register the hook before running sc operations
sc::Status::setHook(myHook);
```

### Relationships

- `Status` — registered and retrieved via `Status::setHook()` / `Status::getHook()`
- `StatusCode` — passed as the first argument to every hook invocation

<!-- ink:api-end name="HookFunction" -->

<!-- ink:api name="Status" module="status/Status" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `Status`

Query and intercept operation results for the `sc` namespace without passing error objects through call stacks.

### When to use this

Call `Status::isOk()` after any `sc` operation to check whether it succeeded before using its output. Use `Status::setHook()` when you need to intercept, remap, or log every status message globally — for example, to redirect messages to a localization layer or to redact sensitive content from logs.

### Example

```cpp
#include "status/Status.h"

// Check whether the last sc operation succeeded
if (!sc::Status::isOk()) {
    StatusCode code = sc::Status::get();
    // Inspect code for the specific failure reason
}

// Install a hook to remap all status messages
sc::Status::setHook([](StatusCode code, std::size_t argIndex, const char* msg) -> const char* {
    // Return msg unchanged, or substitute a localized/redacted string
    return msg;
});

// Retrieve the currently installed hook (nullptr if none)
sc::HookFunction currentHook = sc::Status::getHook();
```

### Returns

Each static method returns independently:

- `isOk()` → `bool` — `true` if the current status represents a successful result.
- `get()` → `StatusCode` — the current status code; inspect when `isOk()` returns `false`.
- `getHook()` → `HookFunction` — the currently registered hook, or `nullptr` if none has been set.
- `setHook(hook)` → `void` — replaces the global hook; pass `nullptr` to clear.

### Watch out for

- `Status` holds global (static) state. In multi-threaded code, concurrent calls to `setHook()` and `isOk()` / `get()` are not guarded by the visible interface — establish external synchronization if the hook may be changed after initialization.
- `setHook()` replaces any previously registered hook silently. If multiple subsystems need hook behavior, compose them manually before registering.

<!-- ink:api-end name="Status" -->
