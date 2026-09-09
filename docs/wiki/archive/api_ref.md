# Archived API Reference Entries


---

<!-- ink:archived id="f6a7b8c9" name="const_pointer" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="const_pointer" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `const_pointer`

Read-only pointer-to-element type, returned by the const overloads of `data()`, `cbegin()`, and `cend()`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `const_pointer` | `const T*` | Always a const pointer regardless of whether `T` is itself const. |

### Relationships

- `ArrayViewTraits<T>` — defines `const_pointer`; `ArrayView<T>` re-exports it for const-qualified access methods

<!-- ink:api-end name="const_pointer" -->


---

<!-- ink:archived id="d4e5f6a7" name="const_reference" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="const_reference" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `const_reference`

Read-only lvalue-reference type for a single element inside an `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `const_reference` | `const T&` | Always a const reference regardless of whether `T` itself is `const`. |

### Relationships

- `ArrayViewTraits<T>` — defines `const_reference`; `ArrayView<T>` re-exports it for the const `operator[]` overload

<!-- ink:api-end name="const_reference" -->


---

<!-- ink:archived id="b8c9d0e1" name="difference_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="difference_type" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `difference_type`

Signed integer type for pointer differences and iterator distance calculations in `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `difference_type` | `std::ptrdiff_t` | Platform-native signed difference type. Satisfies the `std::random_access_iterator` `difference_type` requirement. |

### Relationships

- `ArrayViewTraits<T>` — defines `difference_type`; re-exported by `ArrayView<T>` to satisfy iterator concept requirements

<!-- ink:api-end name="difference_type" -->


---

<!-- ink:archived id="e5f6a7b8" name="pointer" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="pointer" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pointer`

Pointer-to-element type for an `ArrayView`, used by `data()`, `begin()`, and `end()`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `pointer` | `T*` (primary) / `const T*` (const specialisation) | Mutable pointer in the primary template; read-only pointer when `T` is const-qualified. |

### Relationships

- `ArrayViewTraits<T>` — defines `pointer`; `ArrayView<T>` re-exports it for `data()`, `begin()`, and `end()` return types

<!-- ink:api-end name="pointer" -->


---

<!-- ink:archived id="c3d4e5f6" name="reference" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="reference" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `reference`

Lvalue-reference type for a single element inside an `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `reference` | `T&` (primary) / `const T&` (const specialisation) | Mutable reference in the primary template; read-only reference when `T` is const. |

### Relationships

- `ArrayViewTraits<T>` — defines `reference`; `ArrayView<T>` re-exports it via `operator[]` return type

<!-- ink:api-end name="reference" -->


---

<!-- ink:archived id="a7b8c9d0" name="size_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="size_type" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `size_type`

Unsigned integer type used for element counts and index arithmetic in `ArrayView`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `size_type` | `std::size_t` | Platform-native unsigned size type. Matches `size_type` in standard containers so `ArrayView` is range-compatible. |

### Relationships

- `ArrayViewTraits<T>` — defines `size_type`; `ArrayView<T>` uses it for `size()`, `slice()`, and `operator[]` parameters

<!-- ink:api-end name="size_type" -->


---

<!-- ink:archived id="b2c3d4e5" name="value_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/ArrayView.md" -->

<!-- ink:api name="value_type" module="arrayview/ArrayView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `value_type`

The underlying element type of an `ArrayView<T>`, stripped of pointer or reference decoration.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` (primary) / `const T` (const specialisation) | The bare element type. `const T` when `T` is a `const`-qualified type. |

### Relationships

- `ArrayViewTraits<T>` — defines `value_type`; `ArrayView<T>` re-exports it

<!-- ink:api-end name="value_type" -->


---

<!-- ink:archived id="7c9d5e2a" name="Base" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/arrayview/StringView.md" -->

<!-- ink:api name="Base" module="arrayview/StringView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Base`

Type alias for `ConstArrayView<char>`, declared inside `StringView` to name the immediate base class.

### Why this exists

`Base` is a `using` declaration that gives the `StringView` implementation a stable, readable name for its parent type. It allows `StringView` to inherit `ArrayView` constructors via `using Base::ArrayView` without repeating the full template instantiation. This is a class-internal alias and is not part of the public API surface — external code should use `ConstArrayView<char>` or `StringView` directly.

### Relationships

- `ConstArrayView<char>` — the aliased type; `Base` is exactly this type
- `StringView` — the class that declares this alias

<!-- ink:api-end name="Base" -->


---

<!-- ink:archived id="scan_file_./include/dna/JSONStreamReader.h/type" name="type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/dna/JSONStreamReader.md" -->

<!-- ink:api name="type" module="dna/JSONStreamReader" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type`

Type alias that resolves to `pma::FactoryCreate<dna::JSONStreamReader>`, the factory creator policy used by `pma` managed infrastructure to construct `JSONStreamReader` instances.

### Why this exists

The `pma` customization-point protocol requires a nested `type` alias inside each `DefaultInstanceCreator` specialization. This alias is the single token that `pma` reads when it needs to know how to construct a `T` — here, delegating to `JSONStreamReader::create()`.

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamReader>` — the enclosing policy struct that declares this alias
- `pma::FactoryCreate<dna::JSONStreamReader>` — the resolved type

<!-- ink:api-end name="type" -->


---

<!-- ink:archived id="scan_file_./include/dna/JSONStreamWriter.h/type" name="type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/dna/JSONStreamWriter.md" -->

<!-- ink:api name="type" module="dna/JSONStreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `using type = pma::FactoryCreate<dna::JSONStreamWriter>`

Member alias inside `DefaultInstanceCreator<dna::JSONStreamWriter>` that resolves to the concrete factory creator type.

### Why this exists

This `using type` alias is the conventional member name expected by PMA's traits system. PMA utilities look up `DefaultInstanceCreator<T>::type` to find the factory functor to invoke when constructing a `T`. This alias makes `JSONStreamWriter` compatible with those utilities without any additional boilerplate.

### Relationships

- `DefaultInstanceCreator<dna::JSONStreamWriter>` — the struct that contains this alias
- `pma::FactoryCreate<dna::JSONStreamWriter>` — the concrete type this resolves to

<!-- ink:api-end name="type" -->


---

<!-- ink:archived id="scan_file_./include/pma/PolyAllocator.h/rebind" name="rebind" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/pma/PolyAllocator.md" -->

<!-- ink:api name="rebind" module="pma/PolyAllocator" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `rebind<U>`

Produce a `PolyAllocator` for a different element type `U` while preserving the original alignment and `TDefaultMemoryResource` policy.

### Why this exists

Standard library containers such as `std::list` and `std::map` internally allocate node types that differ from the user-facing element type. The `rebind` mechanism is how the standard mandates that an allocator expose a way to clone itself for these internal types. Without `rebind`, containers that need to allocate auxiliary objects alongside `T` cannot use a typed allocator. `PolyAllocator::rebind<U>::other` satisfies that contract by keeping the same resource and alignment while changing only the element type.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `other` | `PolyAllocator<U, Alignment, TDefaultMemoryResource>` | required — the rebound allocator type for element type `U` |

### Relationships

- `PolyAllocator` — the outer class that declares this struct; `rebind` is always accessed as `PolyAllocator<T,...>::rebind<U>::other`
- `std::allocator_traits` — the standard trait layer that queries `rebind::other` on behalf of containers

<!-- ink:api-end name="rebind" -->


---

<!-- ink:archived id="d4e5f6a7" name="type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/riglogic/riglogic/RigInstance.md" -->

<!-- ink:api name="type" module="riglogic/riglogic/RigInstance" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type` (trait member alias in `pma::DefaultInstanceCreator<rl4::RigInstance>` and `pma::DefaultInstanceDestroyer<rl4::RigInstance>`)

The conventional C++ traits `type` member that names the selected creation or destruction policy for `rl4::RigInstance` within the `pma` framework.

### Why this exists

C++ traits structs expose their result through a nested `type` member by convention (`std::type_traits` style). Both `DefaultInstanceCreator<rl4::RigInstance>::type` (`FactoryCreate<rl4::RigInstance>`) and `DefaultInstanceDestroyer<rl4::RigInstance>::type` (`FactoryDestroy<rl4::RigInstance>`) follow this pattern so the `pma` framework can uniformly extract the policy via `typename Traits::type`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` (creator) | `FactoryCreate<rl4::RigInstance>` | Creation policy; used by `pma` to allocate `RigInstance` via the factory |
| `type` (destroyer) | `FactoryDestroy<rl4::RigInstance>` | Destruction policy; used by `pma` to release `RigInstance` via the factory |

<!-- ink:api-end name="type" -->


---

<!-- ink:archived id="scan_file_./include/riglogic/riglogic/RigLogic.h/type" name="type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/riglogic/riglogic/RigLogic.md" -->

<!-- ink:api name="type" module="riglogic/riglogic/RigLogic" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type`

Type alias member within the `DefaultInstanceCreator` and `DefaultInstanceDestroyer` specializations for `rl4::RigLogic`; resolves to the factory strategy type used by the `pma` ownership infrastructure.

### Relationships

- `DefaultInstanceCreator<rl4::RigLogic>` — defines `type = FactoryCreate<rl4::RigLogic>`
- `DefaultInstanceDestroyer<rl4::RigLogic>` — defines `type = FactoryDestroy<rl4::RigLogic>`

<!-- ink:api-end name="type" -->


---

<!-- ink:archived id="c3d4e5f6" name="Type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/status/Provider.md" -->

<!-- ink:api name="Type" module="status/Provider" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Make<0, Offset, Is...>::Type`

Nested type alias that exposes the fully assembled `ISeq` when `Make`'s recursion terminates.

### Why this exists

C++ template metaprogramming requires a metafunction to expose its result through a nested type alias so callers can retrieve it with `typename Make<N, O>::Type`. `Type` is the terminus of `Make`'s recursion: when `Size` reaches zero, the accumulated pack `Is...` is exactly `[Offset, Offset+Size)`, and `Type` names the resulting `ISeq`. The `MakeISeq` alias exists specifically to hide this `::Type` access from call sites.

### Relationships

- `Make` — the metafunction that defines this alias in its base case
- `ISeq` — the concrete type this alias resolves to
- `MakeISeq` — `using MakeISeq = typename Make<Size, Offset>::Type` — the preferred accessor

<!-- ink:api-end name="Type" -->


---

<!-- ink:archived id="scan_file_./include/tdm/Ang.h/value_type" name="value_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/tdm/Ang.md" -->

<!-- ink:api name="value_type" module="tdm/Ang" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `value_type`

The numeric scalar type used to store the angle magnitude inside an `ang<T, TUnit>` instance.

### Why this exists

`value_type` exposes the template parameter `T` as a named member type alias following the standard library convention (`std::vector<T>::value_type`, etc.). Generic code that operates on `ang` specializations can use `ang<T, TUnit>::value_type` to inspect or rebind the underlying scalar type without hard-coding the template argument.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | alias for `T` | required — the scalar floating-point type (e.g., `float`, `double`) used to store the angle's numeric magnitude |

### Relationships

- `ang<T, TUnit>` — the containing struct that declares this alias
- `value` — the `ang` member whose type is `value_type`

<!-- ink:api-end name="value_type" -->


---

<!-- ink:archived id="scan_file_./include/tdm/Mat.h/column_type" name="column_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/tdm/Mat.md" -->

<!-- ink:api name="column_type" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>::column_type`

Type alias for a single column of a `mat<R, C, T>` matrix — a `vec<R, T>` vector with R elements.

### Why this exists

Names the column vector type with R components, which is distinct from `row_type` in non-square matrices. The `rows()` static method returns `column_type::dimensions()` (R), making explicit that the row count equals the column vector height. Generic code receiving a column vector from matrix operations can use this alias to determine the expected type without inspecting the R template parameter directly.

### Relationships

- `row_type` — `vec<C, value_type>`: the corresponding row vector type.
- `from_columns` — factory method that takes `C` arguments of this type.

<!-- ink:api-end name="column_type" -->


---

<!-- ink:archived id="scan_file_./include/tdm/Mat.h/row_type" name="row_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/tdm/Mat.md" -->

<!-- ink:api name="row_type" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>::row_type`

Type alias for a single row of a `mat<R, C, T>` matrix — a `vec<C, T>` vector with C elements.

### Why this exists

Names the concrete row vector type so that generic algorithms and `from_rows` factory callers can construct row arguments at the correct width without repeating the column dimension. The internal `values[R]` storage array is an array of `row_type`, so iterating rows operates directly on this type.

### Relationships

- `column_type` — `vec<R, value_type>`: the corresponding column vector type.
- `from_rows` — factory method that takes `R` arguments of this type.

<!-- ink:api-end name="row_type" -->


---

<!-- ink:archived id="scan_file_./include/tdm/Mat.h/value_type" name="value_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/tdm/Mat.md" -->

<!-- ink:api name="value_type" module="tdm/Mat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `mat<R, C, T>::value_type`

Type alias for the scalar element type `T` of a `mat<R, C, T>` matrix.

### Why this exists

Follows the C++ standard container convention so that generic code can write `typename Mat::value_type` to recover the element type without knowing the concrete instantiation. This enables template utilities to operate uniformly on `mat`, `vec`, and standard containers.

### Relationships

- `row_type` — `vec<C, value_type>`: each row stores elements of this type.
- `column_type` — `vec<R, value_type>`: each column stores elements of this type.

<!-- ink:api-end name="value_type" -->


---

<!-- ink:archived id="api_scan" name="value_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/tdm/Quat.md" -->

<!-- ink:api name="value_type" module="tdm/Quat" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `quat<T>::value_type`

Type alias that exposes the scalar component type of a `quat<T>` specialization.

### Why this exists

Following the C++ named requirement convention for numeric types, `value_type` lets generic code query the underlying scalar type of a `quat` without repeating the template argument. Use `quat<T>::value_type` in template contexts where you have a `quat` type but not the original `T`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` | required — aliases the template parameter `T` of the enclosing `quat<T>` |

### Relationships

- `quat<T>` — the enclosing type that declares this alias

<!-- ink:api-end name="value_type" -->


---

<!-- ink:archived id="scan_file_./include/tdm/Vec.h/value_type" name="value_type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/tdm/Vec.md" -->

<!-- ink:api name="value_type" module="tdm/Vec" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `value_type`

Member type alias that exposes the element type `T` of a `vec<L, T>` instantiation for use in generic template code.

### Why this exists

`value_type` follows the standard C++ named-requirement convention (same as `std::vector<T>::value_type`), allowing generic algorithms and traits to query the scalar element type of any `vec<L, T>` without knowing the full specialization. This makes `vec` compatible with standard type-trait patterns such as `std::is_same<typename SomeVec::value_type, float>` and template deduction helpers.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `value_type` | `T` | The scalar element type of this `vec` instantiation (e.g., `float` for `vec<3, float>`) |

### Relationships

- `vec<L, T>` — the enclosing struct that declares this alias
- `values[L]` — the underlying storage array whose element type is `value_type`

<!-- ink:api-end name="value_type" -->


---

<!-- ink:archived id="scan_file_./include/trio/streams/MemoryMappedFileStream.h/type" name="type" archived="2026-07-31" reason="not_api_surface" was="docs/wiki/api/trio/streams/MemoryMappedFileStream.md" -->

<!-- ink:api name="type" module="trio/streams/MemoryMappedFileStream" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `type` (pma traits alias)

The resolved factory or destroyer policy type carried by `DefaultInstanceCreator` and `DefaultInstanceDestroyer` trait specializations for `MemoryMappedFileStream`.

### Why this exists

In the `pma` traits system, the member alias `using type = ...` is the conventional way to expose the selected policy to generic code (e.g., `typename DefaultInstanceCreator<T>::type`). For `MemoryMappedFileStream`, this resolves to `FactoryCreate<trio::MemoryMappedFileStream>` (in the creator) or `FactoryDestroy<trio::MemoryMappedFileStream>` (in the destroyer). Client code that accesses `::type` receives the concrete factory or destructor functor without needing to name it explicitly.

### Relationships

- `DefaultInstanceCreator<trio::MemoryMappedFileStream>` — the creator trait that holds `type = FactoryCreate<...>`
- `DefaultInstanceDestroyer<trio::MemoryMappedFileStream>` — the destroyer trait that holds `type = FactoryDestroy<...>`

<!-- ink:api-end name="type" -->
