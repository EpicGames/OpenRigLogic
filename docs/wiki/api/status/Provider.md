# API Reference — `status/Provider`

---

<!-- ink:api name="ArgIndices" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ArgIndices`

Local 1-based compile-time index sequence used inside `StatusProvider::set` to map each variadic argument to its hook interception position.

### Why this exists

`ArgIndices` is `MakeISeq<sizeof...(Args), 1>` — a sequence starting at 1 rather than 0. The offset is intentional: index 0 is reserved for the status message string itself in `execHook`, and indices 1…N map to each positional `const char*` argument in the variadic pack. Encoding this offset in the type alias keeps the intent explicit at the `set` call site and avoids an off-by-one error if the sequence were ever generated from zero.

### Relationships

- `MakeISeq` — the alias template `ArgIndices` is instantiated from
- `ISeq` — the concrete type `ArgIndices` resolves to
- `StatusProvider::set` — the only consumer of `ArgIndices`
- `execHook` — receives each index from the `ArgIndices` pack during argument interception

<!-- ink:api-end name="ArgIndices" -->

<!-- ink:api name="ISeq" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ISeq<Is...>`

Compile-time integer sequence holder used to pass index packs through template parameter expansion.

### Why this exists

`ISeq` encodes a list of compile-time indices directly into the type system so they can be unpacked via variadic template expansion at the call site of `StatusProvider::set`. A runtime array or a single integer cannot fill this role because the unpack must occur in the template parameter list at compile time. The `size()` method exposes the cardinality of the sequence without any runtime overhead.

### Construction

```cpp
// Typically produced via MakeISeq rather than written by hand
using Indices = impl::MakeISeq<3, 1>;  // produces ISeq<1, 2, 3>
constexpr std::size_t n = Indices::size();  // 3
```

### Relationships

- `Make` — recursive metafunction that constructs `ISeq` values
- `MakeISeq` — convenience alias that wraps `Make<Size, Offset>::Type`
- `StatusProvider::set` — consumes an `ISeq` to dispatch hook indices

<!-- ink:api-end name="ISeq" -->

<!-- ink:api name="Make" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Make<Size, Offset, Is...>`

Recursive template metafunction that assembles a contiguous integer range as a compile-time `ISeq` type.

### Why this exists

`Make` solves the problem of constructing an `ISeq<Offset, Offset+1, ..., Offset+Size-1>` without requiring the caller to enumerate every index manually. Each recursive step prepends one index until `Size` reaches zero, at which point the base specialisation exposes the fully assembled `ISeq` as its nested `Type`. Access `Make` through the `MakeISeq` alias rather than directly — `MakeISeq<N, Offset>` is the intended call site.

### Construction

```cpp
// Access the result through the nested Type alias
using Seq = impl::Make<4, 0>::Type;  // ISeq<0, 1, 2, 3>
// Prefer MakeISeq for cleaner syntax
using Seq2 = impl::MakeISeq<4, 0>;  // equivalent
```

### Relationships

- `ISeq` — the result type produced by `Make::Type`
- `MakeISeq` — type alias that unwraps `Make<Size, Offset>::Type`

<!-- ink:api-end name="Make" -->

<!-- ink:api name="MakeISeq" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `MakeISeq<Size, Offset>`

Convenience type alias that produces an `ISeq` spanning `[Offset, Offset+Size)` without exposing the `Make::Type` indirection.

### Why this exists

Without `MakeISeq`, every call site would need to write `typename impl::Make<N, O>::Type`, which is verbose and forces knowledge of `Make`'s internal structure. `MakeISeq` wraps that in a single alias, and the default `Offset = 0` further reduces boilerplate for the common zero-based case. Inside `StatusProvider::set`, `ArgIndices` is defined as `MakeISeq<sizeof...(Args), 1>` — the 1-based variant that reserves index 0 for the message string.

### Construction

```cpp
// Zero-based sequence of length 3: ISeq<0, 1, 2>
using ZeroBased = impl::MakeISeq<3>;

// 1-based sequence for argument hook indices: ISeq<1, 2, 3>
using ArgIdx = impl::MakeISeq<3, static_cast<std::size_t>(1)>;
```

### Relationships

- `Make` — the metafunction `MakeISeq` wraps
- `ISeq` — the type `MakeISeq` resolves to
- `ArgIndices` — a specific instantiation used inside `StatusProvider::set`

<!-- ink:api-end name="MakeISeq" -->

<!-- ink:api name="StatusProvider" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `StatusProvider`

Record, query, and hook a propagating status code with optional formatted message across call boundaries.

### When to use this

Use `StatusProvider` when you need a lightweight, call-depth-independent way to propagate a status code from a deep operation back to a top-level caller — without threading a result object through every intermediate frame. The static `get`/`isOk` interface is particularly useful in C-style callback chains or plugin boundaries where return types are fixed.

### Example

```cpp
// Register the status codes this provider handles for this TU
StatusProvider provider{StatusCode::Ok, StatusCode::LoadFailed};

// Simple status set (no format args)
StatusProvider::set(StatusCode::Ok);

// Formatted status with a file path argument
StatusProvider::set(StatusCode::LoadFailed, "/data/rigs/alice.dna");

// Check at a higher call frame
if (!StatusProvider::isOk()) {
    StatusCode sc = StatusProvider::get();
    // sc.message is snprintf-formatted, max 511 chars
    fprintf(stderr, "Load error: %s\n", sc.message);
}

StatusProvider::reset();  // clear before next operation
```

### Parameters

| Name | Type | Description |
|---|---|---|
| `statuses` | `std::initializer_list<StatusCode>` | required — status codes registered with this provider instance |
| `status` | `StatusCode` | required — the status code to record; its `message` field is used as a `printf`-style format string when variadic args are provided |
| `args...` | `Args&&...` | optional — `printf`-style format arguments; `const char*` args are subject to `execHook` interception; all other types pass through unchanged |

### Returns

- `get()` → `StatusCode` — the most recently set status value
- `isOk()` → `bool` — `true` when no error status has been recorded since the last `reset()`

### Watch out for

- The formatted message buffer is fixed at 512 bytes. Strings longer than 511 characters are silently truncated by `snprintf`.
- `execHook` is called once for the message string itself (index 0) and once per `const char*` argument (indices 1…N). Hook return values override the originals. Arguments of any other type bypass the hook entirely.
- `reset()`, `get()`, `isOk()`, and `set()` are all static — they operate on shared (per-TU or global) state. Concurrent calls without external synchronisation are unsafe.

<!-- ink:api-end name="StatusProvider" -->

<!-- ink:api name="Type" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Make<0, Offset, Is...>::Type`

Nested type alias that exposes the fully assembled `ISeq` when `Make`'s recursion terminates.

### Why this exists

C++ template metaprogramming requires a metafunction to expose its result through a nested type alias so callers can retrieve it with `typename Make<N, O>::Type`. `Type` is the terminus of `Make`'s recursion: when `Size` reaches zero, the accumulated pack `Is...` is exactly `[Offset, Offset+Size)`, and `Type` names the resulting `ISeq`. The `MakeISeq` alias exists specifically to hide this `::Type` access from call sites.

### Relationships

- `Make` — the metafunction that defines this alias in its base case
- `ISeq` — the concrete type this alias resolves to
- `MakeISeq` — `using MakeISeq = typename Make<Size, Offset>::Type` — the preferred accessor

<!-- ink:api-end name="Type" -->
