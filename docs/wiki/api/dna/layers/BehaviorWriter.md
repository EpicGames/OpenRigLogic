# API Reference — `dna/layers/BehaviorWriter`

---

<!-- ink:api name="BehaviorWriter" module="dna/layers/BehaviorWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DNAAPI BehaviorWriter : public virtual DefinitionWriter`

Write rig evaluation behavior data into a DNA asset — GUI-to-raw control mapping, PSD expressions, joint group matrices, blend shape channels, and animated maps. Do not subclass this directly; inherit from `Writer` instead.

### When to use this

Use when constructing or modifying the behavioral layer of a DNA asset — populating evaluation data that drives how raw controls respond to GUI inputs, how joints are driven by PSD expressions, or how blend shapes and animated maps are activated per LOD. If you only need to read behavior data, use `BehaviorReader` instead.

### Watch out for

- Implementors must inherit from `Writer`, not from `BehaviorWriter` directly. Subclassing `BehaviorWriter` directly bypasses lifecycle management provided by `Writer`.
- `jointGroupIndex` passed to any `setJointGroup*` method must be less than the value returned by `getJointGroupCount`. Joint group storage is implicitly resized when an index exceeds current capacity, but out-of-range indices still violate the documented precondition.
- The LOD values set via `setBlendShapeChannelLODs` are not interchangeable with the LOD indices set in `DefinitionWriter::setBlendShapeNameIndices` and `DefinitionWriter::setLODBlendShapeMapping`. Passing the wrong set silently produces incorrect blend shape evaluation at runtime.

### Method groups

| Group | Methods |
|-------|---------|
| GUI-to-raw mapping | `setGUIToRawInputIndices`, `setGUIToRawOutputIndices`, `setGUIToRawFromValues`, `setGUIToRawToValues`, `setGUIToRawSlopeValues`, `setGUIToRawCutValues` |
| PSD expressions | `setPSDCount`, `setPSDRowIndices`, `setPSDColumnIndices`, `setPSDValues` |
| Joint matrix dimensions | `setJointRowCount`, `setJointColumnCount` |
| Joint groups | `clearJointGroups`, `deleteJointGroup`, `setJointGroupLODs`, `setJointGroupInputIndices`, `setJointGroupOutputIndices`, `setJointGroupValues`, `setJointGroupJointIndices` |
| Blend shape channels | `setBlendShapeChannelLODs`, `setBlendShapeChannelInputIndices`, `setBlendShapeChannelOutputIndices` |
| Animated maps | `setAnimatedMapLODs`, `setAnimatedMapInputIndices`, `setAnimatedMapOutputIndices`, `setAnimatedMapFromValues`, `setAnimatedMapToValues`, `setAnimatedMapSlopeValues`, `setAnimatedMapCutValues` |

### Example

```cpp
// Populate GUI-to-raw mapping
writer->setGUIToRawInputIndices(guiInputIdx.data(), static_cast<uint16_t>(guiInputIdx.size()));
writer->setGUIToRawOutputIndices(rawOutputIdx.data(), static_cast<uint16_t>(rawOutputIdx.size()));
writer->setGUIToRawFromValues(fromVals.data(), static_cast<uint16_t>(fromVals.size()));
writer->setGUIToRawSlopeValues(slopes.data(), static_cast<uint16_t>(slopes.size()));
writer->setGUIToRawCutValues(cuts.data(), static_cast<uint16_t>(cuts.size()));

// Populate PSD expressions
writer->setPSDCount(static_cast<uint16_t>(psdCount));
writer->setPSDRowIndices(psdRows.data(), static_cast<uint16_t>(psdRows.size()));
writer->setPSDColumnIndices(psdCols.data(), static_cast<uint16_t>(psdCols.size()));
writer->setPSDValues(psdWeights.data(), static_cast<uint16_t>(psdWeights.size()));
```

<!-- ink:api-end name="BehaviorWriter" -->
