# API Reference — `tdm`

_93 entries across 8 modules._

## By Task

### Vector and quaternion math

Compute vector and quaternion operations including dot and cross products, magnitude, normalization, and quaternion conjugates

| API | Module | Summary |
|-----|--------|---------|
| [conjugate](Computations.md) | Computations | Compute the conjugate of a quaternion by negating its vector (imaginary) components. |
| [cross](Computations.md) | Computations | Compute the cross product of two 3D vectors, producing a vector perpendicular to both. |
| [dot](Computations.md) | Computations | Compute the dot product of two vectors or quaternions, a scalar measure of how aligned they are. |
| [length](Computations.md) | Computations | Compute the magnitude (length) of a vector or quaternion. |
| [negate](Computations.md) | Computations | Compute the negation of a vector, matrix, or quaternion, flipping the sign of every component. |
| [normalize](Computations.md) | Computations | Scale a vector or quaternion to unit length while preserving its direction. |

### Rotation representation and conversion

Create and convert between quaternions and Euler angles for representing 3D rotations

| API | Module | Summary |
|-----|--------|---------|
| [euler_to_quat](Quat.md) | Quat | Build a rotation quaternion from Euler angles in a specific axis order, without going through an intermediate rotation matrix. |
| [quat](Quat.md) | Quat | A quaternion representing a rotation in 3D space, stored as `x`, `y`, `z`, `w` components. |
| [quat](Types.md) | Types | Generic quaternion template used to represent 3D rotations without the gimbal-lock and interpolation problems of Euler angles. |
| [quat_to_euler](Quat.md) | Quat | Recover Euler angles from a quaternion in a specific axis order — the inverse of `euler_to_quat`. |

### Rotation interpolation

Smoothly interpolate between rotations using linear and spherical interpolation methods

| API | Module | Summary |
|-----|--------|---------|
| [lerp](Computations.md) | Computations | Linearly interpolate between two quaternions. |
| [slerp](Computations.md) | Computations | Spherically interpolate between two rotations, moving at a constant angular velocity. |

### Quaternion and matrix inversion

Compute inverses of quaternions and matrices to undo their transforms

| API | Module | Summary |
|-----|--------|---------|
| [inverse](Computations.md) | Computations | Compute the inverse of a quaternion or a square matrix, undoing its transform. |
| [inverse](Mat.md) | Mat | Forward declaration of the matrix inverse operation. |

### Matrix construction and manipulation

Create, transform, and inspect matrix properties including transpose, determinant, trace, and element-wise operations

| API | Module | Summary |
|-----|--------|---------|
| [adjoint](Computations.md) | Computations | Compute the adjoint (adjugate) matrix, the transpose of the cofactor matrix. |
| [applied](Mat.md) | Mat | Apply a function to every element of a matrix, returning a new matrix with the results — the source matrix is left unchanged. |
| [determinant](Computations.md) | Computations | Compute the determinant of a square matrix. |
| [is_all_scalar](Mat.md) | Mat | A compile-time trait that checks whether a parameter pack consists entirely of the same scalar type. |
| [mat](Mat.md) | Mat | A fixed-size, generic `R`×`C` matrix template parameterized on element type, used throughout `tdm` for linear transforms. |
| [mat](Types.md) | Types | Generic matrix template parameterized by row count, column count, and element type. |
| [minor](Computations.md) | Computations | Extract the minor matrix formed by removing row `i` and column `j` from a square matrix. |
| [trace](Computations.md) | Computations | Compute the trace of a square matrix, the sum of its diagonal elements. |
| [transpose](Computations.md) | Computations | Flip a matrix's rows and columns. |
| [transpose](Mat.md) | Mat | Forward declaration of the matrix transpose operation, used internally by `mat::from_columns`. |

### Matrix decomposition and linear systems

Decompose matrices and solve linear systems using LU decomposition with partial pivoting

| API | Module | Summary |
|-----|--------|---------|
| [decompose](Computations.md) | Computations | Perform an in-place LU decomposition of a square matrix with partial pivoting, based on the algorithm in *Numerical Recipes in C*. |
| [substitute](Computations.md) | Computations | Solve a linear system in place using the LU-decomposed matrix and permutation produced by `decompose`. |

### Angle representation and conversion

Work with strongly-typed angle values and convert between degrees and radians

| API | Module | Summary |
|-----|--------|---------|
| [ang](Ang.md) | Ang | A strongly-typed angle value tagged with its unit (degrees or radians) at compile time. |
| [ang](Types.md) | Types | Generic angle type parameterized by numeric type and angle unit (radians or degrees). |
| [degrees](Ang.md) | Ang | Convert an angle expressed in radians to degrees. |
| [degrees](Types.md) | Types | Internal tag type used to mark an `ang<T, TUnit>` instantiation as holding a degree value. |
| [pi](Ang.md) | Ang | Get the value of π as a compile-time constant for use in angle conversions and trigonometric calculations. |
| [radians](Ang.md) | Ang | Convert an angle expressed in degrees to radians. |
| [radians](Types.md) | Types | Internal tag type used to mark an `ang<T, TUnit>` instantiation as holding a radian value. |

### Coordinate system definitions

Define and query 3D coordinate system orientations by naming axis directions

| API | Module | Summary |
|-----|--------|---------|
| [axis_dir](Types.md) | Types | Enumerates the six named spatial directions (left, right, up, down, front, back) used to describe how a coordinate system's axes map onto real-world directions. |
| [axis_vector](CoordSys.md) | CoordSys | Convert a named axis direction (`right`, `up`, `front`, etc.) into its unit vector in the canonical coordinate system. |
| [chirality](Types.md) | Types | Enumerates the handedness (left- or right-handed) of a coordinate system. |
| [coord_sys](CoordSys.md) | CoordSys | Describes a 3D coordinate system by naming what each of its X, Y, and Z axes points toward (e.g. right, up, front). |
| [coord_sys](Types.md) | Types | Forward-declared type describing a coordinate system's axis directions, handedness, and rotation conventions as a single value. |

### Rotations

Create, convert, and compose rotations using Euler angles, quaternions, or rotation matrices

| API | Module | Summary |
|-----|--------|---------|
| [euler2mat](Transforms.md) | Transforms | Dispatch to the correct `euler_to_mat` specialization based on a runtime `rot_seq` value. |
| [euler_to_mat](Transforms.md) | Transforms | Build a per-axis rotation matrix from Euler angles by composing `rotx`, `roty`, `rotz` and `rot_mat` for a rotation order fixed at compile time via the `order` template parameter. |
| [mat2euler](Transforms.md) | Transforms | Dispatch to the correct `mat_to_euler` specialization based on a runtime `rot_seq` value. |
| [mat_to_euler](Transforms.md) | Transforms | Decompose a rotation matrix into Euler angles for a rotation order fixed at compile time — the inverse of `euler_to_mat`. |
| [rad3](Types.md) | Types | A 3-component vector of radian-unit angles, used for Euler rotations expressed in radians. |
| [rot_dir](Types.md) | Types | Enumerates whether a rotation is applied in the positive or negative direction around an axis. |
| [rot_mat](Transforms.md) | Transforms | Multiply three per-axis rotation matrices together in the order dictated by a `rot_seq` specialization. |
| [rot_seq](Types.md) | Types | Enumerates the six orderings in which Euler-angle rotations around the X, Y, and Z axes can be composed. |
| [rot_sign](Types.md) | Types | Bundles the rotation direction convention for all three axes (x, y, z) of a coordinate system. |
| [rotate](Transforms.md) | Transforms | Build a 4x4 rotation transform, either from an arbitrary axis and angle (Rodrigues' rotation formula) or from Euler angles via `impl::euler2mat`, and optionally compose it onto an existing `mat4`. |
| [rotx](Transforms.md) | Transforms | Build the elementary rotation matrix for a rotation about the X axis. |
| [roty](Transforms.md) | Transforms | Build the elementary rotation matrix for a rotation about the Y axis. |
| [rotz](Transforms.md) | Transforms | Build the elementary rotation matrix for a rotation about the Z axis. |

### Coordinate system conversion

Transform vectors and transforms between different coordinate systems and bases

| API | Module | Summary |
|-----|--------|---------|
| [change_of_basis](Transforms.md) | Transforms | Build the 3x3 matrix `C` such that `v_dst = v_src * C` for row-vectors, converting between two coordinate systems' bases. |
| [convert_direction](Transforms.md) | Transforms | Reorient a direction vector (normal, tangent, velocity) into a different coordinate space — not a position. For positions, use `convert_position`. |
| [convert_position](Transforms.md) | Transforms | Apply a change-of-basis matrix to a position (or translation) vector to move it into a different coordinate system. |
| [convert_rotation](Transforms.md) | Transforms | Convert a rotation expressed as Euler angles in one coordinate system and rotation convention into Euler angles in another, by round-tripping through a rotation matrix and applying a similarity transform. |
| [convert_scale](Transforms.md) | Transforms | Convert a scale vector between coordinate systems, permuting components by the change-of-basis matrix and applying `sign_policy` to decide whether to keep or discard each component's sign. |
| [sign_policy](Transforms.md) | Transforms | An enum controlling how `convert_scale` treats the sign of each scale component when converting between coordinate systems. |

### Transform matrices

Build translation, rotation, and scaling matrices

| API | Module | Summary |
|-----|--------|---------|
| [scale](Transforms.md) | Transforms | Build a diagonal scaling matrix from per-axis factors, or apply that scale to an existing matrix. |
| [translate](Transforms.md) | Transforms | Build an `(L+1)x(L+1)` homogeneous translation matrix, writing the position components into the last row of the identity matrix. |

### Vectors and matrices

Fundamental fixed-size vector and matrix types for 3D computations

| API | Module | Summary |
|-----|--------|---------|
| [mat3](Types.md) | Types | A generic 3x3 matrix, parameterized only by element type `T`. |
| [mat4](Types.md) | Types | A generic 4x4 matrix, parameterized only by element type `T`. |
| [rad](Types.md) | Types | Alias for an angle value held in radians, `ang<T, impl::radians>`. |
| [vec](Types.md) | Types | The base fixed-size vector template, parameterized on component count `L` and scalar type `T`. |
| [vec](Vec.md) | Vec | Fixed-size, dimension-generic vector type used throughout `tdm` for positions, directions, and other L-dimensional float/int data. |
| [vec2](Types.md) | Types | A 2-component vector, for values like 2D coordinates or UVs. |
| [vec3](Types.md) | Types | A 3-component vector, used throughout `tdm` for positions, directions, and scale factors. |
| [vec4](Types.md) | Types | A 4-component vector, used for homogeneous coordinates (e.g. `[x, y, z, w]`) and quaternion-adjacent storage. |

### Fast math

Optimized mathematical approximations for performance-critical operations

| API | Module | Summary |
|-----|--------|---------|
| [fastasin](Transforms.md) | Transforms | Compute an approximate `asin(value)` using a 7-degree minimax polynomial, trading a small amount of accuracy for speed versus `std::asin`. |

### Vectors

Vector types ranging from 2 to 4 components in floating-point and integer variants, plus generic templates and dimension utilities

| API | Module | Summary |
|-----|--------|---------|
| [dim_t](Types.md) | Types | An alias for `std::size_t`, used as the dimension/index type for `vec` and `mat` templates. |
| [fvec](Types.md) | Types | Generic alias template for a floating-point vector of a given dimension. |
| [fvec2](Types.md) | Types | A 2-component floating-point vector. |
| [fvec3](Types.md) | Types | A 3-component floating-point vector. |
| [fvec4](Types.md) | Types | A 4-component floating-point vector. |
| [ivec](Types.md) | Types | Generic alias template for an integer vector of a given dimension. |
| [ivec2](Types.md) | Types | A 2-component integer vector. |
| [ivec3](Types.md) | Types | A 3-component integer vector. |
| [ivec4](Types.md) | Types | A 4-component integer vector. |

### Matrices

Matrix types ranging from 2x2 to 4x4 in floating-point and integer variants, plus generic templates

| API | Module | Summary |
|-----|--------|---------|
| [fmat](Types.md) | Types | Generic alias template for a floating-point matrix of given row and column counts. |
| [fmat2](Types.md) | Types | A 2x2 floating-point matrix. |
| [fmat3](Types.md) | Types | A 3x3 floating-point matrix. |
| [fmat4](Types.md) | Types | A 4x4 floating-point matrix, the standard type for affine transforms. |
| [imat](Types.md) | Types | Generic alias template for an integer matrix of given row and column counts. |
| [imat2](Types.md) | Types | A 2x2 integer matrix. |
| [imat3](Types.md) | Types | A 3x3 integer matrix. |
| [imat4](Types.md) | Types | A 4x4 integer matrix. |
| [mat2](Types.md) | Types | A generic 2x2 matrix, parameterized only by element type `T`. |

### Rotations and angles

Angle types, quaternions, and Euler rotation vectors for representing 3D orientations in radians or degrees

| API | Module | Summary |
|-----|--------|---------|
| [deg](Types.md) | Types | Alias for an angle value held in degrees, `ang<T, impl::degrees>`. |
| [deg3](Types.md) | Types | A 3-component vector of degree-unit angles, used for Euler rotations expressed in degrees. |
| [fdeg](Types.md) | Types | Single-precision degree-based angle alias, used wherever rotations are expressed in degrees instead of radians. |
| [fdeg3](Types.md) | Types | A 3-component vector of single-precision degree angles, typically used to represent Euler-angle rotations (pitch/yaw/roll) in degrees. |
| [fquat](Types.md) | Types | Single-precision quaternion type, the standard rotation representation used throughout `tdm` for 3D orientation. |
| [frad](Types.md) | Types | Single-precision radian-based angle alias, used wherever rotations are expressed in radians instead of degrees. |
| [frad3](Types.md) | Types | A 3-component vector of single-precision radian angles, typically used to represent Euler-angle rotations in radians. |

## All Modules

| Module | File | Entries |
|--------|------|---------|
| Ang | [Ang.md](Ang.md) | 4 |
| Computations | [Computations.md](Computations.md) | 16 |
| CoordSys | [CoordSys.md](CoordSys.md) | 2 |
| Mat | [Mat.md](Mat.md) | 5 |
| Quat | [Quat.md](Quat.md) | 3 |
| Transforms | [Transforms.md](Transforms.md) | 18 |
| Types | [Types.md](Types.md) | 44 |
| Vec | [Vec.md](Vec.md) | 1 |
