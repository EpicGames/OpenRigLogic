# API Reference — `trio/streams/MemoryStream`

---

<!-- ink:api name="DefaultInstanceCreator" module="trio/streams/MemoryStream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceCreator<trio::MemoryStream>`

Template specialization that binds `trio::MemoryStream` to its factory-based creation function.

### Why this exists

Lets generic `pma` code create a `MemoryStream` without knowing its concrete factory function, by routing through `FactoryCreate<trio::MemoryStream>`. Pairs with `DefaultInstanceDestroyer` to keep creation and destruction consistent for this type.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryCreate<trio::MemoryStream>` | required. The callable type used to create a `MemoryStream` instance. |

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="trio/streams/MemoryStream" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `DefaultInstanceDestroyer<trio::MemoryStream>`

Template specialization that binds `trio::MemoryStream` to its factory-based destruction function.

### Why this exists

Mirrors `DefaultInstanceCreator<trio::MemoryStream>` on the destruction side, routing through `FactoryDestroy<trio::MemoryStream>` so generic `pma` code can free instances without a hardcoded destructor call.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `FactoryDestroy<trio::MemoryStream>` | required. The callable type used to destroy a `MemoryStream` instance created via the factory. |

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="MemoryStream" module="trio/streams/MemoryStream" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class MemoryStream : public BoundedIOStream`

In-memory stream. Use this when you need a `BoundedIOStream`-compatible buffer that isn't backed by a file, for example to stage serialized data before writing it out.

### When to use this

Reach for `MemoryStream` when you need a growable in-memory buffer that behaves like any other `trio` stream — for staging, testing, or in-process transfer of binary data. Use `MemoryStream::create(initialSize, ...)` instead of the no-argument overload when you know the approximate size up front and want to avoid repeated reallocation.

### Example

```cpp
trio::MemoryStream* stream = trio::MemoryStream::create(4096u);
// ... write serialized data into stream ...
trio::MemoryStream::destroy(stream);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `memRes` | `MemoryResource*` | optional. Memory resource used for allocating the instance; a default allocation mechanism is used if not given. |
| `initialSize` | `std::size_t` | optional (second overload). Initial size of the memory stream. |

### Returns

`MemoryStream*` — pointer to the newly created in-memory stream instance.

### Watch out for

- The user is responsible for releasing the returned pointer by calling `destroy` — there is no implicit cleanup.
- Copy construction and copy assignment are deleted; only move construction/assignment are supported.
- The caller is always responsible for calling `destroy(instance)` to release the instance. There is no RAII wrapper — skipping `destroy` leaks the buffer.

<!-- ink:api-end name="MemoryStream" -->
