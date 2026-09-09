# API Reference — `pma/MemoryResource`

---

<!-- ink:api name="MemoryResource" module="pma/MemoryResource" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class PMAAPI MemoryResource`

An abstract base for polymorphic allocators, letting arbitrary allocation strategies be passed across API boundaries without changing the signatures or types involved.

### When to use this

Use this when a component needs to accept a caller-supplied allocator without templating every function or type on the allocator's concrete type — implement `allocate`/`deallocate` and pass a pointer to your `MemoryResource` wherever the API expects one. This is the base type that `PolyAllocator` wraps to plug into standard-library containers.

### Method groups

| Group | Methods |
|-------|---------|
| Allocation | allocate, deallocate |

### Example

```cpp
class MyArenaResource : public pma::MemoryResource {
public:
    void* allocate(std::size_t size, std::size_t alignment) override {
        return arena.alloc(size, alignment);
    }
    void deallocate(void* ptr, std::size_t size, std::size_t alignment) override {
        arena.free(ptr, size, alignment);
    }
};
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `size` | `std::size_t` | required — number of bytes requested (allocate) or being freed (deallocate). |
| `alignment` | `std::size_t` | required — required alignment in bytes for the allocation. |
| `ptr` | `void*` | required — pointer previously returned by `allocate`, being released. |

### Watch out for

- Both `allocate` and `deallocate` are pure virtual — any concrete subclass must implement both or the compiler will reject instantiation.
- `allocate` receives an explicit `alignment` argument. Ignoring it and using a plain `malloc` will produce undefined behavior for over-aligned types. Use `std::aligned_alloc`, `_aligned_malloc`, or a platform equivalent.

<!-- ink:api-end name="MemoryResource" -->
