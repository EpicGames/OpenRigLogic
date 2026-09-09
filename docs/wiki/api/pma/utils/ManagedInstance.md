# API Reference — `pma/utils/ManagedInstance`

---

<!-- ink:api name="ManagedInstance" module="pma/utils/ManagedInstance" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<class TPointer, class TTarget, class TBase = TTarget> class ManagedInstance { static ManagedInstance with(MemoryResource* memRes); PointerType create(Args&&... args); }`

Build a smart pointer (`unique_ptr` or `shared_ptr`) whose object was allocated from a specific `pma::MemoryResource`, with a deleter that knows how to free it back to that same resource.

### When to use this

Use `ManagedInstance::with(memRes).create(...)` instead of `std::make_unique`/`std::make_shared` whenever the object must be allocated through a particular `MemoryResource` — the deleter captures the allocator so the object is freed correctly even though the caller only holds a generic smart pointer.

### Example

```cpp
pma::MemoryResource* memRes = getArenaMemoryResource();
auto instance = pma::impl::ManagedInstance<std::unique_ptr<Base, std::function<void(Base*)>>, Derived, Base>::with(memRes)
                    .create(constructorArg1, constructorArg2);
// instance is a std::unique_ptr<Base, ...> whose deleter frees Derived via memRes
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `memRes` | `MemoryResource*` | required — the memory resource the constructed object is allocated from and later freed to. |
| `args` | `Args&&...` | required — forwarded to `TTarget`'s constructor. |

### Returns

`PointerType` — a `unique_ptr`/`shared_ptr` (per the `TPointer` alias used) holding the newly constructed `TTarget`, deleted through `memRes`.

### Watch out for

- The deleter lambda captures `PolyAllocator<TTarget>` by value. The underlying `MemoryResource*` is stored inside that allocator copy. If the `MemoryResource` is destroyed before the last smart pointer goes out of scope, the deleter accesses a dangling pointer and produces undefined behavior.
- `ManagedInstance` lives in `namespace pma::impl` — use the `UniqueInstance` and `SharedInstance` public aliases rather than naming this template directly. The template parameters are order-sensitive and error-prone to spell manually.

<!-- ink:api-end name="ManagedInstance" -->

<!-- ink:api name="MemoryResource" module="pma/utils/ManagedInstance" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `class MemoryResource;`

Forward declaration of the `MemoryResource` interface used by `ManagedInstance` to allocate and free the objects it manages.

### Why this exists

`ManagedInstance` only needs a pointer to `MemoryResource`, so a forward declaration here avoids pulling in the full `MemoryResource` header just for this utility.

### Relationships

- `ManagedInstance` — *uses `MemoryResource*` to create and destroy managed objects.*

<!-- ink:api-end name="MemoryResource" -->

<!-- ink:api name="PointerType" module="pma/utils/ManagedInstance" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `using PointerType = TPointer;`

Public member typedef of `ManagedInstance` naming the smart-pointer type it produces from `create()`.

### Why this exists

Exposing `PointerType` lets `UniqueInstance`/`SharedInstance` (and callers) refer to the exact return type of `create()` without repeating the full `std::unique_ptr<...>`/`std::shared_ptr<...>` spelling.

<!-- ink:api-end name="PointerType" -->

<!-- ink:api name="SharedInstance" module="pma/utils/ManagedInstance" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `SharedInstance`

A managed-instance alias that wraps a target type in a `std::shared_ptr`, giving multiple owners a reference-counted handle to the same instance.

### Why this exists

Constructing and destroying `pma` types can require routing through custom allocators and factory/destroy functions rather than plain `new`/`delete`. `SharedInstance` packages that lifecycle management behind the familiar `std::shared_ptr` interface so callers get reference-counted shared ownership without hand-rolling a custom deleter each time. It pairs with `UniqueInstance`, which provides the same guarantee for single-owner (`std::unique_ptr`) lifetimes.

### Relationships

- `UniqueInstance` — *sibling alias using `std::unique_ptr` with a custom deleter for single ownership.*
- `ManagedInstance` — *the underlying `impl::ManagedInstance` template that both aliases specialize.*

### Constraints

- The `MemoryResource*` supplied to `with()` must outlive all copies of the returned `shared_ptr`. The deleter embedded in the control block references that pointer at destruction time.

<!-- ink:api-end name="SharedInstance" -->

<!-- ink:api name="UniqueInstance" module="pma/utils/ManagedInstance" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class TTarget, class TBase = TTarget> using UniqueInstance = impl::ManagedInstance<std::unique_ptr<TBase, std::function<void(TBase*)>>, TTarget, TBase>;`

Convenience alias for building `ManagedInstance`s that hand back a `std::unique_ptr` with a memory-resource-aware deleter.

### Why this exists

Spelling out `impl::ManagedInstance<std::unique_ptr<TBase, std::function<void(TBase*)>>, TTarget, TBase>` at every call site is verbose and easy to get wrong; `UniqueInstance<TTarget, TBase>` gives the same behavior with just the two types that matter to the caller.

### Construction

```cpp
auto ptr = pma::UniqueInstance<Derived, Base>::with(memRes).create(args...);
```

### Relationships

- `ManagedInstance` — *the underlying implementation `UniqueInstance` aliases.*
- `SharedInstance` — *sibling alias producing a `shared_ptr` instead.*

### Constraints

- The `MemoryResource*` supplied to `with()` must outlive the returned `unique_ptr`. The deleter holds a copy of the `PolyAllocator` which references that pointer.

<!-- ink:api-end name="UniqueInstance" -->
