# API Reference — `pma`

_38 entries across 8 modules._

## By Task

### Smart pointer lifetime management

Construct and destroy objects with pluggable creator and destroyer policies

| API | Module | Summary |
|-----|--------|---------|
| [DefaultInstanceCreator](ScopedPtr.md) | ScopedPtr | Select `New<T>` as the default creator policy when no custom creator is specified for `ScopedPtr`/`makeScoped`. |
| [DefaultInstanceDestroyer](ScopedPtr.md) | ScopedPtr | Select `Delete<T>` as the default destroyer policy when no custom destroyer is specified for `ScopedPtr`/`makeScoped`. |
| [Delete](ScopedPtr.md) | ScopedPtr | The default destruction policy for `ScopedPtr`, releasing a pointer allocated by `New` via `delete`. |
| [destroyer_type](ScopedPtr.md) | ScopedPtr | Public alias on `ScopedPtr` for its `TDestroyer` template parameter — the policy type invoked to release the owned pointer. |
| [FactoryCreate](ScopedPtr.md) | ScopedPtr | Create an object through a type's static `create` factory method instead of `new`, for types that manage their own construction. |
| [FactoryDestroy](ScopedPtr.md) | ScopedPtr | Destroy an object through a type's static `destroy` method instead of `delete`, matching a `FactoryCreate`-constructed instance. |
| [makeScoped](ScopedPtr.md) | ScopedPtr | Construct a `ScopedPtr<T>` by invoking a creator policy and binding its matched destroyer — the preferred way to create any `ScopedPtr`. |
| [New](ScopedPtr.md) | ScopedPtr | A default construction policy for `ScopedPtr` that allocates an instance with `new` and forwards constructor arguments. |
| [ScopedPtr](ScopedPtr.md) | ScopedPtr | Takes ownership over a given pointer and handles its lifetime, similar in spirit to `std::unique_ptr` but with pluggable, stateful or stateless destroyer policies. |
| [TCreator](ScopedPtr.md) | ScopedPtr | `TCreator` is the template parameter of `makeScoped` naming the callable invoked as `TCreator{}(args...)` to construct the object that ends up owned by the returned `ScopedPtr`. |
| [TCreatorTemplate](ScopedPtr.md) | ScopedPtr | `TCreatorTemplate` is a template-template parameter — a template like `DefaultInstanceCreator` — that this `makeScoped` overload instantiates with `T` to obtain the concrete creator type. |
| [TDestroyer](ScopedPtr.md) | ScopedPtr | `TDestroyer` is the template parameter naming the destroyer policy that `makeScoped` bakes into the returned `ScopedPtr<Base, TDestroyer>`'s type. |

### Memory resource strategies

Implement and use different allocation and deallocation strategies

| API | Module | Summary |
|-----|--------|---------|
| [AlignedMemoryResource](resources/AlignedMemoryResource.md) | AlignedMemoryResource | A `MemoryResource` that honors alignment requirements stricter than the default allocator provides. |
| [ArenaMemoryResource](resources/ArenaMemoryResource.md) | ArenaMemoryResource | Serves allocations from a preallocated memory region instead of hitting the upstream allocator on every request. |
| [DefaultMemoryResource](resources/DefaultMemoryResource.md) | DefaultMemoryResource | A `MemoryResource` that delegates directly to `malloc`/`free`. |
| [MemoryResource](MemoryResource.md) | MemoryResource | An abstract base for polymorphic allocators, letting arbitrary allocation strategies be passed across API boundaries without changing the signatures or types involved. |
| [MemoryResource](utils/ManagedInstance.md) | ManagedInstance | Forward declaration of the `MemoryResource` interface used by `ManagedInstance` to allocate and free the objects it manages. |

### Polymorphic allocator

Use runtime-selected memory strategies in standard library containers without template coupling

| API | Module | Summary |
|-----|--------|---------|
| [PolyAllocator](PolyAllocator.md) | PolyAllocator | A standard-library-compatible allocator that forwards allocation and construction to a runtime-supplied `MemoryResource`, letting containers use arbitrary memory strategies without being templated on the allocator's concrete type. |
| [TDefaultMemoryResource](PolyAllocator.md) | PolyAllocator | The fallback `MemoryResource` type a `PolyAllocator` uses when constructed without an explicit memory resource (or with `nullptr`). |
| [UDefaultMemoryResource](PolyAllocator.md) | PolyAllocator | The corresponding default-memory-resource template parameter for the "other" `PolyAllocator` type in cross-allocator comparisons (`operator==`/`operator!=`). |

### Allocator-aware containers

Standard library container replacements that work with polymorphic memory resources

| API | Module | Summary |
|-----|--------|---------|
| [List](TypeDefs.md) | TypeDefs | Allocator-aware drop-in replacement for `std::list`. |
| [Map](TypeDefs.md) | TypeDefs | Allocator-aware drop-in replacement for `std::map`, ordered by key using `std::less<K>`. |
| [Matrix](TypeDefs.md) | TypeDefs | A 2D, row-of-rows matrix built from nested `pma::Vector`s, for allocator-aware tabular data. |
| [Set](TypeDefs.md) | TypeDefs | Allocator-aware drop-in replacement for `std::set`, ordered by `std::less<T>`. |
| [String](TypeDefs.md) | TypeDefs | Use the project's polymorphic-allocator-backed string type wherever a `std::string` would normally appear. |
| [UnorderedMap](TypeDefs.md) | TypeDefs | Allocator-aware drop-in replacement for `std::unordered_map`. |
| [UnorderedSet](TypeDefs.md) | TypeDefs | Allocator-aware drop-in replacement for `std::unordered_set`. |
| [Vector](TypeDefs.md) | TypeDefs | Use this in place of `std::vector` for elements that should be allocated from a `pma::MemoryResource`. |

### Managed smart pointer instances

Create smart pointers backed by memory resources with automatic cleanup

| API | Module | Summary |
|-----|--------|---------|
| [ManagedInstance](utils/ManagedInstance.md) | ManagedInstance | Build a smart pointer (`unique_ptr` or `shared_ptr`) whose object was allocated from a specific `pma::MemoryResource`, with a deleter that knows how to free it back to that same resource. |
| [PointerType](utils/ManagedInstance.md) | ManagedInstance | Public member typedef of `ManagedInstance` naming the smart-pointer type it produces from `create()`. |
| [SharedInstance](utils/ManagedInstance.md) | ManagedInstance | A managed-instance alias that wraps a target type in a `std::shared_ptr`, giving multiple owners a reference-counted handle to the same instance. |
| [UniqueInstance](utils/ManagedInstance.md) | ManagedInstance | Convenience alias for building `ManagedInstance`s that hand back a `std::unique_ptr` with a memory-resource-aware deleter. |

### Type traits and internal utilities

Compile-time type introspection and alignment helpers used internally by memory and pointer facilities

| API | Module | Summary |
|-----|--------|---------|
| [complete_type_checker](ScopedPtr.md) | ScopedPtr | A local array-type alias (`char[sizeof(T) ? 1 : -1]`) used purely to force a compile-time error when `T` is incomplete at the point `Delete<T>` deletes it. |
| [inspect](ScopedPtr.md) | ScopedPtr | A private trait used by `ScopedPtr` to derive its `pointer`, `element_type`, and `is_array` members from the template argument `T`. |
| [is_array](ScopedPtr.md) | ScopedPtr | Compile-time tag member of `inspect<U>` indicating whether `T` is an array type. |
| [max_align_of](PolyAllocator.md) | PolyAllocator | A compile-time trait that resolves to whichever of two types, `T` or `U`, has the greater alignment. |
| [min_align_t](PolyAllocator.md) | PolyAllocator | An internal alias (`void*`) used as the minimal-alignment reference type when computing a `PolyAllocator`'s default alignment. |
| [pointer_type](ScopedPtr.md) | ScopedPtr | Member typedef of the private `inspect<U>` trait struct, used internally by `ScopedPtr` to derive its public `pointer` alias. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| MemoryResource | [MemoryResource.md](MemoryResource.md) | 1 |
| PolyAllocator | [PolyAllocator.md](PolyAllocator.md) | 5 |
| ScopedPtr | [ScopedPtr.md](ScopedPtr.md) | 16 |
| TypeDefs | [TypeDefs.md](TypeDefs.md) | 8 |
| AlignedMemoryResource | [resources/AlignedMemoryResource.md](resources/AlignedMemoryResource.md) | 1 |
| ArenaMemoryResource | [resources/ArenaMemoryResource.md](resources/ArenaMemoryResource.md) | 1 |
| DefaultMemoryResource | [resources/DefaultMemoryResource.md](resources/DefaultMemoryResource.md) | 1 |
| ManagedInstance | [utils/ManagedInstance.md](utils/ManagedInstance.md) | 5 |
