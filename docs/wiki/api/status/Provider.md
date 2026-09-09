# API Reference — `status/Provider`

---

<!-- ink:api name="ArgIndices" module="status/Provider" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ArgIndices`

A local alias, inside `StatusProvider::set`, for the `MakeISeq` sequence covering the variadic arguments' positions.

### Why this exists

`ArgIndices` names the specific index sequence — `impl::MakeISeq<sizeof...(Args), 1>` — used to number each variadic argument to `set` starting at position 1, so it can be passed into the `ISeq`-based overload of `set` in one readable expression.

### Relationships

- `MakeISeq` — *the alias this local type resolves through.*
- `StatusProvider` — *the class whose `set` method defines and uses this alias.*

<!-- ink:api-end name="ArgIndices" -->

<!-- ink:api name="ISeq" module="status/Provider" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::ISeq`

A compile-time sequence of `std::size_t` indices, used internally to unpack variadic argument positions.

### Why this exists

`StatusProvider::set` needs to pass each variadic argument through a per-argument hook (`execHook`) along with its 1-based position. `ISeq` (built by `Make`/`MakeISeq`) generates that index sequence at compile time so the position can be threaded through without runtime overhead or manual unpacking.

### Relationships

- `Make` / `MakeISeq` — *the metafunctions that construct an `ISeq` of a given size and offset.*
- `StatusProvider` — *the class whose variadic `set` overload consumes an `ISeq`.*

<!-- ink:api-end name="ISeq" -->

<!-- ink:api name="Make" module="status/Provider" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::Make`

A compile-time recursive template that builds an `ISeq` of a given size, starting at a given offset.

### Why this exists

Generating an index sequence like `{1, 2, 3}` for a fixed argument count needs to happen without runtime cost. `Make` recursively peels one index off the requested size until it reaches the `Make<0, Offset, Is...>` base case, which captures the accumulated indices as an `ISeq`.

### Relationships

- `ISeq` — *the type ultimately produced by `Make`'s recursion.*
- `MakeISeq` — *the convenience alias that exposes `Make<Size, Offset>::Type` directly.*

<!-- ink:api-end name="Make" -->

<!-- ink:api name="MakeISeq" module="status/Provider" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::MakeISeq`

A convenience alias for `Make<Size, Offset>::Type`, producing an `ISeq` of `Size` indices starting at `Offset`.

### Why this exists

Using `Make<Size, Offset>::Type` directly at every call site is verbose. `MakeISeq` hides the recursive `Make` machinery behind a single alias, and defaults `Offset` to `0` for the common case.

### Relationships

- `Make` — *the recursive template this alias resolves through.*
- `StatusProvider` — *uses `impl::MakeISeq<sizeof...(Args), 1>` to number variadic arguments starting at 1.*

<!-- ink:api-end name="MakeISeq" -->

<!-- ink:api name="StatusProvider" module="status/Provider" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class SCAPI StatusProvider`

Formats and records status messages with `printf`-style interpolation, running each argument through an optional hook before formatting.

### When to use this

Use `StatusProvider::set` when raising a status/error that needs formatted, interpolated arguments (e.g. a file path or resource name) instead of a fixed message string. Use `StatusProvider::get`/`isOk` to check the current status after calling code that reports through this mechanism.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | StatusProvider (constructor) |
| Query | reset, get, isOk |
| Reporting | set |

### Example

```cpp
sc::StatusProvider provider{StatusCode{1, "Failed to open file: %s"}};
sc::StatusProvider::set(StatusCode{1, "Failed to open file: %s"}, "rig.dna");
if (!sc::StatusProvider::isOk()) {
    // handle the formatted status message
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `statuses` | `std::initializer_list<StatusCode>` | The set of status codes this provider is responsible for registering. |
| `status` | `StatusCode` | required (on `set`). The status code/message template to report. |
| `args` | `Args&&...` | optional (on `set`). Values interpolated into the status message via `snprintf`, each passed through `execHook` first. |

### Watch out for

- The variadic `set(StatusCode, Args&&...)` overload builds an `ISeq` starting at index 1 (not 0) — index 0 is reserved for hooking the message template itself.
- Formatted messages are truncated to a fixed 512-byte buffer (`bufferSize`); very long interpolated arguments can be cut off.
- `-Wformat-security` is deliberately suppressed around the `snprintf` calls because the format string comes from `status.message`, not a literal — callers must ensure `status.message`'s format specifiers match the arguments passed.
- The formatted message buffer is fixed at 512 bytes. Strings longer than 511 characters are silently truncated by `snprintf`.
- `execHook` is called once for the message string itself (index 0) and once per `const char*` argument (indices 1…N). Hook return values override the originals. Arguments of any other type bypass the hook entirely.
- `reset()`, `get()`, `isOk()`, and `set()` are all static — they operate on shared (per-TU or global) state. Concurrent calls without external synchronisation are unsafe.

<!-- ink:api-end name="StatusProvider" -->
