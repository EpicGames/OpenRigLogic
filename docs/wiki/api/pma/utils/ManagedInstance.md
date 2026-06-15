# API Reference — `pma/utils/ManagedInstance`

---

<!-- ink:api name="ManagedInstance" module="pma/utils/ManagedInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `template<class TPointer, class TTarget, class TBase = TTarget> class ManagedInstance`

Allocate a heap object through a custom `MemoryResource` and bind its destructor to that same resource, returning a smart pointer that owns the lifetime. Prefer the `UniqueInstance` or `SharedInstance` aliases over instantiating this template directly.

### When to use this

Reach for `ManagedInstance` (via its aliases) when an object must be allocated and freed through the same `MemoryResource` that owns the memory budget — for example, when a DNA reader and all its internal structures must share a single arena. Use `UniqueInstance` when only one owner is needed; use `SharedInstance` when the object is shared across subsystems with no clear single owner.

### Example

```cpp
// Allocate a DNAReader through a custom memory resource
pma::MemoryResource* memRes = pma::makeDefaultMemoryResource();
auto reader = pma::UniqueInstance<DNAReader>::with(memRes).create(stream, dataLayer);
// reader is a unique_ptr; its deleter calls memRes->deleteObject on destruction
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `memRes` | `MemoryResource*` | required — the memory resource that will allocate and later destroy the object. Must outlive all smart pointers produced by `create()`. |
| `args...` | `Args&&...` | required — constructor arguments forwarded to `TTarget`'s constructor via `newObject`. |

### Returns

`PointerType` — a smart pointer (`unique_ptr` or `shared_ptr` depending on alias) owning the newly allocated `TTarget`, cast to `TBase*`. The pointer's deleter routes back through `PolyAllocator::deleteObject` on the originating `MemoryResource`.

### Watch out for

- The deleter lambda captures `PolyAllocator<TTarget>` by value. The underlying `MemoryResource*` is stored inside that allocator copy. If the `MemoryResource` is destroyed before the last smart pointer goes out of scope, the deleter accesses a dangling pointer and produces undefined behavior.
- `ManagedInstance` lives in `namespace pma::impl` — use the `UniqueInstance` and `SharedInstance` public aliases rather than naming this template directly. The template parameters are order-sensitive and error-prone to spell manually.

<!-- ink:api-end name="ManagedInstance" -->

<!-- ink:api name="MemoryResource" module="pma/utils/ManagedInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `MemoryResource`

Abstract memory resource interface that serves as the allocator policy for `ManagedInstance`. Pass a pointer to this type when constructing `UniqueInstance` or `SharedInstance` via `with()`.

### Why this exists

`MemoryResource` decouples object construction from the global heap by providing a pluggable allocation policy. Any subsystem that controls its own memory budget can supply a custom `MemoryResource` implementation, and all objects created through `ManagedInstance::create()` will be allocated and freed through that resource. This prevents silent heap fragmentation when many short-lived DNA objects are created and destroyed at high frequency.

### Relationships

- `ManagedInstance` — consumes a `MemoryResource*` to allocate `TTarget` objects
- `UniqueInstance` — convenience alias; passes `MemoryResource*` through `with()`
- `SharedInstance` — convenience alias; passes `MemoryResource*` through `with()`

<!-- ink:api-end name="MemoryResource" -->

<!-- ink:api name="PointerType" module="pma/utils/ManagedInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `PointerType`

Member type alias inside `ManagedInstance` that resolves to the smart pointer type `TPointer`. Use it to name the return type of `create()` without repeating the full template instantiation.

### Why this exists

The concrete `TPointer` type (either `unique_ptr<TBase, …>` or `shared_ptr<TBase>`) is long and template-specific. Exposing `PointerType` lets call sites write `ManagedInstance<…>::PointerType` or simply rely on auto, without coupling themselves to the pointer kind selected by the alias. This also allows `UniqueInstance` and `SharedInstance` to unify their public interface under the same member name.

### Relationships

- `ManagedInstance::create()` — returns `PointerType`
- `UniqueInstance` — resolves `PointerType` to `unique_ptr<TBase, function<void(TBase*)>>`
- `SharedInstance` — resolves `PointerType` to `shared_ptr<TBase>`

<!-- ink:api-end name="PointerType" -->

<!-- ink:api name="SharedInstance" module="pma/utils/ManagedInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `SharedInstance<TTarget, TBase>`

Convenience alias for `ManagedInstance` that produces a `std::shared_ptr<TBase>` with a custom allocator-backed deleter, enabling shared ownership of an object allocated through a `MemoryResource`.

### Why this exists

When a DNA object must be referenced by multiple subsystems with no clear single owner, a `unique_ptr` is insufficient. `SharedInstance` wires `ManagedInstance` to produce a `shared_ptr` so the reference count governs lifetime, while still routing deallocation through the originating `MemoryResource` — ensuring that memory is returned to the correct budget rather than to the global heap.

### Construction

```cpp
pma::MemoryResource* memRes = pma::makeDefaultMemoryResource();
// Create a RigInstance shared between the renderer and the physics system
auto rig = pma::SharedInstance<RigInstance>::with(memRes).create(rigConfig);
// rig type: std::shared_ptr<RigInstance>
auto rigCopy = rig; // reference count incremented; same allocator deleter
```

### Relationships

- `ManagedInstance` — the underlying template this alias configures
- `UniqueInstance` — use instead when exclusive ownership is sufficient
- `MemoryResource` — the allocator policy supplied via `with()`

### Constraints

- The `MemoryResource*` supplied to `with()` must outlive all copies of the returned `shared_ptr`. The deleter embedded in the control block references that pointer at destruction time.

<!-- ink:api-end name="SharedInstance" -->

<!-- ink:api name="UniqueInstance" module="pma/utils/ManagedInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `UniqueInstance<TTarget, TBase>`

Convenience alias for `ManagedInstance` that produces a `std::unique_ptr<TBase, std::function<void(TBase*)>>` with a custom deleter that frees the object through the originating `MemoryResource`. Use this when exactly one subsystem owns the object's lifetime.

### Why this exists

Constructing `impl::ManagedInstance` with the correct `unique_ptr` + deleter template arguments is verbose and error-prone. `UniqueInstance` pre-wires those arguments so callers only need to specify the concrete type (`TTarget`) and optionally a base interface type (`TBase`). The `std::function` deleter allows the allocator to be captured by value, so the correct `MemoryResource` is used at destruction even if the pointer is moved across subsystems.

### Construction

```cpp
pma::MemoryResource* memRes = pma::makeDefaultMemoryResource();
// Create a DNAReader with exclusive ownership
auto reader = pma::UniqueInstance<DNAReader>::with(memRes).create(stream, dataLayer);
// reader type: std::unique_ptr<DNAReader, std::function<void(DNAReader*)>>
```

### Relationships

- `ManagedInstance` — the underlying template this alias configures
- `SharedInstance` — use instead when shared ownership across subsystems is required
- `MemoryResource` — the allocator policy supplied via `with()`

### Constraints

- The `MemoryResource*` supplied to `with()` must outlive the returned `unique_ptr`. The deleter holds a copy of the `PolyAllocator` which references that pointer.

<!-- ink:api-end name="UniqueInstance" -->
