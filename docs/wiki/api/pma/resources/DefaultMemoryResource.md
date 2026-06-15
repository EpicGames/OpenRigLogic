# API Reference — `pma/resources/DefaultMemoryResource`

---

<!-- ink:api name="DefaultMemoryResource" module="pma/resources/DefaultMemoryResource" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class PMAAPI DefaultMemoryResource : public MemoryResource`

Use the system allocator (`malloc`/`free`) without any pooling or custom arena. This is the default `MemoryResource` when no custom allocator is needed.

### When to use this

Reach for `DefaultMemoryResource` when you need a valid `MemoryResource` instance but have no custom allocation strategy — for example, during initial integration or prototyping before a pool allocator is warranted. Prefer a pool or arena allocator when allocation frequency or fragmentation is a concern.

### Example

```cpp
pma::DefaultMemoryResource memRes;

// Allocate 256 bytes aligned to double's alignment requirement
void* ptr = memRes.allocate(256, alignof(double));

// ... use ptr ...

// Release the memory using the same size and alignment
memRes.deallocate(ptr, 256, alignof(double));
```

### Parameters

`allocate`:

| Name | Type | Description |
|------|------|-------------|
| `size` | `std::size_t` | required — number of bytes to allocate |
| `alignment` | `std::size_t` | required — alignment requirement in bytes; must be a power of two |

`deallocate`:

| Name | Type | Description |
|------|------|-------------|
| `ptr` | `void*` | required — pointer previously returned by `allocate` |
| `size` | `std::size_t` | required — must match the size passed to the corresponding `allocate` call |
| `alignment` | `std::size_t` | required — must match the alignment passed to the corresponding `allocate` call |

### Returns

`allocate` returns `void*` — pointer to the newly allocated memory block of the requested size and alignment.

<!-- ink:api-end name="DefaultMemoryResource" -->
