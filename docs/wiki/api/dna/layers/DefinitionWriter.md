# API Reference — `dna/layers/DefinitionWriter`

---

<!-- ink:api name="DefinitionWriter" module="dna/layers/DefinitionWriter" last_commit="api_scan" confidence="__CONFIDENCE__" updated="2026-06-10" api_kind="callable" -->

## `class DefinitionWriter : public DescriptorWriter`

Write static rig definition data — control names, joint hierarchy, blend shape channels, animated maps, and meshes — into a DNA asset. Implement via `Writer`, not directly from this class.

### When to use this

Use this interface when constructing or mutating the structural (non-animated) portion of a DNA rig: assigning names to GUI/raw controls, joints, blend shape channels, animated maps, and meshes; wiring those elements into LOD tiers; and setting the joint hierarchy and neutral-pose transforms. Do not subclass `DefinitionWriter` directly — the `@warning` in the class docstring requires implementors to inherit from `Writer` instead, which composes all layer interfaces.

### Method groups

| Group | Methods |
|---|---|
| GUI Controls | `clearGUIControlNames`, `setGUIControlName` |
| Raw Controls | `clearRawControlNames`, `setRawControlName` |
| Joints | `clearJointNames`, `setJointName`, `clearJointIndices`, `setJointIndices`, `clearLODJointMappings`, `setLODJointMapping`, `setJointHierarchy`, `setNeutralJointTranslations`, `setNeutralJointRotations` |
| Blend Shape Channels | `clearBlendShapeChannelNames`, `setBlendShapeChannelName`, `clearBlendShapeChannelIndices`, `setBlendShapeChannelIndices`, `clearLODBlendShapeChannelMappings`, `setLODBlendShapeChannelMapping` |
| Animated Maps | `clearAnimatedMapNames`, `setAnimatedMapName`, `clearAnimatedMapIndices`, `setAnimatedMapIndices`, `clearLODAnimatedMapMappings`, `setLODAnimatedMapMapping` |
| Meshes | `clearMeshNames`, `setMeshName`, `clearMeshIndices`, `setMeshIndices`, `clearLODMeshMappings`, `setLODMeshMapping`, `clearMeshBlendShapeChannelMappings`, `setMeshBlendShapeChannelMapping` |

### Example

```cpp
// writer is a concrete Writer* obtained from a factory (inherits DefinitionWriter)
// --- GUI / Raw controls ---
writer->clearGUIControlNames();
writer->setGUIControlName(0, "CTRL_L_eye_blink");
writer->setGUIControlName(1, "CTRL_R_eye_blink");

writer->clearRawControlNames();
writer->setRawControlName(0, "raw_L_eye_blink");

// --- Joints ---
writer->clearJointNames();
writer->setJointName(0, "spine_01");
writer->setJointName(1, "neck_01");
writer->setJointName(2, "head");

// Joint hierarchy: spine_01 is root (index 0); neck_01 and head are children of spine_01
const std::uint16_t hierarchy[] = {0, 0, 1};
writer->setJointHierarchy(hierarchy, 3);

// Neutral-pose translations
const Vector3 translations[] = {{0.0f, 0.0f, 0.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, 5.0f, 0.0f}};
writer->setNeutralJointTranslations(translations, 3);

// Assign joints to LOD 0 via an index list
const std::uint16_t lod0Joints[] = {0, 1, 2};
writer->clearJointIndices();
writer->setJointIndices(0, lod0Joints, 3);
writer->clearLODJointMappings();
writer->setLODJointMapping(/*lod=*/0, /*index=*/0);

// --- Blend shape channels ---
writer->clearBlendShapeChannelNames();
writer->setBlendShapeChannelName(0, "browInnerUp_L");

// Associate blend shape channel 0 with mesh 0
writer->clearMeshBlendShapeChannelMappings();
writer->setMeshBlendShapeChannelMapping(/*mappingIndex=*/0, /*meshIndex=*/0, /*bsChannelIndex=*/0);
```

### Watch out for

- **Implementors must inherit from `Writer`, not `DefinitionWriter` directly.** The class docstring carries an explicit `@warning` to this effect. Subclassing `DefinitionWriter` alone bypasses the full Writer composition and will produce an incomplete interface.
- **`setLODBlendShapeChannelMapping` uses an independent LOD index space from `BehaviorWriter::setBlendShapeChannelLODs`.** The docstring for that method explicitly notes these values are not interchangeable — mixing them causes silent mapping corruption.
- **All `set*Name` methods copy the passed string**, incurring an extra allocation. Callers can free the source buffer immediately after the call but should not assume zero-copy semantics.
- **Storage for names and indices is implicitly resized by index.** Writing to index 5 without writing indices 0–4 leaves those slots in an unspecified state. Populate slots in order or call the corresponding `clear*` method first to reset.
- **`setJointHierarchy` encodes the entire parent array in one call** — the array position is the child joint index and the value is the parent joint index. The root joint(s) reference themselves (value equals their own index, typically 0).

<!-- ink:api-end name="DefinitionWriter" -->
