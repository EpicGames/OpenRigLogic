# API Reference — `status/StatusCode`

---

<!-- ink:api name="StatusCode" module="status/StatusCode" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `StatusCode`

Pair a numeric status code with a human-readable message for results that need both machine-checkable identity and a displayable reason.

### Why this exists

Raw integer codes force callers to do a separate lookup to get a diagnostic string, while raw strings make programmatic comparison fragile. `StatusCode` solves both: equality is always code-based, so dispatch and matching stay numeric, while `message` is available directly for logging or display. The `SCAPI` export macro signals that this type crosses a shared-library boundary, making the plain-struct layout intentional.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `code` | `int` | required — numeric identifier for the status condition. All equality and comparison operations are based solely on this field. |
| `message` | `const char*` | required — human-readable description of the status. Does not own the pointed-to string; must point to a stable allocation or string literal. |

### Construction

```cpp
// Aggregate-initialise with a stable string literal
sc::StatusCode ok   = {0,    "OK"};
sc::StatusCode fail = {1001, "Joint count mismatch"};

// Compare by code
if (result != ok) {
    fprintf(stderr, "Error %d: %s\n", result.code, result.message);
}
```

### Relationships

- `operator==` — free function in `namespace sc`; returns `true` when both operands share the same `code`, regardless of `message`.
- `operator!=` — free function in `namespace sc`; logical negation of `operator==`.

### Constraints

- Equality is code-only. Two `StatusCode` values with the same `code` but different `message` strings are considered equal by `operator==`. Do not use `message` content as an identity signal.

### Watch out for

- `message` is a raw `const char*` with no ownership. Pointing it at a temporary `std::string::c_str()` or a local buffer produces a dangling pointer. Always use a string literal or a heap allocation whose lifetime exceeds the `StatusCode`.

<!-- ink:api-end name="StatusCode" -->
