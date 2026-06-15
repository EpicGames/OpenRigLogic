# API Reference — `pma/resources/AlignedMemoryResource`

---

<!-- ink:api name="AlignedMemoryResource" module="pma/resources/AlignedMemoryResource" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `AlignedMemoryResource : public MemoryResource`

Allocate and free memory blocks at a specific byte boundary. Use this when your data structures require alignment guarantees — for example, SIMD-optimized types — that the default allocator does not provide.

### When to use this

Reach for `AlignedMemoryResource` when allocation must land on a specific byte boundary, such as 16-byte-aligned buffers for SSE operations or 64-byte boundaries for cache-line-sensitive data. Use the base `MemoryResource` when alignment is not a concern.

### Example

```cpp
pma::AlignedMemoryResource resource;

// Allocate 256 bytes aligned to a 16-byte boundary (e.g., for SSE data)
void* buffer = resource.allocate(256, 16);

// ... use buffer ...

// Deallocate — pass the same size and alignment used in allocate
resource.deallocate(buffer, 256, 16);
```

### Parameters

`allocate`:

| Name | Type | Description |
|------|------|-------------|
| `size` | `std::size_t` | required — number of bytes to allocate |
| `alignment` | `std::size_t` | required — required byte alignment of the returned pointer |

`deallocate`:

| Name | Type | Description |
|------|------|-------------|
| `ptr` | `void*` | required — pointer previously returned by `allocate` |
| `size` | `std::size_t` | required — must match the size passed to `allocate` |
| `alignment` | `std::size_t` | required — must match the alignment passed to `allocate` |

### Returns

`void*` — pointer to the allocated memory block, aligned to the requested boundary. Returned by `allocate`; `deallocate` returns nothing.

### Watch out for

- `deallocate` requires passing the **same** `size` and `alignment` values that were used in the corresponding `allocate` call. Mismatched values are undefined behavior — track these at the call site.

### Relationships

- `MemoryResource` — base class; `AlignedMemoryResource` extends it to honor alignment contracts

<!-- ink:api-end name="AlignedMemoryResource" -->
