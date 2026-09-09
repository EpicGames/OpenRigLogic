# API Reference — `arrayview/StringView`

---


<!-- ink:api name="StringView" module="arrayview/StringView" last_commit="api_scan" updated="2026-09-09" api_kind="callable" -->

## `class StringView : public ConstArrayView<char>`

A non-owning, NULL-safe view over a C string, built on top of `ConstArrayView<char>`.

### When to use this

Use `StringView` instead of a raw `const char*` when you want array-view-style helpers (size, begin/end) over string data while still being able to get back a guaranteed-non-NULL C string via `c_str()` — even when the underlying data pointer is `nullptr`, `c_str()` returns `""` instead of crashing.

### Method groups

| Group | Methods |
|-------|---------|
| Construction | (inherited via `using Base::ArrayView`) |
| Access | c_str, operator const char*, operator* |

### Example

```cpp
av::StringView name{nullptr, 0};
const char* safe = name.c_str();  // "" — never NULL, even for a null-backed view
```

### Returns

`const char*` (from `c_str()`, `operator const char*()`, `operator*()`) — the underlying C string, or `""` if the view has no backing data.

### Watch out for

- `c_str()`, `operator const char*()`, and `operator*()` all guard against a NULL `data()` pointer by returning `""` instead of NULL — callers never need to NULL-check the result themselves.
- All three accessors (`c_str()`, `operator const char*()`, `operator*()`) are identical — they all delegate to `dataOrEmpty()`. There is no semantic difference; use `c_str()` for readability.

<!-- ink:api-end name="StringView" -->
