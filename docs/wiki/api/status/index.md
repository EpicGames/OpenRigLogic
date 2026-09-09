# API Reference — `status`

_8 entries across 3 modules._

## By Task

### Status tracking and reporting

Read status outcomes, install message interception hooks, and format status messages with variadic arguments

| API | Module | Summary |
|-----|--------|---------|
| [HookFunction](Status.md) | Status | A function pointer type for intercepting and overriding status messages and their interpolation arguments before they are used. |
| [Status](Status.md) | Status | Provides read access to the current status and lets callers install a hook to intercept status messages and arguments. |
| [StatusCode](StatusCode.md) | StatusCode | Represents the outcome of an operation as a numeric code paired with a human-readable message. |
| [StatusProvider](Provider.md) | Provider | Formats and records status messages with `printf`-style interpolation, running each argument through an optional hook before formatting. |

### Compile-time index sequences

Template metaprogramming utilities for building and working with compile-time integer index sequences

| API | Module | Summary |
|-----|--------|---------|
| [ArgIndices](Provider.md) | Provider | A local alias, inside `StatusProvider::set`, for the `MakeISeq` sequence covering the variadic arguments' positions. |
| [ISeq](Provider.md) | Provider | A compile-time sequence of `std::size_t` indices, used internally to unpack variadic argument positions. |
| [Make](Provider.md) | Provider | A compile-time recursive template that builds an `ISeq` of a given size, starting at a given offset. |
| [MakeISeq](Provider.md) | Provider | A convenience alias for `Make<Size, Offset>::Type`, producing an `ISeq` of `Size` indices starting at `Offset`. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| Provider | [Provider.md](Provider.md) | 5 |
| Status | [Status.md](Status.md) | 2 |
| StatusCode | [StatusCode.md](StatusCode.md) | 1 |
