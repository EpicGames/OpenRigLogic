# API Reference — `pma`

_36 entries across 8 modules._

## By Task

### Memory management

Allocators and resource strategies for dynamic memory.

| API | Module | Summary |
|-----|--------|---------|
| [MemoryResource](MemoryResource.md) | MemoryResource | Abstract base class for memory allocation |
| [PolyAllocator](PolyAllocator.md) | PolyAllocator | Polymorphic allocator wrapping MemoryResource |
| [DefaultMemoryResource](resources/DefaultMemoryResource.md) | DefaultMemoryResource | Default system memory allocator |
| [AlignedMemoryResource](resources/AlignedMemoryResource.md) | AlignedMemoryResource | Allocator with custom alignment support |
| [ArenaMemoryResource](resources/ArenaMemoryResource.md) | ArenaMemoryResource | Arena-style allocator for bulk allocation |

### Memory resource utilities

| API | Module | Summary |
|-----|--------|---------|
| [max_align_of](PolyAllocator.md) | PolyAllocator | Maximum alignment value for types |
| [min_align_t](PolyAllocator.md) | PolyAllocator | Minimum alignment type |
| [TDefaultMemoryResource](PolyAllocator.md) | PolyAllocator | Template for default memory resource |
| [rebind](PolyAllocator.md) | PolyAllocator | Rebind allocator to different type |

### Pointer management

Safe pointer wrappers for ownership and lifetime management.

| API | Module | Summary |
|-----|--------|---------|
| [ScopedPtr](ScopedPtr.md) | ScopedPtr | Exclusive ownership smart pointer |
| [makeScoped](ScopedPtr.md) | ScopedPtr | Factory function for scoped pointers |
| [New](ScopedPtr.md) | ScopedPtr | Allocate and construct in one call |
| [Delete](ScopedPtr.md) | ScopedPtr | Deallocate with custom destructor |
| [FactoryCreate](ScopedPtr.md) | ScopedPtr | Create instances via factory |
| [FactoryDestroy](ScopedPtr.md) | ScopedPtr | Destroy instances via factory |

### Managed instances

Instances with shared or unique lifetime management.

| API | Module | Summary |
|-----|--------|---------|
| [ManagedInstance](utils/ManagedInstance.md) | ManagedInstance | Base for managed pointer instances |
| [SharedInstance](utils/ManagedInstance.md) | ManagedInstance | Shared ownership instance wrapper |
| [UniqueInstance](utils/ManagedInstance.md) | ManagedInstance | Unique ownership instance wrapper |
| [MemoryResource](utils/ManagedInstance.md) | ManagedInstance | Associated memory resource |
| [PointerType](utils/ManagedInstance.md) | ManagedInstance | Underlying pointer type |

### Standard container aliases

Type aliases for STL containers.

| API | Module | Summary |
|-----|--------|---------|
| [String](TypeDefs.md) | TypeDefs | Alias for `std::string` |
| [Vector](TypeDefs.md) | TypeDefs | Alias for `std::vector` |
| [Matrix](TypeDefs.md) | TypeDefs | Alias for 2D vector (vector of vectors) |
| [List](TypeDefs.md) | TypeDefs | Alias for `std::list` |
| [Set](TypeDefs.md) | TypeDefs | Alias for `std::set` |
| [UnorderedSet](TypeDefs.md) | TypeDefs | Alias for `std::unordered_set` |
| [Map](TypeDefs.md) | TypeDefs | Alias for `std::map` |
| [UnorderedMap](TypeDefs.md) | TypeDefs | Alias for `std::unordered_map` |

### Instance creation & destruction

| API | Module | Summary |
|-----|--------|---------|
| [DefaultInstanceCreator](ScopedPtr.md) | ScopedPtr | Creates default instances |
| [DefaultInstanceDestroyer](ScopedPtr.md) | ScopedPtr | Destroys default instances |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| AlignedMemoryResource | [resources/AlignedMemoryResource.md](resources/AlignedMemoryResource.md) | 1 |
| ArenaMemoryResource | [resources/ArenaMemoryResource.md](resources/ArenaMemoryResource.md) | 1 |
| DefaultMemoryResource | [resources/DefaultMemoryResource.md](resources/DefaultMemoryResource.md) | 1 |
| ManagedInstance | [utils/ManagedInstance.md](utils/ManagedInstance.md) | 5 |
| MemoryResource | [MemoryResource.md](MemoryResource.md) | 1 |
| PolyAllocator | [PolyAllocator.md](PolyAllocator.md) | 5 |
| ScopedPtr | [ScopedPtr.md](ScopedPtr.md) | 8 |
| TypeDefs | [TypeDefs.md](TypeDefs.md) | 8 |
