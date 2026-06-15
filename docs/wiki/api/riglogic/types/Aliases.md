# API Reference — `riglogic/types/Aliases`

---

<!-- ink:api name="ArrayView" module="riglogic/types/Aliases" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `template<typename T> using ArrayView = dna::ArrayView<T>`

A non-owning, bounds-aware view over a contiguous sequence of mutable elements — re-exported from `dna` into the `rl4` namespace for use throughout RigLogic.

### Why this exists

Passing raw pointer + size pairs across API boundaries loses the bounds information and cannot be range-iterated safely. `ArrayView<T>` bundles the pointer and element count into a single object that supports iterators, subscript access, and subview slicing without taking ownership or performing any allocation. The `rl4` alias pulls the `dna`-layer type directly into the RigLogic namespace so callers never need to qualify `dna::ArrayView<T>` when working with rig data arrays.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `ptr` | `T*` | required — pointer to the first element of the viewed sequence |
| `sz` | `std::size_t` | required — number of elements in the view |

### Construction

```cpp
// From a raw pointer and count
float weights[4] = {0.25f, 0.5f, 0.75f, 1.0f};
rl4::ArrayView<float> view{weights, 4};

// From any contiguous container (e.g. std::vector)
std::vector<float> blendWeights = {0.1f, 0.9f};
rl4::ArrayView<float> wView{blendWeights};

// Slice a subview
auto partial = view.subview(1, 2);  // elements at index 1 and 2
```

### Relationships

- `ConstArrayView<T>` — the read-only counterpart; prefer when the callee must not modify the data
- `dna::ArrayView<T>` — the underlying type this alias re-exports; identical in behaviour
- `StringView` — a related alias for character-sequence views, also available in `rl4`

### Constraints

- Does not own the pointed-to memory. The viewed array must outlive any `ArrayView` that references it.
- Subscript `operator[]` and `at()` assert bounds in debug builds; no exception is thrown.
- Constructing from an rvalue container is disabled — the source container must be an lvalue to prevent dangling views.

<!-- ink:api-end name="ArrayView" -->

<!-- ink:api name="ConstArrayView" module="riglogic/types/Aliases" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `template<typename T> using ConstArrayView = dna::ConstArrayView<T>`

A non-owning, read-only view over a contiguous sequence of elements — re-exported from `dna` into `rl4` as the immutable counterpart to `ArrayView<T>`.

### Why this exists

API boundaries that hand data to a callee for reading — not modification — should express that intent in the type. `ConstArrayView<T>` is `ArrayView<const T>`, which makes the compiler enforce read-only access while retaining all iteration, indexing, and subview capabilities of the mutable view. Using `ConstArrayView` at getter boundaries prevents accidental mutation of rig data arrays and documents caller intent without requiring a separate class hierarchy.

### Construction

```cpp
// Typical use: returned by a read-only getter, or constructed directly
const std::vector<uint16_t> jointIndices = {0, 3, 7, 12};
rl4::ConstArrayView<uint16_t> view{jointIndices};

// Access elements — read only
for (std::size_t i = 0; i < view.size(); ++i) {
    process(view[i]);  // ok — read
    // view[i] = 0;   // compile error — const element type
}

// Slice a subview of the first 2 indices
auto head = view.first(2);
```

### Relationships

- `ArrayView<T>` — the mutable counterpart; use when the callee needs write access
- `dna::ConstArrayView<T>` — the underlying type this alias re-exports; identical in behaviour
- `StringView` — character-sequence read-only view, also available in `rl4`

### Constraints

- Does not own the pointed-to memory. The viewed array must outlive any `ConstArrayView` that references it.
- Element type is `const T`; assigning through `operator[]` is a compile error.
- Subscript `operator[]` and `at()` assert bounds in debug builds; no exception is thrown.

<!-- ink:api-end name="ConstArrayView" -->
