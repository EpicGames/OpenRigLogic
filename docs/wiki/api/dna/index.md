# API Reference — `dna`

_92 entries across 37 modules._

## By Task

### DNA stream I/O

Read and write DNA assets in binary or JSON format.

| API | Module | Summary |
|-----|--------|---------|
| [BinaryStreamReader](BinaryStreamReader.md) | BinaryStreamReader | Reads DNA data from binary format |
| [BinaryStreamWriter](BinaryStreamWriter.md) | BinaryStreamWriter | Writes DNA data to binary format |
| [JSONStreamReader](JSONStreamReader.md) | JSONStreamReader | Reads DNA data from JSON format |
| [JSONStreamWriter](JSONStreamWriter.md) | JSONStreamWriter | Writes DNA data to JSON format |
| [Reader](Reader.md) | Reader | Main DNA reader interface |
| [Writer](Writer.md) | Writer | Main DNA writer interface |
| [StreamReader](StreamReader.md) | StreamReader | Abstract base for DNA stream readers |
| [StreamWriter](StreamWriter.md) | StreamWriter | Abstract base for DNA stream writers |

### Configuration & setup

Control load-time options and coordinate transforms.

| API | Module | Summary |
|-----|--------|---------|
| [Configuration](Configuration.md) | Configuration | Aggregate load-time options for DNA reader |
| [CoordinateSystem](Configuration.md) | Configuration | Complete spatial coordinate frame type |
| [CoordinateSystemTransformPolicy](Configuration.md) | Configuration | Decide whether to convert coordinates |
| [DataLayer](Configuration.md) | Configuration | Selects which data layers to load |
| [UnknownLayerPolicy](Configuration.md) | Configuration | Controls handling of unrecognised layers |
| [RotationSequence](Configuration.md) | Configuration | Euler rotation order during conversion |
| [RotationSign](Configuration.md) | Configuration | Per-axis rotation sign during conversion |
| [RotationUnit](Configuration.md) | Configuration | Unit for rotations (radians or degrees) |
| [RotationDirection](Configuration.md) | Configuration | Direction of rotation (clockwise or counter-clockwise) |
| [TranslationUnit](Configuration.md) | Configuration | Unit for translations (meters or centimeters) |
| [Direction](Configuration.md) | Configuration | Axis direction values for coordinate systems |
| [FaceWindingOrder](Configuration.md) | Configuration | Target face winding order |

### DNA layers

Low-level data layer readers and writers.

| API | Module | Summary |
|-----|--------|---------|
| [BehaviorReader](layers/BehaviorReader.md) | BehaviorReader | Reads joint and blend-shape behavior |
| [BehaviorWriter](layers/BehaviorWriter.md) | BehaviorWriter | Writes joint and blend-shape behavior |
| [DefinitionReader](layers/DefinitionReader.md) | DefinitionReader | Reads character definition metadata |
| [DefinitionWriter](layers/DefinitionWriter.md) | DefinitionWriter | Writes character definition metadata |
| [DescriptorReader](layers/DescriptorReader.md) | DescriptorReader | Reads character descriptor |
| [DescriptorWriter](layers/DescriptorWriter.md) | DescriptorWriter | Writes character descriptor |
| [GeometryReader](layers/GeometryReader.md) | GeometryReader | Reads mesh geometry data |
| [GeometryWriter](layers/GeometryWriter.md) | GeometryWriter | Writes mesh geometry data |
| [HeaderReader](layers/HeaderReader.md) | HeaderReader | Reads file header metadata |
| [HeaderWriter](layers/HeaderWriter.md) | HeaderWriter | Writes file header metadata |

### Descriptor & metadata

Character and mesh description structures.

| API | Module | Summary |
|-----|--------|---------|
| [Archetype](layers/Descriptor.md) | Descriptor | Character archetype enumeration |
| [Gender](layers/Descriptor.md) | Descriptor | Gender classification enumeration |
| [Delta](layers/Geometry.md) | Geometry | Shape deformation delta structure |
| [Normal](layers/Geometry.md) | Geometry | Normal vector structure |
| [Position](layers/Geometry.md) | Geometry | Vertex position structure |
| [TextureCoordinate](layers/Geometry.md) | Geometry | Texture UV coordinate structure |
| [VertexLayout](layers/Geometry.md) | Geometry | Vertex data layout descriptor |

### Joint behaviors

Joint transformation and behavior evaluation.

| API | Module | Summary |
|-----|--------|---------|
| [JointBehaviorMetadata](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | Metadata about joint transformation representations |
| [JointBehaviorMetadataReader](layers/JointBehaviorMetadataReader.md) | JointBehaviorMetadataReader | Reads joint behavior metadata |
| [JointBehaviorMetadataWriter](layers/JointBehaviorMetadataWriter.md) | JointBehaviorMetadataWriter | Writes joint behavior metadata |
| [RotationRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | How rotations are represented |
| [TranslationRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | How translations are represented |
| [ScaleRepresentation](layers/JointBehaviorMetadata.md) | JointBehaviorMetadata | How scales are represented |

### Advanced behaviors

RBF, ML-based, and twist-swing joint behaviors.

| API | Module | Summary |
|-----|--------|---------|
| [RBFBehavior](layers/RBFBehavior.md) | RBFBehavior | Radial basis function behavior configuration |
| [RBFBehaviorReader](layers/RBFBehaviorReader.md) | RBFBehaviorReader | Reads RBF behavior data |
| [RBFBehaviorWriter](layers/RBFBehaviorWriter.md) | RBFBehaviorWriter | Writes RBF behavior data |
| [MachineLearnedBehavior](layers/MachineLearnedBehavior.md) | MachineLearnedBehavior | Machine-learned behavior configuration |
| [MachineLearnedBehaviorReader](layers/MachineLearnedBehaviorReader.md) | MachineLearnedBehaviorReader | Reads ML behavior data |
| [MachineLearnedBehaviorWriter](layers/MachineLearnedBehaviorWriter.md) | MachineLearnedBehaviorWriter | Writes ML behavior data |
| [ActivationFunction](layers/MachineLearnedBehavior.md) | MachineLearnedBehavior | Neural network activation function type |
| [MachineLearnedBehaviorExt](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Extended ML behavior configuration |
| [MachineLearnedBehaviorExtReader](layers/MachineLearnedBehaviorExtReader.md) | MachineLearnedBehaviorExtReader | Reads extended ML behavior |
| [MachineLearnedBehaviorExtWriter](layers/MachineLearnedBehaviorExtWriter.md) | MachineLearnedBehaviorExtWriter | Writes extended ML behavior |
| [MachineLearnedBehaviorOperationType](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Type of ML operation |
| [MachineLearnedBehaviorParameterKey](layers/MachineLearnedBehaviorExt.md) | MachineLearnedBehaviorExt | Key for ML behavior parameters |
| [TwistAxis](layers/Twist.md) | Twist | Axis for twist joint rotation |
| [TwistSwingBehaviorReader](layers/TwistSwingBehaviorReader.md) | TwistSwingBehaviorReader | Reads twist-swing behavior |
| [TwistSwingBehaviorWriter](layers/TwistSwingBehaviorWriter.md) | TwistSwingBehaviorWriter | Writes twist-swing behavior |

### RBF configuration

Radial basis function solver parameters.

| API | Module | Summary |
|-----|--------|---------|
| [AutomaticRadius](layers/RBFBehavior.md) | RBFBehavior | Automatic radius calculation mode |
| [RBFDistanceMethod](layers/RBFBehavior.md) | RBFBehavior | Distance metric for RBF solver |
| [RBFFunctionType](layers/RBFBehavior.md) | RBFBehavior | RBF basis function type |
| [RBFNormalizeMethod](layers/RBFBehavior.md) | RBFBehavior | Normalization method for RBF outputs |
| [RBFSolverType](layers/RBFBehavior.md) | RBFBehavior | RBF solver algorithm type |

### Types & utilities

Mathematical and utility types.

| API | Module | Summary |
|-----|--------|---------|
| [Vector3](types/Vector3.md) | Vector3 | 3-component floating-point vector |
| [VersionInfo](version/VersionInfo.md) | VersionInfo | DNA file format version information |
| [DefaultInstanceCreator](BinaryStreamReader.md) | BinaryStreamReader | Creates default reader instances |
| [DefaultInstanceDestroyer](BinaryStreamReader.md) | BinaryStreamReader | Destroys reader instances |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| BehaviorReader | [layers/BehaviorReader.md](layers/BehaviorReader.md) | 1 |
| BehaviorWriter | [layers/BehaviorWriter.md](layers/BehaviorWriter.md) | 1 |
| BinaryStreamReader | [BinaryStreamReader.md](BinaryStreamReader.md) | 3 |
| BinaryStreamWriter | [BinaryStreamWriter.md](BinaryStreamWriter.md) | 3 |
| Configuration | [Configuration.md](Configuration.md) | 12 |
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
| JSONStreamReader | [JSONStreamReader.md](JSONStreamReader.md) | 4 |
| JSONStreamWriter | [JSONStreamWriter.md](JSONStreamWriter.md) | 6 |
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
| Reader | [Reader.md](Reader.md) | 1 |
| StreamReader | [StreamReader.md](StreamReader.md) | 1 |
| StreamWriter | [StreamWriter.md](StreamWriter.md) | 1 |
| Twist | [layers/Twist.md](layers/Twist.md) | 1 |
| TwistSwingBehaviorReader | [layers/TwistSwingBehaviorReader.md](layers/TwistSwingBehaviorReader.md) | 1 |
| TwistSwingBehaviorWriter | [layers/TwistSwingBehaviorWriter.md](layers/TwistSwingBehaviorWriter.md) | 1 |
| Vector3 | [types/Vector3.md](types/Vector3.md) | 1 |
| VersionInfo | [version/VersionInfo.md](version/VersionInfo.md) | 1 |
| Writer | [Writer.md](Writer.md) | 1 |
