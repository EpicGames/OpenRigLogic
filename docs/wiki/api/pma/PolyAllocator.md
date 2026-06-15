# API Reference — `pma/PolyAllocator`

---

<!-- ink:api name="max_align_of" module="pma/PolyAllocator" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `max_align_of<T, U>`

Pick the stricter of two alignment requirements at compile time via `max_align_of<T, U>::type`.

### Why this exists

When an allocator must satisfy the alignment of both the element type `T` and an architectural minimum (captured as `min_align_t`), choosing the larger alignment requires a compile-time branch. `max_align_of` encodes that branch as a metafunction so `PolyAllocator`'s default alignment template argument can be derived automatically without runtime overhead. The result type is whichever of `T` or `U` has the larger `alignof` value.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `T` or `U` | required — the type with the greater alignment; resolved at compile time via `std::conditional` |

<!-- ink:api-end name="max_align_of" -->

<!-- ink:api name="min_align_t" module="pma/PolyAllocator" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `min_align_t`

Internal sentinel type representing the minimum alignment guarantee: the natural alignment of a pointer.

### Why this exists

Allocators that support configurable alignment need a safe lower bound to fall back on when no explicit alignment is requested. Rather than hard-coding `sizeof(void*)` at every call site, `min_align_t` encodes that lower bound as a type so that `max_align_of<T, min_align_t>` can compute the right alignment automatically. This keeps the alignment floor in one place, making it easy to adjust across the entire `pma` allocator layer.

<!-- ink:api-end name="min_align_t" -->

<!-- ink:api name="PolyAllocator" module="pma/PolyAllocator" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<typename T, std::size_t Alignment = alignof(max_align_of<T, min_align_t>::type), class TDefaultMemoryResource = DefaultMemoryResource> class PolyAllocator`

Delegate all memory operations to a swappable `MemoryResource`, letting you control the allocation strategy for any standard container without changing container code.

### When to use this

Reach for `PolyAllocator` when you need a standard-library-compatible allocator whose backing memory resource can be injected at construction time — for example, to point a `std::vector` at a pool allocator or arena during a performance-critical phase. Use the default constructor when a global `DefaultMemoryResource` is sufficient; pass an explicit `MemoryResource*` to override it per-container.

### Example

```cpp
// Point a vector at a custom pool allocator
pma::PolyAllocator<float> poolAlloc{myPoolMemoryResource};
std::vector<float, pma::PolyAllocator<float>> weights(poolAlloc);
weights.resize(1024);  // allocations come from myPoolMemoryResource

// Use default resource (static DefaultMemoryResource)
pma::PolyAllocator<int> defaultAlloc;
std::vector<int, pma::PolyAllocator<int>> indices(defaultAlloc);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `T` | type parameter | required — the element type to allocate |
| `Alignment` | `std::size_t` | optional — allocation alignment; defaults to `alignof(max_align_of<T, min_align_t>::type)` |
| `TDefaultMemoryResource` | type parameter | optional — fallback resource type instantiated as a static local when no resource is supplied; defaults to `DefaultMemoryResource` |
| `memRes` | `MemoryResource*` | optional (constructor) — memory resource to use; if `nullptr`, the default resource is used instead |

### Watch out for

- The default `MemoryResource` is a **static local** inside the default constructor, created once per `<T, Alignment, TDefaultMemoryResource>` instantiation. Its destructor runs at program exit. Clang's `-Wexit-time-destructors` warning is explicitly suppressed here; be aware of destruction ordering if your program relies on destructor sequencing.
- Passing `nullptr` explicitly is equivalent to the default constructor — it does **not** disable allocation; it selects the static default resource.
- The outer `pma::PolyAllocator` inherits from `std::scoped_allocator_adaptor`, meaning nested containers (e.g., `std::vector<std::string>`) automatically propagate the same resource to their inner allocators. If you want independent resources per nesting level, do not use the scoped adaptor directly.

### Constraints

- Thread safety of allocations depends entirely on the supplied `MemoryResource`. The allocator wrapper itself performs no synchronization.

<!-- ink:api-end name="PolyAllocator" -->

<!-- ink:api name="rebind" module="pma/PolyAllocator" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rebind<U>`

Produce a `PolyAllocator` for a different element type `U` while preserving the original alignment and `TDefaultMemoryResource` policy.

### Why this exists

Standard library containers such as `std::list` and `std::map` internally allocate node types that differ from the user-facing element type. The `rebind` mechanism is how the standard mandates that an allocator expose a way to clone itself for these internal types. Without `rebind`, containers that need to allocate auxiliary objects alongside `T` cannot use a typed allocator. `PolyAllocator::rebind<U>::other` satisfies that contract by keeping the same resource and alignment while changing only the element type.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `other` | `PolyAllocator<U, Alignment, TDefaultMemoryResource>` | required — the rebound allocator type for element type `U` |

### Relationships

- `PolyAllocator` — the outer class that declares this struct; `rebind` is always accessed as `PolyAllocator<T,...>::rebind<U>::other`
- `std::allocator_traits` — the standard trait layer that queries `rebind::other` on behalf of containers

<!-- ink:api-end name="rebind" -->

<!-- ink:api name="TDefaultMemoryResource" module="pma/PolyAllocator" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `TDefaultMemoryResource`

Template policy parameter that specifies the concrete `MemoryResource` type instantiated as a static fallback when `PolyAllocator` is default-constructed.

### Why this exists

`PolyAllocator` needs a safe default when no `MemoryResource*` is supplied at construction. Rather than hard-wiring a specific allocator type, the default resource type is a template parameter. This lets library users substitute their own global allocator (e.g., a thread-local pool or a debug-tracking allocator) without modifying `PolyAllocator` itself. The default value is `DefaultMemoryResource`.

### Relationships

- `PolyAllocator` — the template class that uses this parameter as its static fallback resource type
- `MemoryResource` — the base interface that `TDefaultMemoryResource` must satisfy

<!-- ink:api-end name="TDefaultMemoryResource" -->
