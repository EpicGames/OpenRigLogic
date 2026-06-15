# API Reference — `arrayview/ArrayView`

---

<!-- ink:api name="ArrayView" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T> class ArrayView`

Hold a non-owning view over a contiguous sequence of `T` elements — a pointer plus count — without any allocation or deallocation.

### When to use this

Reach for `ArrayView<T>` when you want to pass a slice of an array or a standard container to a function without copying the data and without tying the function to a specific container type. Use the `const`-qualified alias `ConstArrayView<T>` (equivalently `ArrayView<const T>`) when the callee should not modify the elements. Do not use `ArrayView` to store data across asynchronous boundaries or beyond the lifetime of the underlying container — it does not extend object lifetimes.

### Example

```cpp
#include "arrayview/ArrayView.h"
#include <vector>

std::vector<float> weights = {0.1f, 0.4f, 0.5f};

// Construct from a container — no copy
av::ArrayView<float> view{weights};

// Construct from a raw pointer and count
av::ArrayView<float> rawView{weights.data(), weights.size()};

// Slice the last two elements
av::ArrayView<float> tail = view.slice(1);  // {0.4f, 0.5f}

// Read-only view — ConstArrayView is ArrayView<const float>
av::ConstArrayView<float> readOnly{weights};

// By-value comparison with the original container
assert(view == weights);  // true
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `src` | `pointer` + `size_type` | Pointer to the first element and the number of elements in the sequence; required for the raw-pointer constructor. |
| `src` (container overload) | `U&&` | Any lvalue or rvalue container whose `value_type` is compatible and which is not an rvalue reference (lifetime safety guard). |

### Returns

`ArrayView<T>` — a lightweight view (pointer + size) into an existing buffer. `slice()` returns a new `ArrayView<T>` backed by the same memory.

### Constraints

- Does not own memory — the caller is responsible for ensuring the underlying buffer outlives all `ArrayView` instances that reference it.
- The container constructor is disabled for rvalue containers (`!std::is_rvalue_reference<U&&>`) to prevent dangling views over temporaries.
- `ConstArrayView<T>` (`ArrayView<const T>`) is constructible from `ArrayView<T>`, but the reverse is prohibited by `IsCompatible`.

### Watch out for

- Constructing an `ArrayView` from a temporary container (`std::vector<T>{}`) is rejected at compile time via SFINAE, but constructing from a named local variable that goes out of scope before the view is used will compile silently and produce undefined behaviour at runtime.
- `slice(from)` and `slice(from, count)` do not perform bounds checking in the source visible here — verify `from + count <= size()` before calling to avoid out-of-bounds pointer arithmetic.

<!-- ink:api-end name="ArrayView" -->

<!-- ink:api name="ArrayViewTraits" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ArrayViewTraits<T>`

Traits policy struct that maps a raw element type `T` to the standard set of associated types used throughout `ArrayView`.

### Why this exists

`ArrayView` needs a uniform set of member type aliases (`value_type`, `pointer`, `reference`, etc.) that behave correctly for both mutable and const elements — but the rules differ: `ArrayViewTraits<const T>` makes all references and pointers read-only while keeping `value_type` as `const T`. Centralising this in a traits struct separates the policy from `ArrayView`'s logic and allows the const specialisation to be maintained in one place without littering `ArrayView`'s class body with `std::conditional` noise.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` / `const T` | The element type; `const T` in the `const` specialisation. |
| `reference` | `T&` / `const T&` | Lvalue reference to an element. |
| `const_reference` | `const T&` | Const lvalue reference to an element. |
| `pointer` | `T*` / `const T*` | Mutable or read-only pointer to an element. |
| `const_pointer` | `const T*` | Read-only pointer to an element. |
| `size_type` | `std::size_t` | Unsigned type for element counts and indices. |
| `difference_type` | `std::ptrdiff_t` | Signed type for pointer differences and iterator distances. |

### Relationships

- `ArrayView<T>` — consumes all `ArrayViewTraits<T>` members as its own `using` declarations
- `ConstArrayView<T>` — alias for `ArrayView<const T>`, which resolves through `ArrayViewTraits<const T>`

<!-- ink:api-end name="ArrayViewTraits" -->

<!-- ink:api name="const_pointer" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `const_pointer`

Read-only pointer-to-element type, returned by the const overloads of `data()`, `cbegin()`, and `cend()`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `const_pointer` | `const T*` | Always a const pointer regardless of whether `T` is itself const. |

### Relationships

- `ArrayViewTraits<T>` — defines `const_pointer`; `ArrayView<T>` re-exports it for const-qualified access methods

<!-- ink:api-end name="const_pointer" -->

<!-- ink:api name="const_reference" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `const_reference`

Read-only lvalue-reference type for a single element inside an `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `const_reference` | `const T&` | Always a const reference regardless of whether `T` itself is `const`. |

### Relationships

- `ArrayViewTraits<T>` — defines `const_reference`; `ArrayView<T>` re-exports it for the const `operator[]` overload

<!-- ink:api-end name="const_reference" -->

<!-- ink:api name="ConstArrayView" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `ConstArrayView<T>`

Read-only non-owning view over a contiguous sequence of `T` elements; a type alias for `ArrayView<const T>`.

### Why this exists

Providing a named alias for `ArrayView<const T>` gives callers an intent-communicating type at API boundaries: a function accepting `ConstArrayView<float>` signals clearly that it will not modify the elements. It also simplifies call sites — constructing `ConstArrayView<float>` from a `std::vector<float>` works without explicit template arguments because `IsCompatible<const float, float>` is satisfied.

### Relationships

- `ArrayView<T>` — `ConstArrayView<T>` is exactly `ArrayView<const T>`; all `ArrayView` methods are available
- `IsCompatible<const T, T>` — enables implicit construction from a mutable `ArrayView<T>`

<!-- ink:api-end name="ConstArrayView" -->

<!-- ink:api name="difference_type" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `difference_type`

Signed integer type for pointer differences and iterator distance calculations in `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `difference_type` | `std::ptrdiff_t` | Platform-native signed difference type. Satisfies the `std::random_access_iterator` `difference_type` requirement. |

### Relationships

- `ArrayViewTraits<T>` — defines `difference_type`; re-exported by `ArrayView<T>` to satisfy iterator concept requirements

<!-- ink:api-end name="difference_type" -->

<!-- ink:api name="IsCompatible" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `IsCompatible<T, U>`

Compile-time predicate that gates cross-type `ArrayView` construction: true when `U` is the same underlying type as `T` and the const-correctness direction is valid.

### Why this exists

When constructing an `ArrayView<T>` from an `ArrayView<U>`, two conditions must hold: the element types must be the same ignoring cv-qualifiers, and the construction must not silently discard `const` (you can create `ArrayView<const int>` from `ArrayView<int>`, but not the reverse). `IsCompatible` encodes both rules as a single `bool` constant so that `std::enable_if` SFINAE guards on `ArrayView`'s constructors remain readable at a glance.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value` | `static constexpr bool` | `true` when `T` and `U` share the same non-cv type AND the construction does not discard const; `false` otherwise. |

### Relationships

- `ArrayView<T>` — uses `IsCompatible<T, U>` in `std::enable_if` on all cross-type conversion constructors

<!-- ink:api-end name="IsCompatible" -->

<!-- ink:api name="pointer" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pointer`

Pointer-to-element type for an `ArrayView`, used by `data()`, `begin()`, and `end()`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `pointer` | `T*` (primary) / `const T*` (const specialisation) | Mutable pointer in the primary template; read-only pointer when `T` is const-qualified. |

### Relationships

- `ArrayViewTraits<T>` — defines `pointer`; `ArrayView<T>` re-exports it for `data()`, `begin()`, and `end()` return types

<!-- ink:api-end name="pointer" -->

<!-- ink:api name="reference" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `reference`

Lvalue-reference type for a single element inside an `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `reference` | `T&` (primary) / `const T&` (const specialisation) | Mutable reference in the primary template; read-only reference when `T` is const. |

### Relationships

- `ArrayViewTraits<T>` — defines `reference`; `ArrayView<T>` re-exports it via `operator[]` return type

<!-- ink:api-end name="reference" -->

<!-- ink:api name="size_type" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `size_type`

Unsigned integer type used for element counts and index arithmetic in `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `size_type` | `std::size_t` | Platform-native unsigned size type. Matches `size_type` in standard containers so `ArrayView` is range-compatible. |

### Relationships

- `ArrayViewTraits<T>` — defines `size_type`; `ArrayView<T>` uses it for `size()`, `slice()`, and `operator[]` parameters

<!-- ink:api-end name="size_type" -->

<!-- ink:api name="value_type" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `value_type`

The underlying element type of an `ArrayView<T>`, stripped of pointer or reference decoration.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` (primary) / `const T` (const specialisation) | The bare element type. `const T` when `T` is a `const`-qualified type. |

### Relationships

- `ArrayViewTraits<T>` — defines `value_type`; `ArrayView<T>` re-exports it

<!-- ink:api-end name="value_type" -->
