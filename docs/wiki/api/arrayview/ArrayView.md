# API Reference — `arrayview/ArrayView`

---

<!-- ink:api name="ArrayView" module="arrayview/ArrayView" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T> class ArrayView`

A non-owning view over a contiguous sequence of objects — a pointer-and-count pair with helper methods, but with no allocation or deallocation of its own.

### When to use this

Use `ArrayView` when a function needs to accept "some contiguous sequence of `T`" without caring whether the caller passed a `std::vector`, a raw array, or another `ArrayView`, and without taking ownership of the data. Use `ConstArrayView<T>` (an alias for `ArrayView<const T>`) specifically when the view should be read-only; the templated constructors reject converting a `ConstArrayView` into a mutable `ArrayView` at compile time via `IsCompatible`.

### Method groups

| Group | Methods |
|-------|---------|
| Construction | ArrayView (default, copy, move, pointer+size, converting from ArrayView<U>, converting from container) |
| Size | size |
| Access | data, begin, end, cbegin, cend |

### Example

```cpp
std::vector<float> weights = {0.1f, 0.5f, 0.9f};
av::ArrayView<float> view{weights.data(), weights.size()};
av::ConstArrayView<float> readOnly = view;  // implicit, const-widening conversion
for (float w : readOnly) {
    // iterate without copying the underlying vector
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `src` | `pointer` | required — pointer to the first element of the sequence being viewed |
| `size` | `size_type` | required — number of elements in the sequence |

### Returns

`size_type` (from `size()`) — the number of elements the view covers.

### Watch out for

- `ArrayView` does not own the memory it points to — it performs no allocation or deallocation, so the viewed data must outlive the view.
- Constructing an `ArrayView` from a temporary container (`std::vector<T>{}`) is rejected at compile time via SFINAE, but constructing from a named local variable that goes out of scope before the view is used will compile silently and produce undefined behaviour at runtime.
- `slice(from)` and `slice(from, count)` do not perform bounds checking in the source visible here — verify `from + count <= size()` before calling to avoid out-of-bounds pointer arithmetic.

### Constraints

- The container constructor is disabled for rvalue containers (`!std::is_rvalue_reference<U&&>`) to prevent dangling views over temporaries.
- `ConstArrayView<T>` (`ArrayView<const T>`) is constructible from `ArrayView<T>`, but the reverse is prohibited by `IsCompatible`.

<!-- ink:api-end name="ArrayView" -->

<!-- ink:api name="ArrayViewTraits" module="arrayview/ArrayView" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T> struct ArrayViewTraits`

A traits type that resolves the value/reference/pointer typedefs `ArrayView<T>` needs, with a specialization for `const T` that keeps all pointer and reference types const-correct.

### Why this exists

`ArrayView<T>` must behave correctly for both mutable (`ArrayView<T>`) and immutable (`ArrayView<const T>`) views without duplicating its implementation. `ArrayViewTraits` centralizes the const-correctness decisions (whether `reference`/`pointer` are const) into one specialized traits type that `ArrayView` simply aliases from.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` (or `const T`) | The element type stored/viewed. |
| `reference` | `T&` (or `const T&`) | Reference type returned by mutable accessors. |
| `const_reference` | `const T&` | Reference type returned by const accessors. |
| `pointer` | `T*` (or `const T*`) | Pointer type used for mutable data access. |
| `const_pointer` | `const T*` | Pointer type used for const data access. |
| `size_type` | `std::size_t` | Type used for element counts. |
| `difference_type` | `std::ptrdiff_t` | Type used for iterator differences. |

### Relationships

- `ArrayView` — *uses this traits type to derive all of its member typedefs*

<!-- ink:api-end name="ArrayViewTraits" -->


<!-- ink:api name="ConstArrayView" module="arrayview/ArrayView" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ConstArrayView`

A read-only view over a contiguous sequence of objects — an alias for `ArrayView<const T>`.

### Why this exists

`ConstArrayView<T>` exists so callers can express "a read-only view of `T` elements" directly rather than writing `ArrayView<const T>`, while still getting the same const-safety guarantees enforced by `ArrayView`'s `IsCompatible` trait (a `ConstArrayView` cannot be implicitly converted back into a mutable `ArrayView`).

### Relationships

- `ArrayView` — *`ConstArrayView<T>` is simply `ArrayView<const T>`*
- `StringView` — *inherits from `ConstArrayView<char>` to add string-specific accessors*

<!-- ink:api-end name="ConstArrayView" -->


<!-- ink:api name="IsCompatible" module="arrayview/ArrayView" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename U> struct IsCompatible`

A compile-time trait that checks whether an `ArrayView<U>` can be converted to an `ArrayView<T>` — same underlying type, and not converting a const view to a non-const one.

### Why this exists

`ArrayView`'s converting constructors (from `ArrayView<U>&`, `const ArrayView<U>&`, `ArrayView<U>&&`, and generic containers) are enabled via `enable_if<IsCompatible<T, U>::value, ...>`. Without this trait, the templated constructors would silently accept incompatible element types or let a `ConstArrayView` be converted into a mutable `ArrayView`, breaking const-safety at compile time rather than being rejected.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value` | `bool` (`constexpr`) | `true` when `T` and `U` have the same type ignoring cv-qualifiers, and the conversion does not strip constness. |

<!-- ink:api-end name="IsCompatible" -->
