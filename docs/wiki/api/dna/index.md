# API Reference — `dna`

_75 entries across 38 modules._

## By Task

### Stream I/O configuration

Read and write DNA rig data in binary or JSON format

| API | Module | Summary |
|-----|--------|---------|
| [BinaryStreamReader](BinaryStreamReader.md) | BinaryStreamReader | Reads DNA rig data from a binary stream, with factory methods that let callers control which data layers and levels of detail get loaded. |
| [BinaryStreamReader](BinaryStreamWriter.md) | BinaryStreamWriter | A forward-declared reader type that `BinaryStreamWriter` accepts as a data source when copying DNA content between streams. |
| [BinaryStreamReader](JSONStreamWriter.md) | JSONStreamWriter | Forward-declared peer type used as a data source when copying binary DNA data into a `JSONStreamWriter` via `setFrom`. |
| [BinaryStreamWriter](BinaryStreamWriter.md) | BinaryStreamWriter | Write DNA rig data to a binary stream, and optionally populate it directly from an existing `BinaryStreamReader` or `JSONStreamReader` instance. |
| [Configuration](Configuration.md) | Configuration | Bundles all the load-time options for reading a DNA file — which layers, which LODs, and which coordinate/unit conventions to apply. |
| [CoordinateSystem](Configuration.md) | Configuration | Alias for `tdm::coord_sys`, describing the axis directions for all coordinate axes. |
| [CoordinateSystemTransformPolicy](Configuration.md) | Configuration | Controls whether loaded rig data is converted into a destination coordinate system or left as authored. |
| [DataLayer](Configuration.md) | Configuration | Bitmask enum identifying the loadable layers of a DNA rig, from the lightweight `Descriptor` up through `All`. |
| [Direction](Configuration.md) | Configuration | Alias for `tdm::axis_dir`, representing a signed direction along a coordinate axis. |
| [FaceWindingOrder](Configuration.md) | Configuration | Face vertex winding order of the geometry data, viewed along the outward surface normal. |
| [JSONStreamReader](BinaryStreamWriter.md) | BinaryStreamWriter | A forward-declared reader type that `BinaryStreamWriter` accepts as a data source when the build supports JSON, letting a writer be populated from JSON-formatted DNA data. |
| [JSONStreamReader](JSONStreamReader.md) | JSONStreamReader | Read DNA rig data from a stream that holds the human-readable JSON representation instead of the compact binary format. |
| [JSONStreamReader](JSONStreamWriter.md) | JSONStreamWriter | Forward-declared peer type used as a data source when copying JSON DNA data into a `JSONStreamWriter` via `setFrom`. |
| [JSONStreamWriter](JSONStreamWriter.md) | JSONStreamWriter | Write DNA rig data out as human-readable JSON, and optionally populate it directly from an existing `BinaryStreamReader` or `JSONStreamReader`. |
| [RotationDirection](Configuration.md) | Configuration | Alias for `tdm::rot_dir`, representing the sign of rotation (positive/negative) about an axis. |
| [RotationSequence](Configuration.md) | Configuration | Alias for `tdm::rot_seq`, representing the order in which axis rotations are composed (e.g. XYZ). |
| [RotationSign](Configuration.md) | Configuration | Alias for `tdm::rot_sign`, a per-axis triple of `RotationDirection` values specifying the sign convention for rotation on each axis. |
| [RotationUnit](Configuration.md) | Configuration | Unit of measurement (degrees or radians) used for rotation values in the rig. |
| [TranslationUnit](Configuration.md) | Configuration | Unit of measurement (centimeters or meters) used for translation values in the rig. |
| [UnknownLayerPolicy](Configuration.md) | Configuration | Controls whether layers the reader/writer doesn't recognize are kept or discarded. |

### Generic reader and writer

Aggregate interfaces for reading and writing all DNA layers, and base types for format-specific implementations

| API | Module | Summary |
|-----|--------|---------|
| [Reader](Reader.md) | Reader | Aggregate reader interface combining every DNA layer's reader interface into one type. |
| [Reader](Writer.md) | Writer | Forward declaration of the DNA `Reader` type, used by `Writer::setFrom` as the source to copy data from. |
| [StreamReader](StreamReader.md) | StreamReader | Extends `Reader` with the ability to actually pull rig data from a stream into internal structures. |
| [StreamWriter](StreamWriter.md) | StreamWriter | Persist a DNA populated in memory to a stream by calling a single `write()` method. |
| [Writer](Writer.md) | Writer | Write every DNA layer through one interface, and clone an existing Reader into it with `setFrom`. |

### Instance creation helpers

Factory template specializations for the pma framework to construct and destroy stream readers and writers

| API | Module | Summary |
|-----|--------|---------|
| [DefaultInstanceCreator](BinaryStreamReader.md) | BinaryStreamReader | A `pma` framework specialization that tells the generic instance-creation machinery how to construct a `BinaryStreamReader` by default. |
| [DefaultInstanceCreator](BinaryStreamWriter.md) | BinaryStreamWriter | Template specialization that tells generic `pma` factory code to use `BinaryStreamWriter::create` when constructing instances of this type. |
| [DefaultInstanceCreator](JSONStreamReader.md) | JSONStreamReader | Template specialization that tells generic `pma` factory code to use `JSONStreamReader::create` when constructing instances of this type. |
| [DefaultInstanceCreator](JSONStreamWriter.md) | JSONStreamWriter | Template specialization that tells generic `pma` factory code to use `JSONStreamWriter::create` when constructing instances of this type. |
| [DefaultInstanceDestroyer](BinaryStreamReader.md) | BinaryStreamReader | A `pma` framework specialization that tells the generic instance-destruction machinery how to release a `BinaryStreamReader` created via the factory pattern. |
| [DefaultInstanceDestroyer](BinaryStreamWriter.md) | BinaryStreamWriter | Template specialization that tells generic `pma` factory code to use `BinaryStreamWriter::destroy` when releasing instances of this type. |
| [DefaultInstanceDestroyer](JSONStreamReader.md) | JSONStreamReader | Template specialization that tells generic `pma` factory code to use `JSONStreamReader::destroy` when releasing instances of this type. |
| [DefaultInstanceDestroyer](JSONStreamWriter.md) | JSONStreamWriter | Template specialization that tells generic `pma` factory code to use `JSONStreamWriter::destroy` when releasing instances of this type. |

### Layer readers and writers

Interfaces for reading and writing specific DNA layers like behavior and definition data

| API | Module | Summary |
|-----|--------|---------|
| [BehaviorReader](layers/BehaviorReader.md) | BehaviorReader | Read the DNA attributes that define how the rig evaluates: GUI-to-raw control mapping, PSD expressions and joint matrices. |
| [BehaviorWriter](layers/BehaviorWriter.md) | BehaviorWriter | Write the DNA attributes that define how the rig evaluates: GUI-to-raw control mapping and PSD expressions. |
| [DefinitionReader](layers/DefinitionReader.md) | DefinitionReader | Read the rig's static data: GUI and raw control names, joint names, per-LOD joint indices and the joint hierarchy. |
| [DefinitionWriter](layers/DefinitionWriter.md) | DefinitionWriter | Write the rig's static data: GUI and raw control names, joint names and joint index lists. |
| [MeshBlendShapeChannelMapping](layers/DefinitionReader.md) | DefinitionReader | Mapping that associates a blend shape channel with its mesh. |

### Character metadata

Read and write character and rig information such as name, archetype, gender, age, and file format version

| API | Module | Summary |
|-----|--------|---------|
| [Archetype](layers/Descriptor.md) | Descriptor | Character archetype stored in the DNA descriptor. |
| [DescriptorReader](layers/DescriptorReader.md) | DescriptorReader | Read metadata about the character and the rig: name, archetype, gender, age, key-value metadata, coordinate conventions, LOD count and database info. |
| [DescriptorWriter](layers/DescriptorWriter.md) | DescriptorWriter | Write metadata about the character and the rig: name, archetype, gender, age, key-value metadata, coordinate conventions, LOD count and database info. |
| [Gender](layers/Descriptor.md) | Descriptor | Character gender stored in the DNA descriptor. |
| [HeaderReader](layers/HeaderReader.md) | HeaderReader | Read the file format generation and version of a rig's DNA data. |
| [HeaderWriter](layers/HeaderWriter.md) | HeaderWriter | Set the file format generation and version of a rig's DNA data. |
| [VersionInfo](version/VersionInfo.md) | VersionInfo | Query the version of the DNA library that the current binary was built against. |

### Geometry data

Read and write mesh vertex positions, normals, texture coordinates, and vertex layouts

| API | Module | Summary |
|-----|--------|---------|
| [Delta](layers/Geometry.md) | Geometry | Type alias for `Vector3` that represents a per-vertex offset. |
| [GeometryReader](layers/GeometryReader.md) | GeometryReader | Read vertex data (positions and texture coordinates) of the meshes in a rig, without modifying it. |
| [GeometryWriter](layers/GeometryWriter.md) | GeometryWriter | Write mesh geometry (vertex positions, texture coordinates, normals and layouts) into a rig. Write-only counterpart of the geometry reader. |
| [Normal](layers/Geometry.md) | Geometry | Type alias for `Vector3` that represents a vertex normal. |
| [Position](layers/Geometry.md) | Geometry | Type alias for `Vector3` that represents a vertex position. |
| [TextureCoordinate](layers/Geometry.md) | Geometry | A 2D UV coordinate for a vertex. |
| [Vector3](types/Vector3.md) | Vector3 | A plain 3-component float vector (x, y, z) with the standard set of component-wise arithmetic operators. |
| [VertexLayout](layers/Geometry.md) | Geometry | Describes one vertex as a set of indices into the position, texture coordinate and normal arrays. |

### Joint transform metadata

Read and write how each joint's translation, rotation, and scale are represented at runtime

| API | Module | Summary |
|-----|--------|---------|
| [JointBehaviorMetadataReader](layers/JointBehaviorMetadataReader.md) | JointBehaviorMetadataReader | Read how each joint's translation, rotation and scale are represented internally when the rig is evaluated at runtime. |
| [JointBehaviorMetadataWriter](layers/JointBehaviorMetadataWriter.md) | JointBehaviorMetadataWriter | Set how each joint's translation, rotation and scale are represented internally when the rig is evaluated at runtime. |
| [RotationRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Describes how a joint's rotation values are represented. |
| [ScaleRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Describes how a joint's scale values are represented. |
| [TranslationRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Describes how a joint's translation values are represented. |
| [TwistAxis](layers/Twist.md) | Twist | Identifies which local axis (X, Y, or Z) a twist or swing transformation is measured around. |

### Machine learned behavior

Read and write neural network configuration, operations, and which networks drive mesh regions

| API | Module | Summary |
|-----|--------|---------|
| [ActivationFunction](layers/MachineLearnedBehavior.md) | MachineLearnedBehavior | Enumerates the activation functions available for machine learned behavior. |
| [MachineLearnedBehaviorExtReader](layers/MachineLearnedBehaviorExtReader.md) | MachineLearnedBehaviorExtReader | Read-only accessors to the neural network extension data associated with a rig — the per-model operation graph (operation sets, operation types, static parameters, and dependency indices) that `MachineLearnedBehaviorReader` itself does not expose. |
| [MachineLearnedBehaviorExtWriter](layers/MachineLearnedBehaviorExtWriter.md) | MachineLearnedBehaviorExtWriter | Write-only accessors for the operation instance data associated with a rig — building up the same per-model operation graph that `MachineLearnedBehaviorExtReader` exposes for reading. |
| [MachineLearnedBehaviorOperationType](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Enumerates the kind of computation a single ML operation performs within a neural network layer. |
| [MachineLearnedBehaviorParameterKey](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Enumerates the static parameter keys an ML operation can reference, covering joint transform types, coordinate axes, rotation signs, and rotation sequence. |
| [MachineLearnedBehaviorReader](layers/MachineLearnedBehaviorReader.md) | MachineLearnedBehaviorReader | Read-only accessors to the neural network data associated with a rig — ML control names, neural network counts, which networks are active at each LOD, and which networks drive which mesh region. |
| [MachineLearnedBehaviorWriter](layers/MachineLearnedBehaviorWriter.md) | MachineLearnedBehaviorWriter | Write-only accessors for the neural network data associated with a rig — defining ML control names, the neural-network index lists, which networks run at each LOD, and mesh-region associations. |

### RBF pose solving

Read and write radial basis function pose targets, distance methods, and solver configuration

| API | Module | Summary |
|-----|--------|---------|
| [AutomaticRadius](layers/RBFBehavior.md) | RBFBehavior | Toggles whether an RBF target's radius of influence is computed automatically rather than set explicitly. |
| [RBFBehaviorReader](layers/RBFBehaviorReader.md) | RBFBehaviorReader | Read-only accessors to the RBF (radial basis function) pose data associated with a rig — pose names, their driven joint/blend-shape/animated-map outputs, output values, scale, and the controls that feed them. |
| [RBFBehaviorWriter](layers/RBFBehaviorWriter.md) | RBFBehaviorWriter | Write-only accessors for the RBF pose data associated with a rig — pose names, scale, control names, and the input/output control indices that connect poses to the rest of the rig. |
| [RBFDistanceMethod](layers/RBFBehavior.md) | RBFBehavior | Selects how the distance between an input and an RBF pose target is measured. |
| [RBFFunctionType](layers/RBFBehavior.md) | RBFBehavior | Selects the radial falloff function applied to distance when computing an RBF target's contribution weight. |
| [RBFNormalizeMethod](layers/RBFBehavior.md) | RBFBehavior | Selects when contribution weights from an RBF solve are normalized to sum to a consistent total. |
| [RBFSolverType](layers/RBFBehavior.md) | RBFBehavior | Selects the algorithm used to combine RBF (radial basis function) pose target contributions into a final weight. |

### Twist and swing deformation

Read and write twist and swing joint transformations that follow primary joint rotations

| API | Module | Summary |
|-----|--------|---------|
| [TwistSwingBehaviorReader](layers/TwistSwingBehaviorReader.md) | TwistSwingBehaviorReader | Read-only accessors to the swing and twist data associated with a rig — which joint rotations drive twist/swing transformations, which joints they output to, and the blend weights applied. |
| [TwistSwingBehaviorWriter](layers/TwistSwingBehaviorWriter.md) | TwistSwingBehaviorWriter | Write-only accessors to the swing and twist data associated with a rig — defining twist/swing parameter groups, their axis, driver control indices, driven joints, and blend weights. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| BinaryStreamReader | [BinaryStreamReader.md](BinaryStreamReader.md) | 3 |
| BinaryStreamWriter | [BinaryStreamWriter.md](BinaryStreamWriter.md) | 5 |
| Configuration | [Configuration.md](Configuration.md) | 12 |
| JSONStreamReader | [JSONStreamReader.md](JSONStreamReader.md) | 3 |
| JSONStreamWriter | [JSONStreamWriter.md](JSONStreamWriter.md) | 5 |
| Reader | [Reader.md](Reader.md) | 1 |
| StreamReader | [StreamReader.md](StreamReader.md) | 1 |
| StreamWriter | [StreamWriter.md](StreamWriter.md) | 1 |
| Writer | [Writer.md](Writer.md) | 2 |
| BehaviorReader | [layers/BehaviorReader.md](layers/BehaviorReader.md) | 1 |
| BehaviorWriter | [layers/BehaviorWriter.md](layers/BehaviorWriter.md) | 1 |
| DefinitionReader | [layers/DefinitionReader.md](layers/DefinitionReader.md) | 2 |
| DefinitionWriter | [layers/DefinitionWriter.md](layers/DefinitionWriter.md) | 1 |
| Descriptor | [layers/Descriptor.md](layers/Descriptor.md) | 2 |
| DescriptorReader | [layers/DescriptorReader.md](layers/DescriptorReader.md) | 1 |
| DescriptorWriter | [layers/DescriptorWriter.md](layers/DescriptorWriter.md) | 1 |
| Geometry | [layers/Geometry.md](layers/Geometry.md) | 5 |
| GeometryReader | [layers/GeometryReader.md](layers/GeometryReader.md) | 1 |
| GeometryWriter | [layers/GeometryWriter.md](layers/GeometryWriter.md) | 1 |
| HeaderReader | [layers/HeaderReader.md](layers/HeaderReader.md) | 1 |
| HeaderWriter | [layers/HeaderWriter.md](layers/HeaderWriter.md) | 1 |
| JointBehaviorMetadata | [layers/JointBehaviorMetadata.md](layers/JointBehaviorMetadata.md) | 3 |
| JointBehaviorMetadataReader | [layers/JointBehaviorMetadataReader.md](layers/JointBehaviorMetadataReader.md) | 1 |
| JointBehaviorMetadataWriter | [layers/JointBehaviorMetadataWriter.md](layers/JointBehaviorMetadataWriter.md) | 1 |
| MachineLearnedBehavior | [layers/MachineLearnedBehavior.md](layers/MachineLearnedBehavior.md) | 1 |
| MachineLearnedBehaviorExt | [layers/MachineLearnedBehaviorExt.md](layers/MachineLearnedBehaviorExt.md) | 2 |
| MachineLearnedBehaviorExtReader | [layers/MachineLearnedBehaviorExtReader.md](layers/MachineLearnedBehaviorExtReader.md) | 1 |
| MachineLearnedBehaviorExtWriter | [layers/MachineLearnedBehaviorExtWriter.md](layers/MachineLearnedBehaviorExtWriter.md) | 1 |
| MachineLearnedBehaviorReader | [layers/MachineLearnedBehaviorReader.md](layers/MachineLearnedBehaviorReader.md) | 1 |
| MachineLearnedBehaviorWriter | [layers/MachineLearnedBehaviorWriter.md](layers/MachineLearnedBehaviorWriter.md) | 1 |
| RBFBehavior | [layers/RBFBehavior.md](layers/RBFBehavior.md) | 5 |
| RBFBehaviorReader | [layers/RBFBehaviorReader.md](layers/RBFBehaviorReader.md) | 1 |
| RBFBehaviorWriter | [layers/RBFBehaviorWriter.md](layers/RBFBehaviorWriter.md) | 1 |
| Twist | [layers/Twist.md](layers/Twist.md) | 1 |
| TwistSwingBehaviorReader | [layers/TwistSwingBehaviorReader.md](layers/TwistSwingBehaviorReader.md) | 1 |
| TwistSwingBehaviorWriter | [layers/TwistSwingBehaviorWriter.md](layers/TwistSwingBehaviorWriter.md) | 1 |
| Vector3 | [types/Vector3.md](types/Vector3.md) | 1 |
| VersionInfo | [version/VersionInfo.md](version/VersionInfo.md) | 1 |
