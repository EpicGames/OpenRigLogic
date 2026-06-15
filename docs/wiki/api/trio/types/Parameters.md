# API Reference — `trio/types/Parameters`

---

<!-- ink:api name="AccessMode" module="trio/types/Parameters" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `AccessMode`

Specify whether a stream is opened for reading, writing, or both.

### Why this exists

Raw integer flags for stream direction are error-prone — a caller passing `3` conveys no intent, and the compiler cannot catch a transposed read/write flag. `AccessMode` makes the permitted operation explicit at the call site and lets the compiler reject nonsensical combinations. The three enumerators cover the full set of single-direction and bidirectional access without overlap.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Read` | `int (= 1)` | required — open the stream for input only |
| `Write` | `int (= 2)` | required — open the stream for output only |
| `ReadWrite` | `int (= 3)` | required — open the stream for both input and output |

### Construction

```cpp
// Pass as an argument when opening a stream
AccessMode mode = AccessMode::ReadWrite;
```

### Relationships

- `OpenMode` — paired enum that controls binary vs. text encoding; used alongside `AccessMode` when opening a stream

<!-- ink:api-end name="AccessMode" -->

<!-- ink:api name="OpenMode" module="trio/types/Parameters" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="data_shape" -->

## `OpenMode`

Specify whether a stream transfers raw bytes or text with platform newline handling.

### Why this exists

On some platforms, opening a file in text mode causes the runtime to translate newline sequences (`\r\n` ↔ `\n`), which silently corrupts binary data. Expressing this choice as a typed enum — rather than a bare flag or boolean — makes the intent self-documenting at the call site and prevents mixing incompatible modes. `OpenMode` is kept separate from `AccessMode` so the two concerns (direction and encoding) compose independently.

### Fields

| Name | Type | Description |
|------|------|-------------|
| `Binary` | `int (= 4)` | required — transfer raw bytes without newline translation |
| `Text` | `int (= 8)` | required — apply platform newline handling on read and write |

### Construction

```cpp
// Combine with AccessMode when opening a stream
OpenMode encoding = OpenMode::Binary;
```

### Relationships

- `AccessMode` — paired enum that controls read/write direction; used alongside `OpenMode` when opening a stream

<!-- ink:api-end name="OpenMode" -->
