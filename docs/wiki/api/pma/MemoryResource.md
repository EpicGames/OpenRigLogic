# API Reference — `pma/MemoryResource`

---

<!-- ink:api name="MemoryResource" module="pma/MemoryResource" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class PMAAPI MemoryResource`

Abstract base for polymorphic memory allocation. Implement this interface to supply a custom allocator across API boundaries without modifying the signatures of the APIs that consume it.

### When to use this

Use `MemoryResource` when you need to pass a custom allocator into a subsystem that you do not own — for example, to redirect all `pma` allocations to a pool, arena, or tracking wrapper. Because it decouples the allocator policy from the API surface, you can swap implementations at runtime without touching call sites.

### Example

```cpp
#include "pma/MemoryResource.h"
#include <cstdlib>

class MallocResource : public pma::MemoryResource {
public:
    void* allocate(std::size_t size, std::size_t alignment) override {
        // Use aligned_alloc or platform equivalent
        return std::aligned_alloc(alignment, size);
    }

    void deallocate(void* ptr, std::size_t /*size*/, std::size_t /*alignment*/) override {
        std::free(ptr);
    }
};

// Pass to any pma API that accepts a MemoryResource*
MallocResource resource;
auto* reader = pma::BinaryStreamReader::create(&stream, &resource);
```

### Watch out for

- Both `allocate` and `deallocate` are pure virtual — any concrete subclass must implement both or the compiler will reject instantiation.
- `allocate` receives an explicit `alignment` argument. Ignoring it and using a plain `malloc` will produce undefined behavior for over-aligned types. Use `std::aligned_alloc`, `_aligned_malloc`, or a platform equivalent.

<!-- ink:api-end name="MemoryResource" -->
