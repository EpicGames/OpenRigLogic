# API Reference — `dna`

_75 entries across 38 modules._

## By Task

### Binary DNA stream I/O

Read and write DNA rig data in the compact binary format, with control over which layers and detail levels to load

| API | Module | Summary |
|-----|--------|---------|
| [BinaryStreamReader](BinaryStreamReader.md) | BinaryStreamReader | Reads DNA rig data from a binary stream, with factory methods that let callers control which data layers and levels of detail get loaded. |
| [BinaryStreamReader](BinaryStreamWriter.md) | BinaryStreamWriter | A forward-declared reader type that `BinaryStreamWriter` accepts as a data source when copying DNA content between streams. |
| [BinaryStreamReader](JSONStreamWriter.md) | JSONStreamWriter | Forward-declared peer type used as a data source when copying binary DNA data into a `JSONStreamWriter` via `setFrom`. |
| [BinaryStreamWriter](BinaryStreamWriter.md) | BinaryStreamWriter | Write DNA rig data to a binary stream, and optionally populate it directly from an existing `BinaryStreamReader` or `JSONStreamReader` instance. |
| [DefaultInstanceCreator](BinaryStreamReader.md) | BinaryStreamReader | A `pma` framework specialization that tells the generic instance-creation machinery how to construct a `BinaryStreamReader` by default. |
| [DefaultInstanceCreator](BinaryStreamWriter.md) | BinaryStreamWriter | Template specialization that tells generic `pma` factory code to use `BinaryStreamWriter::create` when constructing instances of this type. |
| [DefaultInstanceCreator](JSONStreamReader.md) | JSONStreamReader | Template specialization that tells generic `pma` factory code to use `JSONStreamReader::create` when constructing instances of this type. |
| [DefaultInstanceCreator](JSONStreamWriter.md) | JSONStreamWriter | Template specialization that tells generic `pma` factory code to use `JSONStreamWriter::create` when constructing instances of this type. |
| [DefaultInstanceDestroyer](BinaryStreamReader.md) | BinaryStreamReader | A `pma` framework specialization that tells the generic instance-destruction machinery how to release a `BinaryStreamReader` created via the factory pattern. |
| [DefaultInstanceDestroyer](BinaryStreamWriter.md) | BinaryStreamWriter | Template specialization that tells generic `pma` factory code to use `BinaryStreamWriter::destroy` when releasing instances of this type. |
| [DefaultInstanceDestroyer](JSONStreamReader.md) | JSONStreamReader | Template specialization that tells generic `pma` factory code to use `JSONStreamReader::destroy` when releasing instances of this type. |
| [DefaultInstanceDestroyer](JSONStreamWriter.md) | JSONStreamWriter | Template specialization that tells generic `pma` factory code to use `JSONStreamWriter::destroy` when releasing instances of this type. |

### JSON DNA stream I/O

Read and write DNA rig data in human-readable JSON format, with support for format conversion

| API | Module | Summary |
|-----|--------|---------|
| [JSONStreamReader](BinaryStreamWriter.md) | BinaryStreamWriter | A forward-declared reader type that `BinaryStreamWriter` accepts as a data source when the build supports JSON, letting a writer be populated from JSON-formatted DNA data. |
| [JSONStreamReader](JSONStreamReader.md) | JSONStreamReader | Read DNA rig data from a stream that holds the human-readable JSON representation instead of the compact binary format. |
| [JSONStreamReader](JSONStreamWriter.md) | JSONStreamWriter | Forward-declared peer type used as a data source when copying JSON DNA data into a `JSONStreamWriter` via `setFrom`. |
| [JSONStreamWriter](JSONStreamWriter.md) | JSONStreamWriter | Write DNA rig data out as human-readable JSON, and optionally populate it directly from an existing `BinaryStreamReader` or `JSONStreamReader`. |

### Load and save configuration

Configure coordinate systems, units, data layers, and transform policies for DNA I/O operations

| API | Module | Summary |
|-----|--------|---------|
| [Configuration](Configuration.md) | Configuration | Bundles all the load-time options for reading a DNA file — which layers, which LODs, and which coordinate/unit conventions to apply. |
| [CoordinateSystem](Configuration.md) | Configuration | Alias for `tdm::coord_sys`, describing the axis directions for all coordinate axes. |
| [CoordinateSystemTransformPolicy](Configuration.md) | Configuration | Controls whether loaded rig data is converted into a destination coordinate system or left as authored. |
| [DataLayer](Configuration.md) | Configuration | Bitmask enum identifying the loadable layers of a DNA rig, from the lightweight `Descriptor` up through `All`. |
| [Direction](Configuration.md) | Configuration | Alias for `tdm::axis_dir`, representing a signed direction along a coordinate axis. |
| [FaceWindingOrder](Configuration.md) | Configuration | Face vertex winding order of the geometry data, viewed along the outward surface normal. |
| [RotationDirection](Configuration.md) | Configuration | Alias for `tdm::rot_dir`, representing the sign of rotation (positive/negative) about an axis. |
| [RotationSequence](Configuration.md) | Configuration | Alias for `tdm::rot_seq`, representing the order in which axis rotations are composed (e.g. XYZ). |
| [RotationSign](Configuration.md) | Configuration | Alias for `tdm::rot_sign`, a per-axis triple of `RotationDirection` values specifying the sign convention for rotation on each axis. |
| [RotationUnit](Configuration.md) | Configuration | Unit of measurement (degrees or radians) used for rotation values in the rig. |
| [TranslationUnit](Configuration.md) | Configuration | Unit of measurement (centimeters or meters) used for translation values in the rig. |
| [UnknownLayerPolicy](Configuration.md) | Configuration | Controls whether layers the reader/writer doesn't recognize are kept or discarded. |

### Abstract DNA reader interface

Unified and stream-based interfaces for reading complete DNA documents and individual layers

| API | Module | Summary |
|-----|--------|---------|
| [Reader](Reader.md) | Reader | Aggregate reader interface combining every DNA layer's reader interface into one type. |
| [Reader](Writer.md) | Writer | A forward-declared reader type that `Writer::setFrom` accepts as the source when copying an entire DNA document into a writer. |
| [StreamReader](StreamReader.md) | StreamReader | Extends `Reader` with the ability to actually pull rig data from a stream into internal structures. |

### Abstract DNA writer interface

Unified and stream-based interfaces for writing complete DNA documents and individual layers

| API | Module | Summary |
|-----|--------|---------|
| [StreamWriter](StreamWriter.md) | StreamWriter | Abstract base defining the interface for writing DNA data to an output stream. Obtain a concrete instance via `BinaryStreamWriter` or `JSONStreamWriter` rather than subclassing directly. |
| [Writer](Writer.md) | Writer | Abstract unified write interface for all DNA data layers. Use this as the base type when you need to write a complete DNA asset — it combines geometry, behavior, machine-learned behavior, RBF, joint metadata, and twist/swing writing into a single entry point. |

### Behavioral layer read and write

Access and modify behavior data including GUI-to-control mappings, PSD expressions, and joint matrices

| API | Module | Summary |
|-----|--------|---------|
| [BehaviorReader](layers/BehaviorReader.md) | BehaviorReader | Access the behavioral data layer of a DNA file — GUI-to-raw control mappings, PSD expressions, and joint matrix structure — in read-only mode. |
| [BehaviorWriter](layers/BehaviorWriter.md) | BehaviorWriter | Write rig evaluation behavior data into a DNA asset — GUI-to-raw control mapping, PSD expressions, joint group matrices, blend shape channels, and animated maps. Do not subclass this directly; inherit from `Writer` instead. |

### Definition layer read and write

Access and modify static rig definition data including controls, joints, blend shapes, and mesh metadata

| API | Module | Summary |
|-----|--------|---------|
| [DefinitionReader](layers/DefinitionReader.md) | DefinitionReader | Read-only interface to the rig's static definition data — controls, joints, blend shape channels, animated maps, meshes, and neutral joint transforms. |
| [DefinitionWriter](layers/DefinitionWriter.md) | DefinitionWriter | Write static rig definition data — control names, joint hierarchy, blend shape channels, animated maps, and meshes — into a DNA asset. Implement via `Writer`, not directly from this class. |
| [MeshBlendShapeChannelMapping](layers/DefinitionReader.md) | DefinitionReader | Maps a blend shape channel to its owning mesh by pairing a mesh index with a blend shape channel index. |

### Character and rig metadata

Query and configure character identity, demographic archetype, rig version, and file format information

| API | Module | Summary |
|-----|--------|---------|
| [Archetype](layers/Descriptor.md) | Descriptor | Classifies a character's demographic archetype for DNA rig selection. |
| [DescriptorReader](layers/DescriptorReader.md) | DescriptorReader | Query character identity, rig configuration, coordinate conventions, LOD range, and database provenance from a loaded DNA asset. |
| [DescriptorWriter](layers/DescriptorWriter.md) | DescriptorWriter | Write character and rig metadata — identity, coordinate conventions, LOD range, and database provenance — into a DNA asset. |
| [Gender](layers/Descriptor.md) | Descriptor | Specifies the gender of a character for DNA rig selection. |
| [HeaderReader](layers/HeaderReader.md) | HeaderReader | Read the file format generation and version numbers from a DNA rig's header block. |
| [HeaderWriter](layers/HeaderWriter.md) | HeaderWriter | Set the file format generation and version fields in a rig's DNA header during a write pass. |
| [VersionInfo](version/VersionInfo.md) | VersionInfo | Query the version of the DNA library that the current binary was built against. |

### Mesh geometry

Access and modify vertex positions, normals, texture coordinates, and mesh structure

| API | Module | Summary |
|-----|--------|---------|
| [Delta](layers/Geometry.md) | Geometry | Semantic alias for `Vector3` that marks a value as a blend-shape displacement rather than a position or direction. |
| [GeometryReader](layers/GeometryReader.md) | GeometryReader | Access all geometry data — vertex positions, texture coordinates, and related mesh data — for a rig's meshes. This is the read-only side of the geometry layer; pair it with `GeometryWriter` when you need to modify mesh data. |
| [GeometryWriter](layers/GeometryWriter.md) | GeometryWriter | Write geometry data (meshes, vertex positions, normals, texture coordinates, and layouts) into a rig DNA structure. |
| [Normal](layers/Geometry.md) | Geometry | Semantic alias for `Vector3` that marks a value as a surface normal direction rather than a position or displacement. |
| [Position](layers/Geometry.md) | Geometry | Semantic alias for `Vector3` that marks a value as a spatial position rather than a direction or displacement. |
| [TextureCoordinate](layers/Geometry.md) | Geometry | A UV pair that locates a point in 2D texture space for a mesh vertex. |
| [VertexLayout](layers/Geometry.md) | Geometry | Index triple that assembles a logical mesh vertex from three separate attribute arrays — position, texture coordinate, and normal. |

### Joint transform representations

Query and configure how joint rotations, scales, and translations are encoded in the DNA file

| API | Module | Summary |
|-----|--------|---------|
| [JointBehaviorMetadataReader](layers/JointBehaviorMetadataReader.md) | JointBehaviorMetadataReader | Query the internal representation hints that govern how each joint's translation, rotation, and scale components are evaluated during rig execution. |
| [JointBehaviorMetadataWriter](layers/JointBehaviorMetadataWriter.md) | JointBehaviorMetadataWriter | Configure per-joint internal representation metadata that controls how each joint's translation, rotation, and scale components are evaluated during rig execution. |
| [RotationRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Identifies whether joint rotation data is stored as Euler angles or as a quaternion in a DNA file. |
| [ScaleRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Identifies the storage format used for joint scale data in a DNA file. |
| [TranslationRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Identifies the storage format used for joint translation data in a DNA file. |

### Pose and deformation behaviors

Configure and access radial basis function poses, machine-learned neural networks, and twist-swing transformations

| API | Module | Summary |
|-----|--------|---------|
| [ActivationFunction](layers/MachineLearnedBehavior.md) | MachineLearnedBehavior | Specify the nonlinear activation function applied at each neuron layer in a machine-learned rig behavior network. |
| [AutomaticRadius](layers/RBFBehavior.md) | RBFBehavior | Toggles whether an RBF target's radius of influence is computed automatically rather than set explicitly. |
| [MachineLearnedBehaviorExtReader](layers/MachineLearnedBehaviorExtReader.md) | MachineLearnedBehaviorExtReader | Read-only accessors to the neural network extension data associated with a rig — the per-model operation graph (operation sets, operation types, static parameters, and dependency indices) that `MachineLearnedBehaviorReader` itself does not expose. |
| [MachineLearnedBehaviorExtWriter](layers/MachineLearnedBehaviorExtWriter.md) | MachineLearnedBehaviorExtWriter | Write-only accessors for the operation instance data associated with a rig — building up the same per-model operation graph that `MachineLearnedBehaviorExtReader` exposes for reading. |
| [MachineLearnedBehaviorOperationType](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Enumerates the kind of computation a single ML operation performs within a neural network layer. |
| [MachineLearnedBehaviorParameterKey](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Enumerates the static parameter keys an ML operation can reference, covering joint transform types, coordinate axes, rotation signs, and rotation sequence. |
| [MachineLearnedBehaviorReader](layers/MachineLearnedBehaviorReader.md) | MachineLearnedBehaviorReader | Read-only accessors to the neural network data associated with a rig — ML control names, neural network counts, which networks are active at each LOD, and which networks drive which mesh region. |
| [MachineLearnedBehaviorWriter](layers/MachineLearnedBehaviorWriter.md) | MachineLearnedBehaviorWriter | Write-only accessors for the neural network data associated with a rig — defining ML control names, the neural-network index lists, which networks run at each LOD, and mesh-region associations. |
| [RBFBehaviorReader](layers/RBFBehaviorReader.md) | RBFBehaviorReader | Read-only accessors to the RBF (radial basis function) pose data associated with a rig — pose names, their driven joint/blend-shape/animated-map outputs, output values, scale, and the controls that feed them. |
| [RBFBehaviorWriter](layers/RBFBehaviorWriter.md) | RBFBehaviorWriter | Write-only accessors for the RBF pose data associated with a rig — pose names, scale, control names, and the input/output control indices that connect poses to the rest of the rig. |
| [RBFDistanceMethod](layers/RBFBehavior.md) | RBFBehavior | Selects how the distance between an input and an RBF pose target is measured. |
| [RBFFunctionType](layers/RBFBehavior.md) | RBFBehavior | Selects the radial falloff function applied to distance when computing an RBF target's contribution weight. |
| [RBFNormalizeMethod](layers/RBFBehavior.md) | RBFBehavior | Selects when contribution weights from an RBF solve are normalized to sum to a consistent total. |
| [RBFSolverType](layers/RBFBehavior.md) | RBFBehavior | Selects the algorithm used to combine RBF (radial basis function) pose target contributions into a final weight. |
| [TwistAxis](layers/Twist.md) | Twist | Identifies which local axis (X, Y, or Z) a twist or swing transformation is measured around. |
| [TwistSwingBehaviorReader](layers/TwistSwingBehaviorReader.md) | TwistSwingBehaviorReader | Read-only accessors to the swing and twist data associated with a rig — which joint rotations drive twist/swing transformations, which joints they output to, and the blend weights applied. |
| [TwistSwingBehaviorWriter](layers/TwistSwingBehaviorWriter.md) | TwistSwingBehaviorWriter | Write-only accessors to the swing and twist data associated with a rig — defining twist/swing parameter groups, their axis, driver control indices, driven joints, and blend weights. |

### Foundation types

Basic mathematical and utility types used throughout the DNA API

| API | Module | Summary |
|-----|--------|---------|
| [Vector3](types/Vector3.md) | Vector3 | A plain 3-component float vector (x, y, z) with the standard set of component-wise arithmetic operators. |

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
