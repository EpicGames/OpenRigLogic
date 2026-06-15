# API Reference — `trio/Concepts`

---

<!-- ink:api name="Bounded" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Bounded`

Abstract interface to query the total byte size of a stream without reading it.

### When to use this

Implement `Bounded` when callers need to pre-allocate buffers, validate integrity, or display progress — any situation where knowing the stream's total length before reading is useful. Combine with `Seekable` when you also need to rewind after inspecting the size.

### Method groups

| Group | Methods |
|-------|---------|
| Measurement | `size` |

### Example

```cpp
// Pre-allocate a buffer exactly sized to the stream before reading
void loadAll(trio::Readable* src, trio::Bounded* meta) {
    std::uint64_t totalBytes = meta->size();
    std::vector<char> buffer(static_cast<std::size_t>(totalBytes));
    src->read(buffer.data(), buffer.size());
}
```

### Returns

`std::uint64_t` — total size of the stream in bytes.

<!-- ink:api-end name="Bounded" -->

<!-- ink:api name="Buffered" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Buffered`

Abstract interface for streams that stage writes in memory and must be explicitly flushed to persist data to the filesystem.

### When to use this

Call `flush()` after a batch of writes to guarantee that in-memory staged data reaches underlying storage. Implement `Buffered` when your concrete stream wraps a buffered I/O layer (such as `FILE*` or `std::ofstream`) where process termination without flushing can silently discard data.

### Watch out for

- `flush()` is distinct from `close()`. Closing a stream does not guarantee that buffered data is flushed unless the implementation explicitly does so. Call `flush()` before `close()` when durability is required.

### Method groups

| Group | Methods |
|-------|---------|
| Buffer control | `flush` |

### Example

```cpp
FileWriteStream stream("output.dna");
stream.open();
// ... write all DNA layers ...
stream.flush();  // ensure bytes reach disk before signaling completion
stream.close();
```

<!-- ink:api-end name="Buffered" -->

<!-- ink:api name="Closeable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Closeable`

Abstract interface that provides a single `close()` method to release access to a stream resource.

### When to use this

Implement `Closeable` when your stream holds a resource that must be explicitly released — file handles, network connections, or memory-mapped regions. Most callers use `Controllable` (which inherits both `Openable` and `Closeable`) rather than `Closeable` alone.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | `close` |

### Example

```cpp
// Use Closeable directly only when open semantics are handled elsewhere.
class MappedRegion : public trio::Closeable, public trio::Readable {
public:
    void close() override {
        if (ptr_) { munmap(ptr_, length_); ptr_ = nullptr; }
    }
    std::size_t read(char* dst, std::size_t n) override { /* memcpy impl */ }
    std::size_t read(trio::Writable* dst, std::size_t n) override { /* pipe impl */ }
private:
    void* ptr_ = nullptr;
    std::size_t length_ = 0;
};
```

<!-- ink:api-end name="Closeable" -->

<!-- ink:api name="Controllable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Controllable : public Openable, public Closeable`

Convenience mixin that combines `Openable` and `Closeable` into a single base for streams that require explicit open/close lifecycle management.

### When to use this

Inherit from `Controllable` instead of inheriting `Openable` and `Closeable` separately — it is the standard base for any concrete stream type that owns a resource. Callers that accept a `Controllable*` can manage the full open/close lifecycle without knowing the concrete stream type.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | `open`, `close` |

### Example

```cpp
class FileStream : public trio::Controllable,
                   public trio::Readable,
                   public trio::Seekable {
public:
    void open() override  { file_ = fopen("character.dna", "rb"); }
    void close() override { if (file_) { fclose(file_); file_ = nullptr; } }
    std::size_t read(char* dst, std::size_t n) override { return fread(dst, 1, n, file_); }
    std::size_t read(trio::Writable* dst, std::size_t n) override { /* pipe impl */ }
    std::uint64_t tell() override { return ftell(file_); }
    void seek(std::uint64_t pos) override { fseek(file_, pos, SEEK_SET); }
private:
    FILE* file_ = nullptr;
};

void process(trio::Controllable* stream) {
    stream->open();
    // use stream ...
    stream->close();
}
```

<!-- ink:api-end name="Controllable" -->

<!-- ink:api name="Openable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Openable`

Abstract interface that provides a single `open()` method to begin access to a stream resource.

### When to use this

Implement `Openable` when your stream requires explicit initialization before reads or writes — opening a file descriptor, establishing a connection, or mapping memory. Most concrete stream types in `trio` inherit both `Openable` and `Closeable` via `Controllable`; implement `Openable` alone only when close semantics are not needed.

### Method groups

| Group | Methods |
|-------|---------|
| Lifecycle | `open` |

### Example

```cpp
// Prefer Controllable for types needing both open and close.
// Use Openable in isolation only when no close step is required.
class FileStream : public trio::Controllable, public trio::Readable {
public:
    void open() override  { file_ = fopen("rig.dna", "rb"); }
    void close() override { if (file_) { fclose(file_); file_ = nullptr; } }
    std::size_t read(char* dst, std::size_t n) override { return fread(dst, 1, n, file_); }
    std::size_t read(trio::Writable* dst, std::size_t n) override { /* pipe impl */ }
private:
    FILE* file_ = nullptr;
};

FileStream stream;
stream.open();
// ... read DNA data ...
stream.close();
```

<!-- ink:api-end name="Openable" -->

<!-- ink:api name="Readable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Readable`

Abstract interface for reading bytes from a stream — into a raw buffer or directly into a `Writable` destination.

### When to use this

Implement `Readable` when constructing a source stream that callers will consume. Use the buffer overload (`read(char*, size_t)`) to fill an in-memory buffer; use the stream overload (`read(Writable*, size_t)`) to forward bytes directly to a `Writable` without staging them in memory.

### Method groups

| Group | Methods |
|-------|---------|
| Read | `read` |

### Example

```cpp
// Implement Readable over a memory region
class MemoryReadable : public trio::Readable {
public:
    MemoryReadable(const char* data, std::size_t length)
        : data_{data}, length_{length}, pos_{0} {}

    std::size_t read(char* destination, std::size_t size) override {
        std::size_t n = std::min(size, length_ - pos_);
        std::memcpy(destination, data_ + pos_, n);
        pos_ += n;
        return n;
    }
    std::size_t read(trio::Writable* destination, std::size_t size) override {
        std::size_t n = std::min(size, length_ - pos_);
        std::size_t written = destination->write(data_ + pos_, n);
        pos_ += written;
        return written;
    }
private:
    const char* data_;
    std::size_t length_;
    std::size_t pos_;
};
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `destination` | `char*` or `trio::Writable*` | required — buffer or stream to write the read bytes into |
| `size` | `std::size_t` | required — number of bytes to read from the stream |

### Returns

`std::size_t` — number of bytes actually read. Returns less than `size` at end-of-stream.

<!-- ink:api-end name="Readable" -->

<!-- ink:api name="Resizable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Resizable`

Abstract interface for streams that support explicit resizing — truncating or extending the underlying storage to a requested byte length.

### When to use this

Implement `Resizable` when the stream's backing store is growable or truncatable (files, memory buffers). Use `resize()` to pre-allocate space before a large write sequence, or to truncate after rewriting a smaller payload into an existing stream.

### Method groups

| Group | Methods |
|-------|---------|
| Storage control | `resize` |

### Example

```cpp
// Truncate a DNA stream to a newly computed size after stripping unused layers
void trimStream(trio::Resizable* stream, std::uint64_t newSize) {
    stream->resize(newSize);
    // subsequent writes stay within [0, newSize)
}
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `size` | `std::uint64_t` | required — target size in bytes; may be larger (extend) or smaller (truncate) than the current size |

<!-- ink:api-end name="Resizable" -->

<!-- ink:api name="Seekable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Seekable`

Abstract interface for random-access positioning within a stream — query the current byte offset or jump to an absolute position.

### When to use this

Implement `Seekable` when the stream supports non-sequential access (files, memory buffers). Callers combine it with `Readable` or `Writable` to rewind, skip headers, or patch previously written data. Do not implement it for purely sequential streams such as network sockets where repositioning is undefined.

### Method groups

| Group | Methods |
|-------|---------|
| Position | `tell`, `seek` |

### Example

```cpp
class FileStream : public trio::Readable, public trio::Seekable {
public:
    std::uint64_t tell() override {
        return static_cast<std::uint64_t>(ftell(file_));
    }
    void seek(std::uint64_t position) override {
        fseek(file_, static_cast<long>(position), SEEK_SET);
    }
    std::size_t read(char* dst, std::size_t n) override { return fread(dst, 1, n, file_); }
    std::size_t read(trio::Writable* dst, std::size_t n) override { /* pipe impl */ }
private:
    FILE* file_;
};

// Rewind after scanning the header
std::uint64_t headerEnd = stream.tell();
stream.seek(0);  // back to beginning
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `position` | `std::uint64_t` | required (`seek` only) — absolute byte offset from stream start; 0 is the beginning |

### Returns

`std::uint64_t` (`tell`) — current byte offset from stream start; 0 denotes the beginning.

<!-- ink:api-end name="Seekable" -->

<!-- ink:api name="Writable" module="trio/Concepts" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class TRIOAPI Writable`

Abstract interface for writing bytes to a stream — from a raw buffer or by pulling from a `Readable` source.

### When to use this

Implement `Writable` when constructing a destination stream that downstream code writes into. Use the buffer overload (`write(const char*, size_t)`) when data lives in memory; use the stream overload (`write(Readable*, size_t)`) to pipe a `Readable` directly into this stream without an intermediate copy.

### Method groups

| Group | Methods |
|-------|---------|
| Write | `write` |

### Example

```cpp
// Implement Writable to wrap a file handle
class FileWritable : public trio::Writable {
public:
    std::size_t write(const char* source, std::size_t size) override {
        return fwrite(source, 1, size, file_);
    }
    std::size_t write(trio::Readable* source, std::size_t size) override {
        char buf[4096];
        std::size_t written = 0;
        while (written < size) {
            std::size_t chunk = std::min(size - written, sizeof(buf));
            std::size_t r = source->read(buf, chunk);
            if (r == 0) break;
            written += write(buf, r);
        }
        return written;
    }
private:
    FILE* file_;
};
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `source` | `const char*` or `trio::Readable*` | required — buffer or stream to read bytes from |
| `size` | `std::size_t` | required — number of bytes to write to the stream |

### Returns

`std::size_t` — number of bytes actually written. May be less than `size` if the stream cannot accept all data.

<!-- ink:api-end name="Writable" -->
