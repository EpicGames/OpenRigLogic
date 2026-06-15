# API Reference — `arrayview`

_13 entries across 2 modules._

## By Task

### View containers

Wrap arrays without copying. Use `ArrayView<T>` for mutable or `ConstArrayView<T>` for read-only access.

| API | Module | Summary |
|-----|--------|---------|
| [ArrayView](ArrayView.md) | ArrayView | Non-owning view over a contiguous sequence without allocation |
| [ConstArrayView](ArrayView.md) | ArrayView | Read-only non-owning view; type alias for `ArrayView<const T>` |
| [StringView](StringView.md) | StringView | Lightweight non-owning string view, similar to `std::string_view` |

### Associated types

Type members and traits for `ArrayView` and string views.

| API | Module | Summary |
|-----|--------|---------|
| [ArrayViewTraits](ArrayView.md) | ArrayView | Traits policy struct mapping element type to associated types |
| [const_pointer](ArrayView.md) | ArrayView | Read-only pointer-to-element type |
| [const_reference](ArrayView.md) | ArrayView | Read-only lvalue-reference type |
| [difference_type](ArrayView.md) | ArrayView | Signed integer type for pointer differences |
| [IsCompatible](ArrayView.md) | ArrayView | Compile-time predicate for cross-type construction |
| [pointer](ArrayView.md) | ArrayView | Pointer-to-element type |
| [reference](ArrayView.md) | ArrayView | Lvalue-reference type |
| [size_type](ArrayView.md) | ArrayView | Unsigned integer type for element counts |
| [Base](StringView.md) | StringView | Base class providing string view semantics |
| [value_type](ArrayView.md) | ArrayView | Underlying element type stripped of decoration |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| ArrayView | [ArrayView.md](ArrayView.md) | 11 |
| StringView | [StringView.md](StringView.md) | 2 |
