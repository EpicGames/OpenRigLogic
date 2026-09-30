# API Reference — `dna/layers/BehaviorReader`

---

<!-- ink:api name="BehaviorReader" module="dna/layers/BehaviorReader" last_commit="api_scan" updated="2026-09-30" api_kind="callable" -->
## `class DNAAPI BehaviorReader : public virtual DefinitionReader`

Read the DNA attributes that define how the rig evaluates: GUI-to-raw control mapping, PSD expressions and joint matrices.

### When to use this

Use this interface when you need to read rig evaluation data from a DNA file without modifying it. It covers three behavioral data domains: how GUI controls map to raw rig controls (including the piecewise-linear slope/cut formula), which PSD (Pose Space Deformation) expressions exist and their weights, and the structure of the joint matrix across LODs. Use `BehaviorWriter` when you need to write or modify this data.

### Method groups

| Group | Methods |
|-------|---------|
| GUIToRaw | getGUIToRawInputIndices, getGUIToRawOutputIndices, getGUIToRawFromValues, getGUIToRawToValues, getGUIToRawSlopeValues, getGUIToRawCutValues |
| PSD | getPSDCount, getPSDRowIndices, getPSDColumnIndices, getPSDValues |
| Joint | getJointRowCount, getJointColumnCount, getJointVariableAttributeIndices, getJointGroupCount |

### Example

```cpp
// Obtain a BehaviorReader via a Reader implementation (e.g. BinaryStreamReader)
BehaviorReader* behavior = reader->getBehaviorReader();

// Inspect GUI-to-raw control mapping
auto inputIndices  = behavior->getGUIToRawInputIndices();
auto outputIndices = behavior->getGUIToRawOutputIndices();
auto slopes        = behavior->getGUIToRawSlopeValues();
auto cuts          = behavior->getGUIToRawCutValues();

// Iterate PSD expressions
std::uint16_t psdCount = behavior->getPSDCount();
auto psdRows    = behavior->getPSDRowIndices();    // PSD expression indices
auto psdCols    = behavior->getPSDColumnIndices(); // Control input indices
auto psdWeights = behavior->getPSDValues();

// Query joint matrix dimensions and per-LOD variable attributes
std::uint16_t rows   = behavior->getJointRowCount();
std::uint16_t cols   = behavior->getJointColumnCount();
std::uint16_t groups = behavior->getJointGroupCount();
auto lod0Attrs = behavior->getJointVariableAttributeIndices(0); // LOD 0
```

### Returns

Array accessors return a `ConstArrayView` over the stored values. Counts return `std::uint16_t`.

### Watch out for

- The destructor is protected, so you cannot delete through a `BehaviorReader` pointer.
- `getJointVariableAttributeIndices` takes the `lod` whose joint attribute indices you want.
- Do not implement this interface directly. The docstring explicitly warns that implementors must inherit from `Reader`, not `BehaviorReader`. Inheriting from `BehaviorReader` alone bypasses required lifecycle and stream-management contracts.
<!-- ink:api-end name="BehaviorReader" -->
