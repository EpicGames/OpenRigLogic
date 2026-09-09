# API Reference — `status/Status`

---

<!-- ink:api name="HookFunction" module="status/Status" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `HookFunction`

A function pointer type for intercepting and overriding status messages and their interpolation arguments before they are used.

### Why this exists

Callers may want to customize how status messages or their `const char*` arguments are rendered (for example, translating messages or sanitizing arguments) without modifying `StatusProvider`/`Status` themselves. `HookFunction` defines the fixed signature — `(StatusCode, std::size_t, const char*) -> const char*` — that any such override must implement, where the `std::size_t` index distinguishes the message itself (index 0) from each interpolation argument (index 1+).

### Relationships

- `Status` — *exposes `getHook`/`setHook` to install a `HookFunction`.*
- `StatusProvider` — *invokes the installed hook via `execHook` while formatting status messages.*

<!-- ink:api-end name="HookFunction" -->

<!-- ink:api name="Status" module="status/Status" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class SCAPI Status`

Provides read access to the current status and lets callers install a hook to intercept status messages and arguments.

### When to use this

Call `Status::isOk()` or `Status::get()` after invoking code that reports through `StatusProvider` to check whether an operation succeeded. Use `Status::setHook` when you need to intercept or transform status messages/arguments globally — for example, to redact sensitive values before they are formatted.

### Method groups

| Group | Methods |
|-------|---------|
| Query | isOk, get |
| Hook | getHook, setHook |

### Example

```cpp
if (!sc::Status::isOk()) {
    sc::StatusCode code = sc::Status::get();
    // inspect code.code / code.message
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `hook` | `HookFunction` | required (on `setHook`). The function to invoke for each status message and interpolation argument. |

### Returns

`bool` (from `isOk`) — whether the current status represents success. `StatusCode` (from `get`) — the current status code and message. `HookFunction` (from `getHook`) — the currently installed hook, if any.

### Relationships

- `HookFunction` — *the callback type installed and retrieved via `setHook`/`getHook`.*
- `StatusProvider` — *the counterpart class that reports the statuses `Status` reads.*

### Watch out for

- `Status` holds global (static) state. In multi-threaded code, concurrent calls to `setHook()` and `isOk()` / `get()` are not guarded by the visible interface — establish external synchronization if the hook may be changed after initialization.
- `setHook()` replaces any previously registered hook silently. If multiple subsystems need hook behavior, compose them manually before registering.

<!-- ink:api-end name="Status" -->
