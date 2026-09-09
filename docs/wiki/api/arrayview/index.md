# API Reference — `arrayview`

_5 entries across 2 modules._

## By Task

### Array and string views

Create and use non-owning views over contiguous sequences of objects and C strings

| API | Module | Summary |
|-----|--------|---------|
| [ArrayView](ArrayView.md) | ArrayView | A non-owning view over a contiguous sequence of objects — a pointer-and-count pair with helper methods, but with no allocation or deallocation of its own. |
| [ConstArrayView](ArrayView.md) | ArrayView | A read-only view over a contiguous sequence of objects — an alias for `ArrayView<const T>`. |
| [StringView](StringView.md) | StringView | A non-owning, NULL-safe view over a C string, built on top of `ConstArrayView<char>`. |

### View type traits

Compile-time type checking and trait utilities for safe view conversions and const-correctness

| API | Module | Summary |
|-----|--------|---------|
| [ArrayViewTraits](ArrayView.md) | ArrayView | A traits type that resolves the value/reference/pointer typedefs `ArrayView<T>` needs, with a specialization for `const T` that keeps all pointer and reference types const-correct. |
| [IsCompatible](ArrayView.md) | ArrayView | A compile-time trait that checks whether an `ArrayView<U>` can be converted to an `ArrayView<T>` — same underlying type, and not converting a const view to a non-const one. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| ArrayView | [ArrayView.md](ArrayView.md) | 4 |
| StringView | [StringView.md](StringView.md) | 1 |
