# API Reference — `status/StatusCode`

---

<!-- ink:api name="StatusCode" module="status/StatusCode" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `StatusCode`

Represents the outcome of an operation as a numeric code paired with a human-readable message.

### Why this exists

`StatusCode` exists so that operations across module boundaries (including C API boundaries where exceptions cannot cross) can report success or failure without relying on exceptions. Pairing an integer `code` with a `message` lets callers branch on the code while still surfacing a readable description for logging or diagnostics.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `code` | `int` | required — numeric status/error code; `0` conventionally indicates success, non-zero indicates a specific failure condition |
| `message` | `const char*` | required — human-readable description of the status, typically for logging or diagnostics |

### Relationships

- `operator==` / `operator!=` — compare two `StatusCode` values by their `code` field only (message is ignored for equality)

### Constraints

- Equality is code-only. Two `StatusCode` values with the same `code` but different `message` strings are considered equal by `operator==`. Do not use `message` content as an identity signal.

### Watch out for

- `message` is a raw `const char*` with no ownership. Pointing it at a temporary `std::string::c_str()` or a local buffer produces a dangling pointer. Always use a string literal or a heap allocation whose lifetime exceeds the `StatusCode`.

<!-- ink:api-end name="StatusCode" -->
