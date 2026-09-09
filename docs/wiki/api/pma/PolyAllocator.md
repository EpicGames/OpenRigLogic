# API Reference — `pma/PolyAllocator`

---

<!-- ink:api name="max_align_of" module="pma/PolyAllocator" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::max_align_of<T, U>`

A compile-time trait that resolves to whichever of two types, `T` or `U`, has the greater alignment.

### Why this exists

`PolyAllocator`'s default alignment must be at least as strict as both the element type's natural alignment and a baseline minimum (`min_align_t`); a raw `alignof` comparison on just one type could under-align the allocator's storage. This trait picks the correct type to take `alignof` of without requiring the caller to reason about the comparison themselves.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `typename std::conditional<(alignof(T) > alignof(U)), T, U>::type` | The type among `T`/`U` with the larger alignment. |

### Relationships

- `PolyAllocator` — *default `Alignment` parameter is `alignof(max_align_of<T, min_align_t>::type)`.*

<!-- ink:api-end name="max_align_of" -->

<!-- ink:api name="min_align_t" module="pma/PolyAllocator" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `impl::min_align_t`

An internal alias (`void*`) used as the minimal-alignment reference type when computing a `PolyAllocator`'s default alignment.

### Why this exists

`PolyAllocator`'s default `Alignment` template parameter needs a baseline alignment to compare a type's own alignment against, and `void*` is a convenient stand-in for "pointer-sized minimum alignment" without depending on a specific concrete type. It exists purely to feed `max_align_of` inside the `impl` namespace.

### Relationships

- `max_align_of` — *compares a type's alignment against `min_align_t` to compute the larger of the two.*
- `PolyAllocator` — *uses `max_align_of<T, min_align_t>` as its default alignment.*

<!-- ink:api-end name="min_align_t" -->

<!-- ink:api name="PolyAllocator" module="pma/PolyAllocator" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<typename T, std::size_t Alignment, class TDefaultMemoryResource> class PolyAllocator : public std::scoped_allocator_adaptor<...>`

A standard-library-compatible allocator that forwards allocation and construction to a runtime-supplied `MemoryResource`, letting containers use arbitrary memory strategies without being templated on the allocator's concrete type.

### When to use this

Use this to plug a custom `MemoryResource` (arena, pool, tracking allocator) into `std::vector`, `std::map`, or any other allocator-aware standard container. Construct it with a `MemoryResource*`; passing `nullptr` falls back to a default (`TDefaultMemoryResource`) static instance.

### Method groups

| Group | Methods |
|-------|---------|
| Construction | PolyAllocator() (default and from `MemoryResource*` / `nullptr` / rebind-compatible allocator) |
| Raw allocation | allocateBytes, deallocateBytes |
| Object allocation | allocateObject, deallocateObject, newObject, deleteObject |
| STL allocator interface | allocate (and the deallocate/rebind machinery inherited from `scoped_allocator_adaptor`) |

### Example

```cpp
pma::MemoryResource* memRes = getArenaMemoryResource();
pma::PolyAllocator<dna::Vector3> alloc{memRes};
std::vector<dna::Vector3, pma::PolyAllocator<dna::Vector3>> joints{alloc};
joints.push_back({1.0f, 0.0f, 0.0f});
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `memRes` | `MemoryResource*` | optional — the backing memory resource; if `nullptr`, falls back to a static `TDefaultMemoryResource` instance. |
| `count` | `std::size_t` | required (for `allocateObject`/`deallocateObject`) — number of objects to allocate/deallocate. |
| `alignment` | `std::size_t` | optional — defaults to the allocator's `Alignment` template parameter. |

### Watch out for

- Two `PolyAllocator` instances compare equal only if both `Alignment` matches and their underlying `MemoryResource` pointers match — allocators from different memory resources are never interchangeable.
- The default-constructed default memory resource is a function-local `static`, guarded against clang's exit-time-destructor warning — its lifetime spans the whole program.
- The default `MemoryResource` is a **static local** inside the default constructor, created once per `<T, Alignment, TDefaultMemoryResource>` instantiation. Its destructor runs at program exit. Clang's `-Wexit-time-destructors` warning is explicitly suppressed here; be aware of destruction ordering if your program relies on destructor sequencing.
- Passing `nullptr` explicitly is equivalent to the default constructor — it does **not** disable allocation; it selects the static default resource.
- The outer `pma::PolyAllocator` inherits from `std::scoped_allocator_adaptor`, meaning nested containers (e.g., `std::vector<std::string>`) automatically propagate the same resource to their inner allocators. If you want independent resources per nesting level, do not use the scoped adaptor directly.

### Constraints

- Thread safety of allocations depends entirely on the supplied `MemoryResource`. The allocator wrapper itself performs no synchronization.

<!-- ink:api-end name="PolyAllocator" -->


<!-- ink:api name="TDefaultMemoryResource" module="pma/PolyAllocator" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `TDefaultMemoryResource` (template parameter)

The fallback `MemoryResource` type a `PolyAllocator` uses when constructed without an explicit memory resource (or with `nullptr`).

### Why this exists

Requiring every `PolyAllocator` use site to pass a `MemoryResource*` would make simple cases (no custom allocator needed) verbose; this template parameter lets `PolyAllocator` default to `DefaultMemoryResource` while still allowing callers to override it for specialized allocation strategies.

### Relationships

- `PolyAllocator` — *default-constructs a static instance of this type when no explicit `MemoryResource*` is supplied.*
- `MemoryResource` — *`TDefaultMemoryResource` must be a concrete subclass of this abstract base.*

<!-- ink:api-end name="TDefaultMemoryResource" -->

<!-- ink:api name="UDefaultMemoryResource" module="pma/PolyAllocator" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `UDefaultMemoryResource` (template parameter)

The corresponding default-memory-resource template parameter for the "other" `PolyAllocator` type in cross-allocator comparisons (`operator==`/`operator!=`).

### Why this exists

`PolyAllocator` equality/inequality comparisons need to compare two possibly-differently-templated `PolyAllocator` instances; `UDefaultMemoryResource` names the second instance's default-resource type parameter symmetrically to `TDefaultMemoryResource` on the first, even though it plays no role in the equality check itself (only alignment and the resolved `MemoryResource*` do).

### Relationships

- `PolyAllocator::operator==` / `operator!=` — *free functions comparing a `PolyAllocator<T, TAlignment, TDefaultMemoryResource>` against a `PolyAllocator<U, UAlignment, UDefaultMemoryResource>`.*

<!-- ink:api-end name="UDefaultMemoryResource" -->

