# API Reference — `pma/ScopedPtr`

---

<!-- ink:api name="DefaultInstanceCreator" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::DefaultInstanceCreator<T>`

Type trait that resolves the default creator policy for `T`. Evaluates to `New<T>` out of the box; specialize it to override the default for a specific type.

### Why this exists

`makeScoped<T>(args...)` needs to know which creator to use without requiring the caller to spell it out every time. `DefaultInstanceCreator<T>::type` provides that default in one specializable place. If a type later switches from `operator new` to a factory pattern, specializing this trait — rather than updating every call site — keeps all `makeScoped<T>()` uses correct automatically.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `New<T>` | The resolved creator type. Access via `DefaultInstanceCreator<T>::type`. |

### Construction

```cpp
// Consume via trait lookup (typical)
using Creator = pma::DefaultInstanceCreator<MyClass>::type;  // == pma::New<MyClass>
Creator creator;
MyClass* obj = creator(arg1, arg2);

// Specialize to override the default for a given type:
namespace pma {
template<>
struct DefaultInstanceCreator<MyPooledClass> {
    using type = FactoryCreate<MyPooledClass>;
};
}
```

### Relationships

- `DefaultInstanceDestroyer<T>` — the parallel trait for the destroyer side
- `New<T>` — the default resolved type
- `makeScoped` — queries this trait when no explicit creator is provided

<!-- ink:api-end name="DefaultInstanceCreator" -->

<!-- ink:api name="DefaultInstanceDestroyer" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::DefaultInstanceDestroyer<T>`

Type trait that resolves the default destroyer policy for `T`. Evaluates to `Delete<T>` out of the box; specialize it to change the destruction path for a specific type without touching its call sites.

### Why this exists

`ScopedPtr<T>` uses `DefaultInstanceDestroyer<T>::type` as its default `TDestroyer`. This means you can globally change how `T` is destroyed — switching from `delete` to a factory `T::destroy()` call — by specializing this one trait. All `ScopedPtr<T>` and `makeScoped<T>()` usages then pick up the correct destroyer automatically.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `type` | `Delete<T>` | The resolved destroyer type. Access via `DefaultInstanceDestroyer<T>::type`. |

### Construction

```cpp
// Consume via trait lookup:
using Destroyer = pma::DefaultInstanceDestroyer<MyClass>::type;  // == pma::Delete<MyClass>

// Specialize to override:
namespace pma {
template<>
struct DefaultInstanceDestroyer<MyPooledClass> {
    using type = FactoryDestroy<MyPooledClass>;
};
}
// Now ScopedPtr<MyPooledClass> automatically calls MyPooledClass::destroy()
```

### Relationships

- `DefaultInstanceCreator<T>` — the parallel trait for the creator side
- `Delete<T>` — the default resolved type
- `ScopedPtr` — reads this trait as its default `TDestroyer` parameter

<!-- ink:api-end name="DefaultInstanceDestroyer" -->

<!-- ink:api name="Delete" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::Delete<T, B>`

Deallocate a `B*` via `operator delete`, enforcing at compile time that `T` is a complete type. The array specialization `Delete<T[]>` uses `delete[]`.

### Why this exists

Calling `delete` on an incomplete type is undefined behavior in C++, but the compiler only warns — it does not error. `Delete` uses a `complete_type_checker` typedef (`char[sizeof(T) ? 1 : -1]`) that produces a compile error for incomplete types, making the UB impossible to reach silently. This is the safety contract that makes `ScopedPtr` safe to use with forward-declared types as long as the destroyer is instantiated where `T` is complete.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type param | The concrete type being destroyed. Must be a complete type at the point where `Delete<T>::operator()` is instantiated. |
| `B` | template type param | The base pointer type accepted; defaults to `T`. |

### Construction

```cpp
// Single-object form — called by ScopedPtr automatically
pma::Delete<MyClass> destroyer;
destroyer(ptr);   // equivalent to: delete ptr (with completeness check)

// Array form
pma::Delete<MyClass[]> array_destroyer;
array_destroyer(arr);   // equivalent to: delete[] arr
```

### Watch out for

- `Delete<T>::operator()` must be instantiated in a translation unit where `T` is fully defined. If `T` is forward-declared in the header that owns `ScopedPtr<T>`, move the `ScopedPtr` destructor definition to a `.cpp` where `T` is complete. The compile error from `complete_type_checker` signals exactly this problem.

### Relationships

- `New<T>` — the companion creator; `Delete<T>` is the corresponding destroyer
- `DefaultInstanceDestroyer<T>` — trait that resolves to `Delete<T>`
- `ScopedPtr` — uses `Delete<T>` as its default `TDestroyer`
- `FactoryDestroy<T>` — alternative destroyer for types that use a static `T::destroy()` factory pattern

<!-- ink:api-end name="Delete" -->

<!-- ink:api name="FactoryCreate" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::FactoryCreate<T, B>`

Create a `T` by calling `T::create(args...)` and return the result as `B*`. Use this as the creator policy for `ScopedPtr` when `T` controls its own allocation via a static factory method.

### Why this exists

Many C++ types in this codebase (and plugin/module boundaries in general) expose a `static T* create(…)` / `static void destroy(T*)` pair rather than public constructors, because they manage their own memory allocator or need to return a different concrete subtype. `FactoryCreate` adapts that pattern to the `ScopedPtr` policy interface so that factory-managed objects participate in the same scoped lifetime system as heap-allocated objects.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type param | The concrete type whose static `create()` method is called. Must have `static B* T::create(Args...)`. |
| `B` | template type param | The base type returned; defaults to `T`. |

### Construction

```cpp
// Pair with FactoryDestroy and use via makeScoped:
auto ptr = pma::makeScoped<MyService, pma::FactoryCreate, pma::FactoryDestroy>();
// ptr is ScopedPtr<MyService, FactoryDestroy<MyService>>
// constructed via MyService::create(), destroyed via MyService::destroy()
```

### Relationships

- `FactoryDestroy<T>` — the mandatory companion destroyer; always pair these two
- `New<T>` — the alternative creator for types using `operator new`
- `makeScoped` — accepts `FactoryCreate` and `FactoryDestroy` as template-template args

<!-- ink:api-end name="FactoryCreate" -->

<!-- ink:api name="FactoryDestroy" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::FactoryDestroy<T, B>`

Release a `B*` by calling `T::destroy(static_cast<T*>(ptr))`. Always pair with `FactoryCreate<T>` when owning objects that use a static factory allocation pattern.

### Why this exists

When a type owns its memory via `T::create()` / `T::destroy()`, calling `delete` on the pointer would bypass the type's deallocation path — potentially leaking allocator state or custom heap resources. `FactoryDestroy` enforces that the correct destruction path is always taken, and it does so with zero overhead when used as a stateless policy in `ScopedPtr`.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type param | The concrete type whose static `destroy()` method is called. Must have `static void T::destroy(T*)`. |
| `B` | template type param | The base pointer type accepted; defaults to `T`. The cast `static_cast<T*>(ptr)` is applied before dispatch. |

### Construction

```cpp
pma::FactoryDestroy<MyService> destroyer;
destroyer(ptr);   // calls MyService::destroy(static_cast<MyService*>(ptr))

// Typical usage — let makeScoped wire it automatically:
auto svc = pma::makeScoped<MyService, pma::FactoryCreate, pma::FactoryDestroy>();
```

### Watch out for

- The `static_cast<T*>(ptr)` in `operator()` means `B*` must be safely downcasted to `T*`. If `B` is a virtual base or an unrelated type, the cast produces undefined behavior. Ensure the pointer was originally a `T*` before assigning it to a `ScopedPtr<B, FactoryDestroy<T, B>>`.

### Relationships

- `FactoryCreate<T>` — always pair together
- `Delete<T>` — the alternative for types using `operator delete`
- `ScopedPtr` — stores this as its `TDestroyer` policy

<!-- ink:api-end name="FactoryDestroy" -->

<!-- ink:api name="makeScoped" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

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

<!-- ink:api name="New" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `pma::New<T, B>`

Allocate a `T` instance on the heap and return it as `B*`. Used as the default creator policy for `ScopedPtr` and `makeScoped`.

### Why this exists

`ScopedPtr` separates object creation from lifetime management, letting you swap either half independently. `New` is the default creation half — it calls `operator new T{args...}` and is zero-cost when used as a stateless policy. The array specialization `New<T[]>` allocates with `new T[size]{}` (value-initialized), which is the safe default for raw arrays.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `T` | template type param | The concrete type to allocate. |
| `B` | template type param | The base type returned; defaults to `T`. Allows returning a base-class pointer from a derived allocation. |

### Construction

```cpp
// Single-object form
pma::New<MyClass> creator;
MyClass* obj = creator(arg1, arg2);  // calls new MyClass{arg1, arg2}

// Array form
pma::New<MyClass[]> array_creator;
MyClass* arr = array_creator(16);   // calls new MyClass[16]{} (value-initialized)
```

### Relationships

- `Delete<T>` — the companion destroyer; used alongside `New<T>` as the default destroyer policy
- `DefaultInstanceCreator<T>` — trait that resolves to `New<T>`; use when selecting the creator via traits
- `ScopedPtr` — consumes `New` as its `TDestroyer`-paired creator via `makeScoped`
- `makeScoped` — factory that instantiates `New` automatically when no creator is specified

<!-- ink:api-end name="New" -->

<!-- ink:api name="ScopedPtr" module="pma/ScopedPtr" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `pma::ScopedPtr<T, TDestroyer>`

Take ownership of a raw pointer and automatically destroy it via `TDestroyer` when the `ScopedPtr` goes out of scope. Prefer `makeScoped<T>(args...)` over constructing `ScopedPtr` directly.

### When to use this

Use `ScopedPtr` when you own a raw pointer that must be released through a specific destructor policy — particularly for types with a static `T::destroy()` factory (use `FactoryDestroy`) or custom allocators. Prefer `std::unique_ptr` for simple heap ownership with no policy requirements; reach for `ScopedPtr` when you need the swappable creator/destroyer policy system.

### Description

`ScopedPtr` inherits from `TDestroyer` privately, making stateless destroyer policies (like `Delete<T>` or `FactoryDestroy<T>`) zero-cost via the empty base optimization. For stateful destroyers (lambdas with captures, custom allocator instances), pass the destroyer instance to the `ScopedPtr(pointer, destroyer_type&&)` constructor. Copy construction and copy assignment are deleted; only move semantics are supported. Cross-type moves are supported when the source pointer is convertible to the target pointer type.

### Example

```cpp
// Simple heap-allocated object (Delete<MyClass> destroyer is zero-cost)
auto ptr = pma::makeScoped<MyClass>(ctorArg1, ctorArg2);
// ptr is ScopedPtr<MyClass, Delete<MyClass>>
ptr->doWork();
// destroyed automatically on scope exit

// Factory-managed type
auto svc = pma::makeScoped<MyService, pma::FactoryCreate, pma::FactoryDestroy>();
// calls MyService::create(), destroys via MyService::destroy()

// Array form
auto buf = pma::makeScoped<float[]>(1024);
buf[0] = 1.0f;

// Stateful destroyer (e.g., pool allocator)
MyAllocator alloc;
auto pooled = pma::ScopedPtr<MyClass, MyAllocator>(
    alloc.allocate<MyClass>(), std::move(alloc));
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `T` | template type param | The owned type (may be `T[]` for array ownership). |
| `TDestroyer` | template type param | optional — The destroyer policy; defaults to `DefaultInstanceDestroyer<T>::type`. Must be callable as `TDestroyer()(pointer)`. |
| `ptr_` | `pointer` | The raw pointer to take ownership of. Must not be shared with any other owner. |
| `destroyer` | `destroyer_type&&` | optional — A stateful destroyer instance; only for the `(pointer, destroyer_type&&)` constructor overload. |

### Watch out for

- `ScopedPtr` inherits `TDestroyer` privately, so the destroyer is invoked directly — do not pass a destroyer type whose `operator()` has preconditions that require it to be initialized after construction.
- Moving from a `ScopedPtr<U, UDestroyer>` to `ScopedPtr<T, TDestroyer>` requires that `U*` is implicitly convertible to `T*`. The static assertion in `makeScoped` checks this; the cross-type move constructor does not independently verify it.
- After `release()` the caller owns the pointer and is responsible for its destruction — the `ScopedPtr` no longer manages it.

### Relationships (see also)

- `makeScoped` — preferred factory; avoids spelling out policy types
- `New` / `Delete` — default creator/destroyer policies
- `FactoryCreate` / `FactoryDestroy` — policies for static-factory types
- `DefaultInstanceCreator` / `DefaultInstanceDestroyer` — traits that resolve the defaults

<!-- ink:api-end name="ScopedPtr" -->
