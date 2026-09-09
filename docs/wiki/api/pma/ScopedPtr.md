# API Reference — `pma/ScopedPtr`

---

<!-- ink:api name="complete_type_checker" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `complete_type_checker` (local alias inside `Delete::operator()`)

A local array-type alias (`char[sizeof(T) ? 1 : -1]`) used purely to force a compile-time error when `T` is incomplete at the point `Delete<T>` deletes it.

### Why this exists

Calling `delete` on an incomplete type is undefined behavior and compilers often accept it silently; `complete_type_checker` forces `sizeof(T)` to be evaluated, which fails to compile for an incomplete `T`, turning a silent UB risk into a hard compile error. It has no runtime effect — `static_cast<void>(sizeof(complete_type_checker))` discards the value, keeping only the compile-time check.

<!-- ink:api-end name="complete_type_checker" -->

<!-- ink:api name="DefaultInstanceCreator" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class T> struct DefaultInstanceCreator { using type = New<T>; }`

Select `New<T>` as the default creator policy when no custom creator is specified for `ScopedPtr`/`makeScoped`.

### When to use this

You don't call this directly — `makeScoped<T>(args...)` uses it internally to pick `New<T>` as the creator when the type has an ordinary constructor. Provide a custom creator (e.g., `FactoryCreate`) only when `T` needs factory-based construction instead.

### Returns

`type` — alias for `New<T>`, the creator used by `makeScoped` unless a custom creator template is supplied.

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class T> struct DefaultInstanceDestroyer { using type = Delete<T>; }`

Select `Delete<T>` as the default destroyer policy when no custom destroyer is specified for `ScopedPtr`/`makeScoped`.

### When to use this

Used internally as `ScopedPtr`'s default `TDestroyer` template argument, so a plain `ScopedPtr<T>` calls `delete` on destruction without any extra typing. Override it only when `T` requires factory-based or array destruction (`FactoryDestroy`, `Delete<T[]>`).

### Returns

`type` — alias for `Delete<T>`, the destroyer `ScopedPtr` inherits from by default.

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="Delete" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<class T, class B = T> struct Delete`

The default destruction policy for `ScopedPtr`, releasing a pointer allocated by `New` via `delete`.

### When to use this

Use this as the destroyer policy paired with `New` whenever `ScopedPtr` owns a heap object allocated with plain `new`. It guards against deleting an incomplete type at compile time rather than allowing undefined behavior at runtime.

### Method groups

| Group | Methods |
|-------|---------|
| Destruction | operator() (calls `delete ptr` after a completeness check) |

### Example

```cpp
pma::Delete<dna::VersionInfo> destroyer;
destroyer(obj); // equivalent to delete obj, with a compile-time completeness guard
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `ptr` | `B*` | required — pointer to the object to delete. |

### Raises

- Compile error via `complete_type_checker` — triggered if `T` is an incomplete type at the point of deletion, since calling `delete` on an incomplete type is undefined behavior. Ensure `T`'s full definition is visible wherever `Delete<T>` is instantiated.

### Watch out for

- `Delete<T[]>` is a separate specialization that calls `delete[]` instead — using the non-array `Delete` on an array-allocated pointer is incorrect.
- `Delete<T>::operator()` must be instantiated in a translation unit where `T` is fully defined. If `T` is forward-declared in the header that owns `ScopedPtr<T>`, move the `ScopedPtr` destructor definition to a `.cpp` where `T` is complete. The compile error from `complete_type_checker` signals exactly this problem.

### Relationships

- `New` — *the matching creator policy.*
- `complete_type_checker` — *the compile-time completeness guard used internally.*

<!-- ink:api-end name="Delete" -->

<!-- ink:api name="destroyer_type" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `using destroyer_type = TDestroyer;`

Public alias on `ScopedPtr` for its `TDestroyer` template parameter — the policy type invoked to release the owned pointer.

### Why this exists

Exposing `destroyer_type` lets calling code (and `ScopedPtr`'s own converting move constructor) refer to the exact destroyer policy in use without re-specifying `TDestroyer`, which matters when moving between `ScopedPtr` instances with related but distinct destroyer types.

<!-- ink:api-end name="destroyer_type" -->

<!-- ink:api name="FactoryCreate" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<class T, class B = T> struct FactoryCreate { B* operator()(Args&&... args); }`

Create an object through a type's static `create` factory method instead of `new`, for types that manage their own construction.

### When to use this

Use `FactoryCreate` as the creator policy for `ScopedPtr`/`makeScoped` when `T` exposes a static `create(...)` factory instead of a public constructor — common for types that need to control allocation or return a base-class pointer. Pair it with `FactoryDestroy` so lifetime is handled through the same factory interface, not `delete`.

### Example

```cpp
// Impl exposes static Impl* create(Args...) / static void destroy(Impl*)
pma::ScopedPtr<Impl, FactoryDestroy<Impl>> instance{FactoryCreate<Impl>{}(regionSize, upstream)};
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `args` | `Args&&...` | required — forwarded verbatim to `T::create`. |

### Returns

`B*` — pointer returned by `T::create`, typed as the base `B` (defaults to `T`).

<!-- ink:api-end name="FactoryCreate" -->

<!-- ink:api name="FactoryDestroy" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<class T, class B = T> struct FactoryDestroy { void operator()(B* ptr); }`

Destroy an object through a type's static `destroy` method instead of `delete`, matching a `FactoryCreate`-constructed instance.

### When to use this

Use this as the destroyer policy for `ScopedPtr`/`makeScoped` whenever the owned type was built with `FactoryCreate` — the two must be paired so the object is torn down through the same factory API it was created with, not a raw `delete`.

### Example

```cpp
// Impl exposes static Impl* create(Args...) / static void destroy(Impl*)
pma::ScopedPtr<Impl, FactoryDestroy<Impl>> instance{FactoryCreate<Impl>{}(regionSize, upstream)};
// destructor calls FactoryDestroy<Impl>{}(ptr) -> Impl::destroy(ptr)
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `ptr` | `B*` | required — pointer to destroy; cast to `T*` before calling `T::destroy`. |

### Watch out for

- The `static_cast<T*>(ptr)` in `operator()` means `B*` must be safely downcasted to `T*`. If `B` is a virtual base or an unrelated type, the cast produces undefined behavior. Ensure the pointer was originally a `T*` before assigning it to a `ScopedPtr<B, FactoryDestroy<T, B>>`.

<!-- ink:api-end name="FactoryDestroy" -->

<!-- ink:api name="inspect" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<typename U> struct inspect { using element_type = U; using pointer_type = element_type*; using is_array = std::false_type; }` (with a `U[]` specialization)

A private trait used by `ScopedPtr` to derive its `pointer`, `element_type`, and `is_array` members from the template argument `T`.

### Why this exists

Because `ScopedPtr<T>` can be instantiated with either a scalar type or an array type (`T[]`), `inspect` centralizes the logic for picking the right pointer/element types and whether array-specific operations (like `operator[]`) should be enabled, via its `U[]` partial specialization.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `element_type` | `U` | The underlying element type — `U` for scalars, or the array element type for `U[]`. |
| `pointer_type` | `element_type*` | The pointer type `ScopedPtr::pointer` is defined as. |
| `is_array` | `std::false_type` / `std::true_type` | Tag type used with `std::enable_if` to select array (`operator[]`) vs. scalar (`operator*`) access. |

<!-- ink:api-end name="inspect" -->

<!-- ink:api name="is_array" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `using is_array = std::false_type;` (or `std::true_type` for the `U[]` specialization)

Compile-time tag member of `inspect<U>` indicating whether `T` is an array type.

### Why this exists

`ScopedPtr` uses `is_array` with `std::enable_if` to enable `operator[]` only for array-typed instantiations and `operator*` only for scalar instantiations, so calling the wrong accessor is a compile error rather than a runtime bug.

<!-- ink:api-end name="is_array" -->

<!-- ink:api name="makeScoped" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-06-10" api_kind="callable" -->

## `pma::makeScoped<T>(args...)` / `pma::makeScoped<T, TCreator, TDestroyer>(args...)`

Construct a `ScopedPtr<T>` by invoking a creator policy and binding its matched destroyer — the preferred way to create any `ScopedPtr`.

### When to use this

Use `makeScoped` instead of constructing `ScopedPtr` directly; it deduces the policy types, checks type compatibility via `static_assert`, and keeps call sites concise. Provide explicit `TCreator`/`TDestroyer` template-template arguments when the type uses a static factory (`FactoryCreate` / `FactoryDestroy`); omit them entirely when `new`/`delete` suffices.

### Example

```cpp
// Simplest form — uses DefaultInstanceCreator/DefaultInstanceDestroyer
auto mesh = pma::makeScoped<RigMesh>(numVertices, numFaces);
// mesh is ScopedPtr<RigMesh, Delete<RigMesh>>

// Factory-managed type
auto reader = pma::makeScoped<dna::StreamReader,
                              pma::FactoryCreate,
                              pma::FactoryDestroy>(stream, layer);
// calls dna::StreamReader::create(stream, layer)
// destroyed via dna::StreamReader::destroy()

// Explicit creator/destroyer types (non-template-template form)
using Creator  = pma::FactoryCreate<dna::StreamReader, dna::Reader>;
using Destroyer = pma::FactoryDestroy<dna::StreamReader, dna::Reader>;
auto reader2 = pma::makeScoped<dna::StreamReader, Creator, Destroyer>(stream, layer);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `T` | template type param | The concrete type to create. |
| `TCreator` | template type param | optional — Creator policy or template-template arg; defaults to `DefaultInstanceCreator<T>::type`. |
| `TDestroyer` | template type param | optional — Destroyer policy or template-template arg; defaults to `DefaultInstanceDestroyer<T>::type`. |
| `args` | `Args&&...` | Arguments forwarded to the creator's `operator()`. |

### Returns

`ScopedPtr<Base, TDestroyer>` — an owning smart pointer whose `Base` type is deduced from the creator's return type. For the simplest overload, `Base == T`.

### Watch out for

- The full-explicit overload (`<T, TCreator, TDestroyer>`) fires a `static_assert` if `T` is not the same as, a base of, or pointer-convertible to `Base` (the type the creator actually returns). The error message is "Incompatible types." — check that `TCreator{}(args...)` returns a pointer convertible to `Base*`.
- When using template-template arguments (`makeScoped<T, FactoryCreate, FactoryDestroy>`), both templates must accept a single type parameter. Custom policy templates with additional parameters require the explicit `<T, Creator, Destroyer>` form.

<!-- ink:api-end name="makeScoped" -->

<!-- ink:api name="New" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `template<class T, class B = T> struct New`

A default construction policy for `ScopedPtr` that allocates an instance with `new` and forwards constructor arguments.

### When to use this

Use this as the default (or explicit) creator policy for `ScopedPtr` when the pointee should be heap-allocated with plain `new`. Pair it with `Delete` as the matching destroyer so the pointer is released with `delete`, not some other deallocation strategy.

### Method groups

| Group | Methods |
|-------|---------|
| Construction | operator() (variadic, forwards args to `new T{...}`) |

### Example

```cpp
pma::New<dna::VersionInfo> creator;
auto* obj = creator(); // equivalent to new dna::VersionInfo{}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `args` | `Args&&...` | optional — forwarded to `T`'s constructor. |

### Returns

`B*` — pointer to the newly constructed `T`, returned as base type `B`.

### Relationships

- `Delete` — *the matching destroyer policy that releases what `New` allocates.*
- `New<T[]>` — *the array specialization, using `new T[size]{}` instead.*
- `ScopedPtr` — *the primary consumer of this creator policy.*

<!-- ink:api-end name="New" -->

<!-- ink:api name="pointer_type" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `using pointer_type = element_type*;`

Member typedef of the private `inspect<U>` trait struct, used internally by `ScopedPtr` to derive its public `pointer` alias.

### Why this exists

Separating `pointer_type` from `element_type` inside `inspect` lets `ScopedPtr` compute the correct pointer type uniformly whether `T` is a scalar or an array element, without repeating the `*`-decoration logic at the `ScopedPtr` level.

<!-- ink:api-end name="pointer_type" -->

<!-- ink:api name="ScopedPtr" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class T, class TDestroyer = typename DefaultInstanceDestroyer<T>::type> class ScopedPtr : private TDestroyer`

Takes ownership over a given pointer and handles its lifetime, similar in spirit to `std::unique_ptr` but with pluggable, stateful or stateless destroyer policies.

### Why this exists

`ScopedPtr` inherits its destroyer type rather than storing it as a member, so stateless lifetime managers (like `Delete<T>` or `FactoryDestroy<T>`) add zero size overhead. A dedicated constructor also accepts a destroyer *instance*, so stateful destroyers — lambdas with captures, or anything holding extra state — work too, which a plain `unique_ptr`-style design using only a type parameter would not support as cleanly.

### Construction

```cpp
// Default destroyer (delete)
pma::ScopedPtr<Impl> owned{new Impl{}};

// Custom destroyer via makeScoped
auto arena = pma::makeScoped<ArenaMemoryResource, FactoryCreate<ArenaMemoryResource>, FactoryDestroy<ArenaMemoryResource>>(regionSize, growthFactor, upstream);
```

### Relationships

- `makeScoped` — *factory function that constructs a `ScopedPtr` with the right creator/destroyer pair.*
- `FactoryCreate` / `FactoryDestroy` — *destroyer/creator policies for factory-managed types.*
- `DefaultInstanceCreator` / `DefaultInstanceDestroyer` — *default `New`/`Delete` policies used when none are specified.*

### Constraints

- Not copyable — copy constructor and copy assignment are deleted.
- Move-only: supports move construction/assignment, including converting moves between compatible `T`/`TDestroyer` pairs.

### Watch out for

- `ScopedPtr` inherits `TDestroyer` privately, so the destroyer is invoked directly — do not pass a destroyer type whose `operator()` has preconditions that require it to be initialized after construction.
- Moving from a `ScopedPtr<U, UDestroyer>` to `ScopedPtr<T, TDestroyer>` requires that `U*` is implicitly convertible to `T*`. The static assertion in `makeScoped` checks this; the cross-type move constructor does not independently verify it.
- After `release()` the caller owns the pointer and is responsible for its destruction — the `ScopedPtr` no longer manages it.

<!-- ink:api-end name="ScopedPtr" -->

<!-- ink:api name="TCreator" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class T, class TCreator, class TDestroyer, typename... Args> ScopedPtr<Base, TDestroyer> makeScoped(Args&&... args)`

`TCreator` is the template parameter of `makeScoped` naming the callable invoked as `TCreator{}(args...)` to construct the object that ends up owned by the returned `ScopedPtr`.

### Why this exists

Making the creator a separate template parameter from the destroyer lets `makeScoped` pair any construction strategy (a plain `new`-based `New<T>`, or a factory-based `FactoryCreate<T>`) with any matching cleanup strategy, and lets `makeScoped` deduce the constructed `Base` type from `TCreator`'s return type via `decltype`.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `TCreator` | template type parameter | required — callable type; `TCreator{}(args...)` must return a pointer convertible to `Base*`. |

<!-- ink:api-end name="TCreator" -->

<!-- ink:api name="TCreatorTemplate" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class T, template<class...> class TCreatorTemplate, template<class...> class TDestroyerTemplate, typename... Args> ScopedPtr<T, TDestroyerTemplate<T>> makeScoped(Args&&... args)`

`TCreatorTemplate` is a template-template parameter — a template like `DefaultInstanceCreator` — that this `makeScoped` overload instantiates with `T` to obtain the concrete creator type.

### When to use this

This overload is selected when you want to pick creator/destroyer *templates* (e.g., `DefaultInstanceCreator`, a custom factory-template) rather than already-instantiated creator/destroyer types, letting `makeScoped<T>(args...)` derive both from `T` in one step.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `TCreatorTemplate` | template-template parameter | required — instantiated as `TCreatorTemplate<T>` to obtain the creator type. |

<!-- ink:api-end name="TCreatorTemplate" -->

<!-- ink:api name="TDestroyer" module="pma/ScopedPtr" last_commit="api_scan" updated="2026-09-09" api_kind="data_shape" -->

## `template<class T, class TCreator, class TDestroyer, typename... Args> ScopedPtr<Base, TDestroyer> makeScoped(Args&&... args)`

`TDestroyer` is the template parameter naming the destroyer policy that `makeScoped` bakes into the returned `ScopedPtr<Base, TDestroyer>`'s type.

### Why this exists

Keeping `TDestroyer` as an explicit template parameter (rather than deriving it) lets `makeScoped` be called with any creator/destroyer pair — including mismatched ones for advanced cases — while still producing a `ScopedPtr` whose static type correctly reflects the destroyer that will run.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `TDestroyer` | template type parameter | required — destroyer policy; becomes the second template argument of the returned `ScopedPtr`. |

<!-- ink:api-end name="TDestroyer" -->

