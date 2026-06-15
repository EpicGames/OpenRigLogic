# API Reference — `pma/TypeDefs`

---

<!-- ink:api name="List" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::List<T, Allocator>`

Allocator-aware doubly-linked list for the pma module, backed by `std::list` with `PolyAllocator` by default.

### Why this exists

`std::list` allocates one node per element on the global heap, scattering lifetime control across independent allocations. This alias routes every node allocation through `PolyAllocator`, allowing the entire list to be freed by releasing the arena rather than deallocating nodes individually. Prefer `pma::Vector` when random access or cache locality matters; reach for `pma::List` when you need stable iterators after insertions and deletions.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type parameter | required — element type |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<T>` |

### Construction

```cpp
pma::List<std::uint32_t> jointIndices{allocator};
jointIndices.push_back(0u);
jointIndices.push_back(3u);
```

### Relationships

- `pma::Vector` — preferred alternative when random access or contiguous memory is needed
- `pma::PolyAllocator` — the default allocator injected by this alias
- `std::list` — the underlying standard type this alias wraps

<!-- ink:api-end name="List" -->

<!-- ink:api name="Map" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::Map<K, V, Allocator>`

Allocator-aware ordered key-value map for the pma module, backed by `std::map` with `PolyAllocator` by default.

### Why this exists

`std::map` allocates one tree node per key-value pair on the global heap. This alias routes those allocations through `PolyAllocator`, scoping the map's entire storage to a `pma` arena. Keys are ordered by `std::less<K>`, giving sorted iteration and logarithmic lookup. Use `pma::UnorderedMap` when insertion order and range iteration are irrelevant and O(1) lookup is preferred.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `K` | template type parameter | required — key type; must satisfy `std::less<K>` |
| `V` | template type parameter | required — mapped value type |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<std::pair<const K, V>>` |

### Construction

```cpp
pma::Map<std::string, std::uint32_t> jointIndexMap{allocator};
jointIndexMap.emplace("spine_01", 0u);
jointIndexMap.emplace("spine_02", 1u);
auto idx = jointIndexMap.at("spine_01"); // 0
```

### Relationships

- `pma::UnorderedMap` — hash-based alternative for O(1) average lookup when key order is irrelevant
- `pma::Set` — key-only ordered companion type
- `pma::PolyAllocator` — the default allocator injected by this alias
- `std::map` — the underlying standard type this alias wraps

<!-- ink:api-end name="Map" -->

<!-- ink:api name="Matrix" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::Matrix<T, Allocator>`

Two-dimensional dynamic array (vector of vectors) for the pma module, with `PolyAllocator` at both nesting levels.

### Why this exists

A raw `std::vector<std::vector<T>>` uses two independent global-heap allocations per row — the outer vector and each inner vector — making it impossible to scope them to a single arena. `pma::Matrix` nests `pma::Vector<T>` inside `pma::Vector<pma::Vector<T>>`, so both levels use `PolyAllocator` and the entire 2D structure participates in the same memory lifetime. Use this for weight tables, blend-shape matrices, and joint influence grids where all rows must be freed together.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type parameter | required — element type of the inner rows |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<Vector<T>>`; controls the outer container's allocator |

### Construction

```cpp
// A 3-row weight matrix, each row pre-reserved
pma::Matrix<float> weights{allocator};
weights.resize(3);
for (auto& row : weights) {
    row.reserve(64);
}
```

### Relationships

- `pma::Vector` — the alias used for both the outer container and each inner row
- `pma::PolyAllocator` — the default allocator for both nesting levels

<!-- ink:api-end name="Matrix" -->

<!-- ink:api name="Set" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::Set<T, Allocator>`

Allocator-aware ordered set for the pma module, backed by `std::set` with `PolyAllocator` by default.

### Why this exists

`std::set` allocates tree nodes on the global heap, making it impossible to scope them to a `pma` arena without an explicit allocator. This alias routes every node through `PolyAllocator`, so the set's lifetime and memory budget are controlled by the arena. Elements are sorted using `std::less<T>` — use `pma::UnorderedSet` when order does not matter and O(1) lookup is preferred.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type parameter | required — element type; must satisfy `std::less<T>` (strict-weak ordering) |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<T>` |

### Construction

```cpp
pma::Set<std::string> controlNames{allocator};
controlNames.insert("jawOpen");
controlNames.insert("eyeBlinkLeft");
```

### Relationships

- `pma::UnorderedSet` — hash-based alternative for O(1) lookup when order is irrelevant
- `pma::PolyAllocator` — the default allocator injected by this alias
- `std::set` — the underlying standard type this alias wraps

<!-- ink:api-end name="Set" -->

<!-- ink:api name="String" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::String<T, Allocator>`

Allocator-aware string type for the pma module, backed by `std::basic_string` with `PolyAllocator` by default.

### Why this exists

Using raw `std::string` bypasses the `pma` memory model entirely. This alias makes any character string in the `pma` namespace automatically use `PolyAllocator`, which supports arena allocation, memory tracking, and custom memory strategies without changing call-site code. Swap in a different allocator by providing the second template parameter; the string interface stays identical.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type parameter | required — character type (e.g. `char`, `wchar_t`) |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<T>`; supply a custom allocator to override arena strategy |

### Construction

```cpp
// Construct with an explicit allocator
pma::String<char> name{"RigLogic", allocator};

// Default-allocator construction
pma::String<char> label = "joint_01";
```

### Relationships

- `pma::Vector` — companion sequence container using the same PolyAllocator convention
- `pma::PolyAllocator` — the default allocator injected by this alias
- `std::basic_string` — the underlying standard type this alias wraps

<!-- ink:api-end name="String" -->

<!-- ink:api name="UnorderedMap" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::UnorderedMap<K, V, Allocator>`

Allocator-aware hash map for the pma module, backed by `std::unordered_map` with `PolyAllocator` by default.

### Why this exists

`std::unordered_map` scatters bucket arrays and node storage across the global heap, making arena-scoped cleanup expensive. This alias injects `PolyAllocator` so the entire map — hash buckets and key-value node storage — is tracked and freed through the `pma` memory model. Choose this over `pma::Map` when you need O(1) average-case lookup and do not require sorted key iteration.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `K` | template type parameter | required — key type; must support `std::hash<K>` and `std::equal_to<K>` |
| `V` | template type parameter | required — mapped value type |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<std::pair<const K, V>>` |

### Construction

```cpp
pma::UnorderedMap<std::string, float> blendWeights{allocator};
blendWeights.emplace("jawOpen", 0.75f);
blendWeights.emplace("eyeBlinkLeft", 0.0f);
float w = blendWeights.at("jawOpen"); // 0.75
```

### Relationships

- `pma::Map` — ordered alternative when sorted key iteration or range queries are needed
- `pma::UnorderedSet` — key-only hash-based companion type
- `pma::PolyAllocator` — the default allocator injected by this alias
- `std::unordered_map` — the underlying standard type this alias wraps

<!-- ink:api-end name="UnorderedMap" -->

<!-- ink:api name="UnorderedSet" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::UnorderedSet<T, Allocator>`

Allocator-aware hash set for the pma module, backed by `std::unordered_set` with `PolyAllocator` by default.

### Why this exists

`std::unordered_set` allocates bucket arrays and node storage on the global heap. This alias injects `PolyAllocator` so the entire structure — buckets and nodes — is managed through the `pma` memory model. Use this over `pma::Set` when element order is irrelevant and you need O(1) average-case lookup, insertion, and removal.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type parameter | required — element type; must be hashable via `std::hash<T>` and equality-comparable via `std::equal_to<T>` |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<T>` |

### Construction

```cpp
pma::UnorderedSet<std::uint16_t> visitedJoints{allocator};
visitedJoints.insert(7u);
visitedJoints.insert(12u);
bool alreadySeen = visitedJoints.count(7u) > 0;
```

### Relationships

- `pma::Set` — ordered alternative when sorted iteration or `std::less` comparison is required
- `pma::UnorderedMap` — key-value companion using the same hash-based strategy
- `pma::PolyAllocator` — the default allocator injected by this alias

<!-- ink:api-end name="UnorderedSet" -->

<!-- ink:api name="Vector" module="pma/TypeDefs" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::Vector<T, Allocator>`

Allocator-aware dynamic array for the pma module, backed by `std::vector` with `PolyAllocator` by default.

### Why this exists

Standard `std::vector` uses the global heap allocator unless explicitly parameterized. This alias injects `PolyAllocator` so every dynamic array in `pma` participates in the module's memory model — enabling arena allocation, lifetime scoping, and memory budget tracking with no call-site changes. It is also the building block for `pma::Matrix`, which layers a second vector over this alias.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type parameter | required — element type |
| `Allocator` | template type parameter | optional — defaults to `PolyAllocator<T>`; override to use a different arena or tracking strategy |

### Construction

```cpp
pma::Vector<float> weights{allocator};
weights.reserve(256);
weights.push_back(1.0f);
```

### Relationships

- `pma::Matrix` — 2D container built as `Vector<Vector<T>>` using this alias
- `pma::PolyAllocator` — the default allocator injected by this alias
- `pma::String` — sibling alias using the same PolyAllocator convention
- `std::vector` — the underlying standard type this alias wraps

<!-- ink:api-end name="Vector" -->
