# API Reference — `pma/TypeDefs`

---

<!-- ink:api name="List" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename Allocator = PolyAllocator<T>> using List = std::list<T, Allocator>;`

Allocator-aware drop-in replacement for `std::list`.

### Why this exists

Keeps node-based sequence containers on the same `PolyAllocator` default as the rest of `pma`'s typedefs, so list-heavy code participates in the same memory-resource control as vectors and maps.

<!-- ink:api-end name="List" -->

<!-- ink:api name="Map" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename K, typename V, typename Allocator = PolyAllocator<std::pair<const K, V>>> using Map = std::map<K, V, std::less<K>, Allocator>;`

Allocator-aware drop-in replacement for `std::map`, ordered by key using `std::less<K>`.

### Why this exists

The allocator is parameterized over `std::pair<const K, V>` (the actual node value type `std::map` allocates), so `Map` correctly routes node allocations through a `pma::MemoryResource` without the caller needing to know that detail.

<!-- ink:api-end name="Map" -->

<!-- ink:api name="Matrix" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename Allocator = PolyAllocator<Vector<T>>> using Matrix = Vector<Vector<T>, Allocator>;`

A 2D, row-of-rows matrix built from nested `pma::Vector`s, for allocator-aware tabular data.

### Why this exists

Rather than introducing a dedicated dense-matrix type, `Matrix` reuses `Vector<Vector<T>>` so rows are independently sized and allocated through the same `PolyAllocator` machinery as every other `pma` container.

<!-- ink:api-end name="Matrix" -->

<!-- ink:api name="Set" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename Allocator = PolyAllocator<T>> using Set = std::set<T, std::less<T>, Allocator>;`

Allocator-aware drop-in replacement for `std::set`, ordered by `std::less<T>`.

### Why this exists

Standardizes the comparator (`std::less<T>`) and allocator across the codebase so ordered-set usages are consistent and can be redirected to a custom memory resource without changing call sites.

<!-- ink:api-end name="Set" -->

<!-- ink:api name="String" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename Allocator = PolyAllocator<T>> using String = std::basic_string<T, std::char_traits<T>, Allocator>;`

Use the project's polymorphic-allocator-backed string type wherever a `std::string` would normally appear.

### Why this exists

`String` swaps in `pma::PolyAllocator` as the default allocator so strings allocate from the memory resource associated with their owning object rather than the global heap, while still behaving exactly like `std::basic_string` at every call site.

<!-- ink:api-end name="String" -->

<!-- ink:api name="UnorderedMap" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename K, typename V, typename Allocator = PolyAllocator<std::pair<const K, V>>> using UnorderedMap = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, Allocator>;`

Allocator-aware drop-in replacement for `std::unordered_map`.

### Why this exists

Like `Map`, the allocator targets the map's actual node value type (`std::pair<const K, V>`), so hash-map instances allocate through the configured `pma::MemoryResource` transparently.

<!-- ink:api-end name="UnorderedMap" -->

<!-- ink:api name="UnorderedSet" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename Allocator = PolyAllocator<T>> using UnorderedSet = std::unordered_set<T, std::hash<T>, std::equal_to<T>, Allocator>;`

Allocator-aware drop-in replacement for `std::unordered_set`.

### Why this exists

Pins the hash (`std::hash<T>`) and equality (`std::equal_to<T>`) policies alongside the `PolyAllocator` default, giving hash-set usages the same memory-resource flexibility as the rest of `pma`'s containers.

<!-- ink:api-end name="UnorderedSet" -->

<!-- ink:api name="Vector" module="pma/TypeDefs" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename T, typename Allocator = PolyAllocator<T>> using Vector = std::vector<T, Allocator>;`

Use this in place of `std::vector` for elements that should be allocated from a `pma::MemoryResource`.

### Why this exists

Defaulting the allocator to `PolyAllocator<T>` means containers throughout the codebase share the same allocator abstraction, so swapping memory resources (arena, default, aligned) doesn't require touching container declarations.

<!-- ink:api-end name="Vector" -->
