# API Reference — `riglogic/types/Aliases`

---

<!-- ink:api name="ArrayView" module="riglogic/types/Aliases" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ArrayView`

An alias for `dna::ArrayView<T>`, a lightweight mutable view over a contiguous array of `T`.

### Why this exists

RigLogic exposes many buffers of typed data (control values, joint outputs, etc.) without copying them. Rather than defining its own view type, `rl4::ArrayView` reuses `dna::ArrayView` so RigLogic and DNA APIs share the same view semantics and can be passed interchangeably.

### Relationships

- `ConstArrayView` — *the read-only counterpart of this alias.*

### Constraints

- Does not own the pointed-to memory. The viewed array must outlive any `ArrayView` that references it.
- Subscript `operator[]` and `at()` assert bounds in debug builds; no exception is thrown.
- Constructing from an rvalue container is disabled — the source container must be an lvalue to prevent dangling views.

<!-- ink:api-end name="ArrayView" -->

<!-- ink:api name="ConstArrayView" module="riglogic/types/Aliases" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `ConstArrayView`

An alias for `dna::ConstArrayView<T>`, a lightweight read-only view over a contiguous array of `T`.

### Why this exists

Many RigLogic accessors (e.g. `getJointOutputs`, `getGUIControlValues`) return read-only access to internal buffers. `ConstArrayView` gives callers that access without a copy and without the ability to mutate RigLogic's internal state.

### Relationships

- `ArrayView` — *the mutable counterpart of this alias.*

### Constraints

- Does not own the pointed-to memory. The viewed array must outlive any `ConstArrayView` that references it.
- Element type is `const T`; assigning through `operator[]` is a compile error.
- Subscript `operator[]` and `at()` assert bounds in debug builds; no exception is thrown.

<!-- ink:api-end name="ConstArrayView" -->
