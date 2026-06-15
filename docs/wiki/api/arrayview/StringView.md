# API Reference — `arrayview/StringView`

---

<!-- ink:api name="Base" module="arrayview/StringView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `Base`

Type alias for `ConstArrayView<char>`, declared inside `StringView` to name the immediate base class.

### Why this exists

`Base` is a `using` declaration that gives the `StringView` implementation a stable, readable name for its parent type. It allows `StringView` to inherit `ArrayView` constructors via `using Base::ArrayView` without repeating the full template instantiation. This is a class-internal alias and is not part of the public API surface — external code should use `ConstArrayView<char>` or `StringView` directly.

### Relationships

- `ConstArrayView<char>` — the aliased type; `Base` is exactly this type
- `StringView` — the class that declares this alias

<!-- ink:api-end name="Base" -->

<!-- ink:api name="StringView" module="arrayview/StringView" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class StringView : public ConstArrayView<char>`

Access a read-only character buffer as a null-safe C string. Use this when bridging `ConstArrayView<char>` data to APIs that expect `const char*`.

### When to use this

Use `StringView` when you hold a `ConstArrayView<char>` and need to pass it to a C API or any function that expects a raw `const char*`. It guarantees a non-null pointer even when the underlying buffer is null — you never have to add a `nullptr` guard at the call site. Use `ConstArrayView<char>` directly when you do not need C-string compatibility.

### Example

```cpp
// Given a StringView from a DNA reader:
StringView name = reader.getName();   // wraps a ConstArrayView<char>

// All three accessors are equivalent — prefer c_str() for clarity:
const char* s1 = name.c_str();        // returns "" if buffer is null
const char* s2 = name;                // implicit conversion
const char* s3 = *name;               // dereference operator

// Safe to pass directly to C APIs:
printf("%s\n", name.c_str());
```

### Returns

`const char*` — pointer to the character data, or `""` (a valid empty C string) when the underlying buffer is null. The pointer is owned by the source buffer; do not free it.

### Watch out for

- All three accessors (`c_str()`, `operator const char*()`, `operator*()`) are identical — they all delegate to `dataOrEmpty()`. There is no semantic difference; use `c_str()` for readability.
- When the underlying `ConstArrayView<char>` was constructed with a null pointer, `StringView` silently returns a pointer to a static empty string (`""`). Callers that need to distinguish "no value" from "empty string" must check `data() == nullptr` on the underlying view before accessing through `StringView`.

<!-- ink:api-end name="StringView" -->
