# API Reference — `trio/streams/MemoryStream`

---

<!-- ink:api name="DefaultInstanceCreator" module="trio/streams/MemoryStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceCreator<trio::MemoryStream>`

Template specialization that registers `trio::MemoryStream` with the `pma` framework's creation policy.

### Why this exists

The `pma` allocator framework uses policy traits to stay decoupled from concrete types: any type that wants to participate in `pma`-managed allocation must specialize `DefaultInstanceCreator`. This specialization delegates creation to `FactoryCreate<trio::MemoryStream>`, which in turn calls `MemoryStream::create()`. Without it, `pma::create<MemoryStream>()` would not compile.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<trio::MemoryStream>` | required — the policy type that `pma` machinery uses to construct `MemoryStream` instances. |

### Relationships

- `DefaultInstanceDestroyer<trio::MemoryStream>` — paired destruction policy; always defined alongside this specialization.
- `MemoryStream::create()` — the factory method that `FactoryCreate` delegates to.

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="trio/streams/MemoryStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<trio::MemoryStream>`

Template specialization that registers `trio::MemoryStream` with the `pma` framework's destruction policy.

### Why this exists

Paired with `DefaultInstanceCreator<trio::MemoryStream>`, this specialization completes the `pma` lifecycle contract for `MemoryStream`. It delegates destruction to `FactoryDestroy<trio::MemoryStream>`, which calls `MemoryStream::destroy()`. Both specializations must be present for `pma`-managed code to create and release `MemoryStream` instances safely.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<trio::MemoryStream>` | required — the policy type that `pma` machinery uses to destroy `MemoryStream` instances. |

### Relationships

- `DefaultInstanceCreator<trio::MemoryStream>` — paired creation policy; always defined alongside this specialization.
- `MemoryStream::destroy()` — the static method that `FactoryDestroy` delegates to.

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="MemoryStream" module="trio/streams/MemoryStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `MemoryStream`

An in-memory byte stream backed by a heap-allocated buffer. Use this when you need a full `BoundedIOStream` interface without touching the filesystem — for instance, to serialize DNA data into a buffer for in-process transfer or round-trip codec testing.

### When to use this

Reach for `MemoryStream` when the data volume fits in RAM and the consumer expects a stream interface (e.g., a `BinaryStreamWriter` or `BinaryStreamReader`). Use a file-backed stream instead when data must persist across process exit or when working with sizes that exceed available heap.

### Example

```cpp
// Create a memory stream with a pre-sized buffer
trio::MemoryStream* stream = trio::MemoryStream::create(4096u);

// ... write or read through the stream ...

// Release when done — ownership is always with the caller
trio::MemoryStream::destroy(stream);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `initialSize` | `std::size_t` | optional — initial capacity of the in-memory buffer in bytes. Omit to use the implementation default. |
| `memRes` | `MemoryResource*` | optional — custom memory resource for the allocation. Pass `nullptr` (or omit) to use the default allocator. |

### Watch out for

- Copy construction and copy assignment are deleted. `MemoryStream` instances cannot be shared — transfer ownership via `std::move` or pass by pointer.
- The caller is always responsible for calling `destroy(instance)` to release the instance. There is no RAII wrapper — skipping `destroy` leaks the buffer.

<!-- ink:api-end name="MemoryStream" -->
