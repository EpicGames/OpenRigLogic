# API Reference — `dna/StreamWriter`

---

<!-- ink:api name="StreamWriter" module="dna/StreamWriter" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class StreamWriter : public Writer`

Persist a DNA populated in memory to a stream by calling a single `write()` method.

### When to use this

Use this type as a polymorphic handle when you need to pass a stream writer without committing to a specific format (binary or JSON). Concrete subclasses handle format-specific serialization — reach for `BinaryStreamWriter` or `JSONStreamWriter` directly if you know the format at construction time.

Use a `StreamWriter` implementation when the Writer data you have filled in (directly or through `Writer::setFrom`) must be flushed to an output stream. It adds the `write()` step on top of the setters inherited from `Writer`.

### Example

```cpp
// Obtain a concrete writer (e.g. binary format), held as the abstract base
dna::BinaryStreamWriter* writer = dna::BinaryStreamWriter::create(stream, memRes);

// Populate internal structures via the Writer layer, then flush to stream
writer->write();

// Release when done
dna::BinaryStreamWriter::destroy(writer);
```

### Parameters

| Name | Type | Description |
|------|------|-------------|
| `write()` | `virtual void write() = 0` | Takes no parameters. Writes data to the stream from internal structures. |

### Watch out for

- The destructor is declared `override` and `write()` is pure virtual, so `StreamWriter` cannot be instantiated directly. Use a concrete implementation.
- The destructor is `override`, meaning ownership and cleanup are managed through the base pointer; do not delete a `StreamWriter*` with a non-virtual destructor in a custom subclass.
<!-- ink:api-end name="StreamWriter" -->
