# API Reference — `dna/StreamWriter`

---

<!-- ink:api name="StreamWriter" module="dna/StreamWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class StreamWriter : public Writer`

Abstract base defining the interface for writing DNA data to an output stream. Obtain a concrete instance via `BinaryStreamWriter` or `JSONStreamWriter` rather than subclassing directly.

### When to use this

Use this type as a polymorphic handle when you need to pass a stream writer without committing to a specific format (binary or JSON). Concrete subclasses handle format-specific serialization — reach for `BinaryStreamWriter` or `JSONStreamWriter` directly if you know the format at construction time.

### Example

```cpp
// Obtain a concrete writer (e.g. binary format), held as the abstract base
dna::BinaryStreamWriter* writer = dna::BinaryStreamWriter::create(stream, memRes);

// Populate internal structures via the Writer layer, then flush to stream
writer->write();

// Release when done
dna::BinaryStreamWriter::destroy(writer);
```

### Watch out for

- `write()` is pure virtual — `StreamWriter` cannot be instantiated directly. Always use a concrete subclass or a factory that returns one.
- The destructor is `override`, meaning ownership and cleanup are managed through the base pointer; do not delete a `StreamWriter*` with a non-virtual destructor in a custom subclass.

<!-- ink:api-end name="StreamWriter" -->
