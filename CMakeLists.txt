cmake_minimum_required(VERSION 3.15)

################################################
# Project setup
set(RL riglogic4)
set(RL_VERSION 13.2.7)
# Prevent in-source-tree builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
# Create compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

project(RigLogic4 VERSION ${RL_VERSION} LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS AND (CMAKE_GENERATOR MATCHES "Make|Ninja") AND (NOT DEFINED CME_COMPILATION_DATABASE_PATH))
    set(CME_COMPILATION_DATABASE_PATH "${PROJECT_BINARY_DIR}/compile_commands.json" CACHE INTERNAL "" FORCE)
endif()

set(CMAKE_MODULES_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULES_ROOT_DIR})

include(FetchContent)
function(module_exists module_name)
    include(${module_name} OPTIONAL RESULT_VARIABLE found)
    set("${module_name}_FOUND" ${found} PARENT_SCOPE)
endfunction()

# Make custom cmake modules available
module_exists(CMakeModulesExtra)
if(NOT CMakeModulesExtra_FOUND)
    include(CMakeModulesExtraLoader)
endif()

include(CMakeModulesExtra)
list(APPEND CMAKE_MODULE_PATH ${CMakeModulesExtra_DIRS})

option(READ_ONLY_SOURCE_TREE "Prevent autogeneration of files in source tree" OFF)

################################################
# Generate version information header
if (NOT READ_ONLY_SOURCE_TREE)
    include(VersionHeader)
    generate_version_header(
        OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/include/riglogic/version/Version.h"
        PREFIX "RL"
        MAJOR ${PROJECT_VERSION_MAJOR}
        MINOR ${PROJECT_VERSION_MINOR}
        PATCH ${PROJECT_VERSION_PATCH})
endif()

################################################
# Generate export definitions header
if (NOT READ_ONLY_SOURCE_TREE)
    include(Symbols)
    generate_export_definitions(
        OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/include/riglogic/Defs.h"
        EXPORT_ATTR_NAME RLAPI
        BUILD_SHARED_NAME RL_BUILD_SHARED
        USE_SHARED_NAME RL_SHARED)
endif()

################################################
# Optional features
# The following options are for compile time CPU feature tests (controlling whether feature support is built in RL or not)
option(RL_AUTODETECT_SSE "Detect if SSE is supported by examining preprocessor definitions" OFF)
option(RL_BUILD_WITH_SSE "Build with SSE (disregarding autodetection results)" OFF)
option(RL_AUTODETECT_AVX "Detect if AVX is supported by examining preprocessor definitions" OFF)
option(RL_BUILD_WITH_AVX "Build with AVX (disregarding autodetection results)" OFF)
option(RL_AUTODETECT_NEON "Detect if NEON is supported by examining preprocessor definitions" OFF)
option(RL_BUILD_WITH_NEON "Build with NEON (disregarding autodetection results)" OFF)
option(RL_AUTODETECT_HALF_FLOATS "Detect if half-floats are supported by examining preprocessor definitions" OFF)
option(RL_BUILD_WITH_HALF_FLOATS "Use half-floats (requires F16C on x86 and FP16 on ARM)" OFF)
# Runtime feature detection will test the above enabled features
option(RL_DISABLE_RUNTIME_FEATURE_DETECTION "Runtime CPU feature detection is on, regardless of the above specified options (unless turned off here)" OFF)
option(RL_BUILD_WITH_XYZ_ROTATION_ORDER "Build with XYZ rotation order support (turn this off to reduce binary size, needed only if rotations need to be output in this order)" ON)
option(RL_BUILD_WITH_XZY_ROTATION_ORDER "Build with XZY rotation order support (turn this off to reduce binary size, needed only if rotations need to be output in this order)" ON)
option(RL_BUILD_WITH_YXZ_ROTATION_ORDER "Build with YXZ rotation order support (turn this off to reduce binary size, needed only if rotations need to be output in this order)" ON)
option(RL_BUILD_WITH_YZX_ROTATION_ORDER "Build with YZX rotation order support (turn this off to reduce binary size, needed only if rotations need to be output in this order)" ON)
option(RL_BUILD_WITH_ZXY_ROTATION_ORDER "Build with ZXY rotation order support (turn this off to reduce binary size, needed only if rotations need to be output in this order)" ON)
option(RL_BUILD_WITH_ZYX_ROTATION_ORDER "Build with ZYX rotation order support (turn this off to reduce binary size, needed only if rotations need to be output in this order)" ON)
set(RL_BUILD_PYTHON_WRAPPER "" CACHE STRING "Build Python wrapper using the specified Python version.")
set(DNA_BUILD_PYTHON_WRAPPER ${RL_BUILD_PYTHON_WRAPPER} CACHE STRING "" FORCE)
if(RL_BUILD_PYTHON_WRAPPER)
    set(RL_BUILD_PIC ON)
endif()

################################################
# Source code
set(INCLUDES
    include/riglogic/Defs.h
    include/riglogic/RigLogic.h

    include/riglogic/riglogic/Configuration.h
    include/riglogic/riglogic/RigInstance.h
    include/riglogic/riglogic/RigLogic.h

    include/riglogic/types/Aliases.h

    include/riglogic/version/Version.h
    include/riglogic/version/VersionInfo.h)
set(SOURCES
    src/riglogic/TypeDefs.h

    src/riglogic/animatedmaps/AnimatedMaps.cpp
    src/riglogic/animatedmaps/AnimatedMaps.h
    src/riglogic/animatedmaps/AnimatedMapsFactory.cpp
    src/riglogic/animatedmaps/AnimatedMapsFactory.h
    src/riglogic/animatedmaps/AnimatedMapsImpl.cpp
    src/riglogic/animatedmaps/AnimatedMapsImpl.h
    src/riglogic/animatedmaps/AnimatedMapsImplOutputInstance.cpp
    src/riglogic/animatedmaps/AnimatedMapsImplOutputInstance.h
    src/riglogic/animatedmaps/AnimatedMapsNull.cpp
    src/riglogic/animatedmaps/AnimatedMapsNull.h
    src/riglogic/animatedmaps/AnimatedMapsNullOutputInstance.cpp
    src/riglogic/animatedmaps/AnimatedMapsNullOutputInstance.h
    src/riglogic/animatedmaps/AnimatedMapsOutputInstance.cpp
    src/riglogic/animatedmaps/AnimatedMapsOutputInstance.h

    src/riglogic/blendshapes/BlendShapes.cpp
    src/riglogic/blendshapes/BlendShapes.h
    src/riglogic/blendshapes/BlendShapesFactory.cpp
    src/riglogic/blendshapes/BlendShapesFactory.h
    src/riglogic/blendshapes/BlendShapesImpl.cpp
    src/riglogic/blendshapes/BlendShapesImpl.h
    src/riglogic/blendshapes/BlendShapesImplOutputInstance.cpp
    src/riglogic/blendshapes/BlendShapesImplOutputInstance.h
    src/riglogic/blendshapes/BlendShapesNull.cpp
    src/riglogic/blendshapes/BlendShapesNull.h
    src/riglogic/blendshapes/BlendShapesNullOutputInstance.cpp
    src/riglogic/blendshapes/BlendShapesNullOutputInstance.h
    src/riglogic/blendshapes/BlendShapesOutputInstance.cpp
    src/riglogic/blendshapes/BlendShapesOutputInstance.h

    src/riglogic/conditionaltable/ConditionalTable.cpp
    src/riglogic/conditionaltable/ConditionalTable.h

    src/riglogic/controls/Controls.cpp
    src/riglogic/controls/Controls.h
    src/riglogic/controls/ControlsFactory.cpp
    src/riglogic/controls/ControlsFactory.h
    src/riglogic/controls/ControlsInputInstance.cpp
    src/riglogic/controls/ControlsInputInstance.h
    src/riglogic/controls/instances/StandardControlsInputInstance.cpp
    src/riglogic/controls/instances/StandardControlsInputInstance.h

    src/riglogic/joints/JointBehaviorFilter.cpp
    src/riglogic/joints/JointBehaviorFilter.h
    src/riglogic/joints/Joints.cpp
    src/riglogic/joints/Joints.h
    src/riglogic/joints/JointsBuilder.cpp
    src/riglogic/joints/JointsBuilder.h
    src/riglogic/joints/JointsEvaluator.cpp
    src/riglogic/joints/JointsEvaluator.h
    src/riglogic/joints/JointsFactory.cpp
    src/riglogic/joints/JointsFactory.h
    src/riglogic/joints/JointsNullEvaluator.cpp
    src/riglogic/joints/JointsNullEvaluator.h
    src/riglogic/joints/JointsNullOutputInstance.cpp
    src/riglogic/joints/JointsNullOutputInstance.h
    src/riglogic/joints/JointsOutputInstance.cpp
    src/riglogic/joints/JointsOutputInstance.h

    src/riglogic/joints/cpu/CPUJointsBuilder.cpp
    src/riglogic/joints/cpu/CPUJointsBuilder.h
    src/riglogic/joints/cpu/CPUJointsEvaluator.cpp
    src/riglogic/joints/cpu/CPUJointsEvaluator.h
    src/riglogic/joints/cpu/CPUJointsOutputInstance.cpp
    src/riglogic/joints/cpu/CPUJointsOutputInstance.h

    src/riglogic/joints/cpu/bpcm/BPCMCalculationStrategy.cpp
    src/riglogic/joints/cpu/bpcm/BPCMCalculationStrategy.h
    src/riglogic/joints/cpu/bpcm/BPCMJointsBuilder.cpp
    src/riglogic/joints/cpu/bpcm/BPCMJointsBuilder.h
    src/riglogic/joints/cpu/bpcm/BPCMJointsBuilderFactory.cpp
    src/riglogic/joints/cpu/bpcm/BPCMJointsBuilderFactory.h
    src/riglogic/joints/cpu/bpcm/BPCMJointsEvaluator.cpp
    src/riglogic/joints/cpu/bpcm/BPCMJointsEvaluator.h
    src/riglogic/joints/cpu/bpcm/JointGroup.h
    src/riglogic/joints/cpu/bpcm/RotationAdapters.h
    src/riglogic/joints/cpu/bpcm/Storage.h

    src/riglogic/joints/cpu/ml/CoordinateSystemTransformer.h
    src/riglogic/joints/cpu/ml/MLJointsBuilder.cpp
    src/riglogic/joints/cpu/ml/MLJointsBuilder.h
    src/riglogic/joints/cpu/ml/MLJointsBuilderFactory.cpp
    src/riglogic/joints/cpu/ml/MLJointsBuilderFactory.h
    src/riglogic/joints/cpu/ml/MLJointsEvaluator.h
    src/riglogic/joints/cpu/ml/RotationAdapters.h

    src/riglogic/joints/cpu/quaternions/JointGroup.h
    src/riglogic/joints/cpu/quaternions/QuaternionCalculationStrategy.cpp
    src/riglogic/joints/cpu/quaternions/QuaternionCalculationStrategy.h
    src/riglogic/joints/cpu/quaternions/QuaternionJointsBuilder.cpp
    src/riglogic/joints/cpu/quaternions/QuaternionJointsBuilder.h
    src/riglogic/joints/cpu/quaternions/QuaternionJointsBuilderFactory.cpp
    src/riglogic/joints/cpu/quaternions/QuaternionJointsBuilderFactory.h
    src/riglogic/joints/cpu/quaternions/QuaternionJointsEvaluator.cpp
    src/riglogic/joints/cpu/quaternions/QuaternionJointsEvaluator.h
    src/riglogic/joints/cpu/quaternions/RotationAdapters.h

    src/riglogic/joints/cpu/twistswing/TwistSwingJointsBuilder.h
    src/riglogic/joints/cpu/twistswing/TwistSwingJointsBuilderFactory.cpp
    src/riglogic/joints/cpu/twistswing/TwistSwingJointsBuilderFactory.h
    src/riglogic/joints/cpu/twistswing/TwistSwingJointsEvaluator.h
    src/riglogic/joints/cpu/twistswing/TwistSwingSetup.h

    src/riglogic/joints/cpu/utils/JointGroupOptimizer.cpp
    src/riglogic/joints/cpu/utils/JointGroupOptimizer.h
    src/riglogic/joints/cpu/utils/LODRegion.h

    src/riglogic/ml/MachineLearnedBehavior.cpp
    src/riglogic/ml/MachineLearnedBehavior.h
    src/riglogic/ml/MachineLearnedBehaviorEvaluator.cpp
    src/riglogic/ml/MachineLearnedBehaviorEvaluator.h
    src/riglogic/ml/MachineLearnedBehaviorFactory.cpp
    src/riglogic/ml/MachineLearnedBehaviorFactory.h
    src/riglogic/ml/MachineLearnedBehaviorNullEvaluator.cpp
    src/riglogic/ml/MachineLearnedBehaviorNullEvaluator.h
    src/riglogic/ml/MachineLearnedBehaviorNullOutputInstance.cpp
    src/riglogic/ml/MachineLearnedBehaviorNullOutputInstance.h
    src/riglogic/ml/MachineLearnedBehaviorOutputInstance.cpp
    src/riglogic/ml/MachineLearnedBehaviorOutputInstance.h
    src/riglogic/ml/cpu/CPUMachineLearnedBehaviorEvaluator.cpp
    src/riglogic/ml/cpu/CPUMachineLearnedBehaviorEvaluator.h
    src/riglogic/ml/cpu/CPUMachineLearnedBehaviorFactory.cpp
    src/riglogic/ml/cpu/CPUMachineLearnedBehaviorFactory.h
    src/riglogic/ml/cpu/CPUMachineLearnedBehaviorOutputInstance.cpp
    src/riglogic/ml/cpu/CPUMachineLearnedBehaviorOutputInstance.h
    src/riglogic/ml/cpu/NeuralNet.h
    src/riglogic/ml/cpu/Operation.cpp
    src/riglogic/ml/cpu/Operation.h
    src/riglogic/ml/cpu/layers/LayerEvaluator.h
    src/riglogic/ml/cpu/layers/LeakyReLULayerEvaluator.h
    src/riglogic/ml/cpu/layers/LinearLayerEvaluator.h
    src/riglogic/ml/cpu/layers/ReLULayerEvaluator.h
    src/riglogic/ml/cpu/layers/SigmoidLayerEvaluator.h
    src/riglogic/ml/cpu/layers/TanHLayerEvaluator.h
    src/riglogic/ml/cpu/layers/Utils.h

    src/riglogic/psdnet/PSDNet.cpp
    src/riglogic/psdnet/PSDNet.h
    src/riglogic/psdnet/PSDNetFactory.cpp
    src/riglogic/psdnet/PSDNetFactory.h
    src/riglogic/psdnet/PSDNetImpl.cpp
    src/riglogic/psdnet/PSDNetImpl.h
    src/riglogic/psdnet/PSDNetImplOutputInstance.cpp
    src/riglogic/psdnet/PSDNetImplOutputInstance.h
    src/riglogic/psdnet/PSDNetNull.cpp
    src/riglogic/psdnet/PSDNetNull.h
    src/riglogic/psdnet/PSDNetNullOutputInstance.cpp
    src/riglogic/psdnet/PSDNetNullOutputInstance.h
    src/riglogic/psdnet/PSDNetOutputInstance.cpp
    src/riglogic/psdnet/PSDNetOutputInstance.h

    src/riglogic/rbf/RBFBehavior.cpp
    src/riglogic/rbf/RBFBehavior.h
    src/riglogic/rbf/RBFBehaviorEvaluator.cpp
    src/riglogic/rbf/RBFBehaviorEvaluator.h
    src/riglogic/rbf/RBFBehaviorFactory.cpp
    src/riglogic/rbf/RBFBehaviorFactory.h
    src/riglogic/rbf/RBFBehaviorNullEvaluator.cpp
    src/riglogic/rbf/RBFBehaviorNullEvaluator.h
    src/riglogic/rbf/RBFBehaviorNullOutputInstance.cpp
    src/riglogic/rbf/RBFBehaviorNullOutputInstance.h
    src/riglogic/rbf/RBFBehaviorOutputInstance.cpp
    src/riglogic/rbf/RBFBehaviorOutputInstance.h
    src/riglogic/rbf/cpu/AdditiveRBFSolver.cpp
    src/riglogic/rbf/cpu/AdditiveRBFSolver.h
    src/riglogic/rbf/cpu/CPURBFBehaviorEvaluator.h
    src/riglogic/rbf/cpu/CPURBFBehaviorFactory.h
    src/riglogic/rbf/cpu/CPURBFBehaviorOutputInstance.cpp
    src/riglogic/rbf/cpu/CPURBFBehaviorOutputInstance.h
    src/riglogic/rbf/cpu/InterpolativeRBFSolver.cpp
    src/riglogic/rbf/cpu/InterpolativeRBFSolver.h
    src/riglogic/rbf/cpu/RBFSolver.cpp
    src/riglogic/rbf/cpu/RBFSolver.h

    src/riglogic/riglogic/ConfigurationSerializer.h
    src/riglogic/riglogic/RigInstanceImpl.h
    src/riglogic/riglogic/RigInstanceImpl.cpp
    src/riglogic/riglogic/RigLogicImpl.cpp
    src/riglogic/riglogic/RigLogicImpl.h
    src/riglogic/riglogic/RigMetadata.h

    src/riglogic/system/simd/Detect.h
    src/riglogic/system/simd/SIMD.h
    src/riglogic/system/simd/Utils.h

    src/riglogic/types/Extent.h
    src/riglogic/types/PaddedBlockView.h
    src/riglogic/types/LODSpec.h
    src/riglogic/types/bpcm/FloatArray.h
    src/riglogic/types/bpcm/Optimizer.h

    src/riglogic/utils/Extd.h
    src/riglogic/utils/Macros.h

    src/riglogic/version/RLVersionInfo.cpp)

################################################
# Add target and build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(RL_DEFAULT_LIBRARY_TYPE STATIC)
if(BUILD_SHARED_LIBS)
    set(RL_DEFAULT_LIBRARY_TYPE SHARED)
endif()

set(RL_LIBRARY_TYPE ${RL_DEFAULT_LIBRARY_TYPE} CACHE STRING "Build RigLogic as a library of type")
set_property(CACHE RL_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED MODULE)

set(RL_DEFAULT_BUILD_PIC OFF)
if(RL_LIBRARY_TYPE STREQUAL SHARED OR RL_LIBRARY_TYPE STREQUAL MODULE)
    set(RL_DEFAULT_BUILD_PIC ON)
endif()
option(RL_BUILD_PIC "Build with position independent code" ${RL_DEFAULT_BUILD_PIC})

add_library(${RL} ${RL_LIBRARY_TYPE})
add_library(RigLogic::${RL} ALIAS ${RL})

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()

set_target_properties(${RL} PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED NO
    CXX_EXTENSIONS NO
    CXX_VISIBILITY_PRESET hidden
    POSITION_INDEPENDENT_CODE ${RL_BUILD_PIC}
    SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    OUTPUT_NAME "${RL}${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")

if (RL_LIBRARY_TYPE STREQUAL SHARED)
    target_compile_definitions(${RL} PUBLIC RL_SHARED PRIVATE RL_BUILD_SHARED)
endif()

option(FORCE_DLL_ATTRIBUTE_EXPORT_IMPORT "Use __attribute__((dllexport)) style dll export/import attributes" OFF)
option(FORCE_DLL_DECLSPEC_EXPORT_IMPORT "Use __declspec(dllexport) style dll export/import attributes" OFF)
option(FORCE_DLL_VISIBILITY_EXPORT_IMPORT "Use __attribute__((visibility(\"default\"))) style dll export/import attributes" OFF)
if(FORCE_DLL_ATTRIBUTE_EXPORT_IMPORT)
    target_compile_definitions(${RL} PUBLIC FORCE_DLL_ATTRIBUTE_EXPORT_IMPORT)
endif()
if(FORCE_DLL_DECLSPEC_EXPORT_IMPORT)
    target_compile_definitions(${RL} PUBLIC FORCE_DLL_DECLSPEC_EXPORT_IMPORT)
endif()
if(FORCE_DLL_VISIBILITY_EXPORT_IMPORT)
    target_compile_definitions(${RL} PUBLIC FORCE_DLL_VISIBILITY_EXPORT_IMPORT)
endif()

include(GNUInstallDirs)

set(RL_PUBLIC_INCLUDE_DIRS
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_include_directories(${RL}
    PUBLIC
        ${RL_PUBLIC_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

option(TREAT_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
if(MSVC)
    set(CXX_FLAGS /W4 /w14061 /w14062 /w14121 /w14242 /w14245
                /w14254 /w14263 /w14265 /w14266 /w14287 /w14289 /w14296
                /w14302 /w14311 /w14365 /w14388 /w14545 /w14546 /w14547
                /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906
                /w14928 /w14987 /w14946)
    if(TREAT_WARNINGS_AS_ERRORS)
        list(APPEND CXX_FLAGS /WX)
    endif()
    set(CXX_EXTRA_FLAGS /permissive-)
else()
    set(CXX_FLAGS -Wall -Wextra -Wpedantic)
    if(TREAT_WARNINGS_AS_ERRORS)
        list(APPEND CXX_FLAGS -Werror)
    endif()
    set(CXX_EXTRA_FLAGS -Wcast-align -Wconversion -Wduplicated-branches
                -Wduplicated-cond -Wexit-time-destructors
                -Wglobal-constructors -Wlogical-op -Wmissing-noreturn
                -Wnon-virtual-dtor -Wnull-dereference -Wold-style-cast
                -Woverloaded-virtual -Wshadow -Wsign-conversion -Wunreachable-code
                -Wunused -Wuseless-cast -Wweak-vtables -Wno-unknown-pragmas -Wno-psabi)
endif()

target_compile_options(${RL} PRIVATE "${CXX_FLAGS}")
include(SupportedCompileOptions)
target_supported_compile_options(${RL} PRIVATE "${CXX_EXTRA_FLAGS}")

if (RL_LIBRARY_TYPE STREQUAL STATIC)
    set(RL_FEATURE_VISIBILITY PUBLIC)
else()
    set(RL_FEATURE_VISIBILITY PRIVATE)
endif()

if(RL_AUTODETECT_SSE)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_AUTODETECT_SSE)
endif()

if(RL_BUILD_WITH_SSE)
    set(TERSE_ENABLE_SSE ON)
    set(TRIMD_ENABLE_SSE ON)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_SSE)
    # Clang can disguise itself as MSVC on windows (clang-cl)
    if((NOT MSVC) OR (CMAKE_CXX_COMPILER_ID MATCHES "(C|c)lang"))
        target_compile_options(${RL} ${RL_FEATURE_VISIBILITY} "-msse4.2")
    endif()
endif()

if(RL_BUILD_WITH_AVX)
    set(TRIMD_ENABLE_AVX ON)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_AVX)
    if(MSVC)
        target_compile_options(${RL} ${RL_FEATURE_VISIBILITY} "/arch:AVX")
    else()
        target_compile_options(${RL} ${RL_FEATURE_VISIBILITY} "-mavx")
    endif()
endif()

if(RL_AUTODETECT_NEON)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_AUTODETECT_NEON)
endif()

if(RL_BUILD_WITH_NEON)
    set(TRIMD_ENABLE_NEON ON)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_NEON)
    if(NOT MSVC)
        target_compile_options(${RL} ${RL_FEATURE_VISIBILITY} "-mfpu=neon")
    endif()
endif()

if(RL_AUTODETECT_HALF_FLOATS)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_AUTODETECT_HALF_FLOATS)
endif()

if(RL_BUILD_WITH_HALF_FLOATS)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_HALF_FLOATS)
    if(RL_BUILD_WITH_SSE OR RL_BUILD_WITH_AVX)
        set(TRIMD_ENABLE_F16C ON)
        # Clang can disguise itself as MSVC on windows (clang-cl)
        if((NOT MSVC) OR (CMAKE_CXX_COMPILER_ID MATCHES "(C|c)lang"))
            target_compile_options(${RL} ${RL_FEATURE_VISIBILITY} "-mf16c")
        endif()
    elseif(RL_BUILD_WITH_NEON)
        set(TRIMD_ENABLE_NEON_FP16 ON)
        # Clang can disguise itself as MSVC on windows (clang-cl)
        if((NOT MSVC) OR (CMAKE_CXX_COMPILER_ID MATCHES "(C|c)lang"))
            target_compile_options(${RL} ${RL_FEATURE_VISIBILITY} "-mfpu=neon-fp16" "-mfp16-format=ieee")
        endif()
    endif()
endif()

if(RL_DISABLE_RUNTIME_FEATURE_DETECTION)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_DISABLE_RUNTIME_FEATURE_DETECTION)
else()
    set(TRIMD_ENABLE_RUNTIME_FEATURE_DETECTION ON)
endif()

if(RL_BUILD_WITH_XYZ_ROTATION_ORDER)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_XYZ_ROTATION_ORDER)
endif()

if(RL_BUILD_WITH_XZY_ROTATION_ORDER)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_XZY_ROTATION_ORDER)
endif()

if(RL_BUILD_WITH_YXZ_ROTATION_ORDER)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_YXZ_ROTATION_ORDER)
endif()

if(RL_BUILD_WITH_YZX_ROTATION_ORDER)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_YZX_ROTATION_ORDER)
endif()

if(RL_BUILD_WITH_ZXY_ROTATION_ORDER)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_ZXY_ROTATION_ORDER)
endif()

if(RL_BUILD_WITH_ZYX_ROTATION_ORDER)
    target_compile_definitions(${RL} ${RL_FEATURE_VISIBILITY} RL_BUILD_WITH_ZYX_ROTATION_ORDER)
endif()

################################################
# Dependencies
include(RLDependencies)

set(ADAPTABLE_INCLUDES)
foreach(hdr IN LISTS INCLUDES)
    list(APPEND ADAPTABLE_INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${hdr}> $<INSTALL_INTERFACE:${hdr}>)
endforeach()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INCLUDES} ${SOURCES})
target_sources(${RL} PRIVATE ${SOURCES} ${OBJECT_SOURCES} PUBLIC ${ADAPTABLE_INCLUDES})
target_link_libraries(${RL} PUBLIC ${RL_PUBLIC_DEPENDENCIES} PRIVATE ${RL_PRIVATE_DEPENDENCIES})

################################################
# VersionInfo
if(WIN32 AND (RL_LIBRARY_TYPE STREQUAL SHARED OR RL_LIBRARY_TYPE STREQUAL MODULE))
    include(VersionInfo)
    add_version_info(${RL}
                     NAME ${PROJECT_NAME}
                     FILENAME "${RL}.dll"
                     MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
                     MINOR_VERSION ${PROJECT_VERSION_MINOR}
                     PATCH_VERSION ${PROJECT_VERSION_PATCH}
                     COMPANY_NAME "Epic Games, Inc."
                     COPYRIGHT "Copyright Epic Games, Inc. All Rights Reserved.")
endif()

################################################
# Export and install target
include(InstallLibrary)
install_library(${RL})

enable_testing()

if(RL_BUILD_PYTHON_WRAPPER)
    set(PYDNA_DEPENDENCIES Spyus::spyus RigLogic::riglogic4)
    set(PYRL_DEPENDENCIES PyDNA::py3dna Spyus::spyus RigLogic::riglogic4)
    add_subdirectory(python)
endif()

get_directory_property(_has_parent PARENT_DIRECTORY)
if(NOT _has_parent)
    set(RUNTIME_DEPENDENCIES_FILE "${CMAKE_BINARY_DIR}/runtime_dependencies.csv" CACHE PATH "File path where to save comma separated list of runtime dependencies.")
    include(RuntimeDependencies)
    get_runtime_dependencies(${RL} _runtime_dependencies)
    file(WRITE ${RUNTIME_DEPENDENCIES_FILE} "${_runtime_dependencies}")
endif()

include(Uninstall)
create_uninstall_target()

################################################
# Package release artifacts
set(CPACK_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_COMPONENTS_ALL ${RL})
string(CONCAT CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}"
                                      "-${CMAKE_PROJECT_VERSION}"
                                      "-${CMAKE_SYSTEM_NAME}"
                                      "-${CMAKE_SYSTEM_PROCESSOR}"
                                      "-${CMAKE_CXX_COMPILER_ID}${CMAKE_CXX_COMPILER_VERSION}"
                                      "-${CMAKE_BUILD_TYPE}"
                                      "-${RL_LIBRARY_TYPE}")
if(RL_BUILD_PYTHON_WRAPPER)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-py${RL_BUILD_PYTHON_WRAPPER}")
endif()
if(ENABLE_SANITIZERS)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-sanitized")
endif()
include(CPack)

################################################
# Tests
option(RL_BUILD_TESTS "Build tests" ON)
if(RL_BUILD_TESTS)
    if (RL_LIBRARY_TYPE STREQUAL SHARED)
        message(WARNING "Tests depend on non-exported, private symbols of RigLogic and thus cannot be built when RigLogic is a shared library.")
        set(RL_BUILD_TESTS OFF)
    else()
        set(RL_TEST_LIB rltestlib)
        set(RL_TEST_RUNNER rltests)
        set(GOOGLETEST_VERSION release-1.12.1)
        add_subdirectory(tests)
        set(RL_TEST_TARGETS ${RL_TEST_LIB} ${RL_TEST_RUNNER} gtest)
        target_compile_options(${RL_TEST_LIB} PRIVATE "${CXX_FLAGS}")
        include(SupportedCompileOptions)
        target_supported_compile_options(${RL_TEST_LIB} PRIVATE "${CXX_EXTRA_FLAGS}")
    endif()
endif()

################################################
# Benchmarks
option(RL_BUILD_BENCHMARKS "Build benchmarks" ON)
if(RL_BUILD_BENCHMARKS)
    if (RL_LIBRARY_TYPE STREQUAL SHARED)
        message(WARNING "Benchmarks depend on non-exported, private symbols of RigLogic and thus cannot be built when RigLogic is a shared library.")
        set(RL_BUILD_BENCHMARKS OFF)
    else()
        set(RL_BENCHMARK_RUNNER rlbenchmarks)
        set(GOOGLEBENCHMARK_VERSION v1.8.3)
        add_subdirectory(benchmarks)
        set(RL_BENCHMARK_TARGETS ${RL_BENCHMARK_RUNNER} benchmark)
        target_compile_options(${RL_BENCHMARK_RUNNER} PRIVATE "${CXX_FLAGS}")
        include(SupportedCompileOptions)
        target_supported_compile_options(${RL_BENCHMARK_RUNNER} PRIVATE "${CXX_EXTRA_FLAGS}")
        include(InstallApplication)
        install_application(${RL_BENCHMARK_RUNNER} ${RL})
    endif()
endif()

################################################
# Examples
option(RL_BUILD_EXAMPLES "Build examples" ON)
if(RL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

################################################
# Static analysis and diagnostics

# Enable sanitizers (if command line parameter is present)
include(Sanitizers)
sanitize_targets(${RL} ${RL_TEST_TARGETS} ${RL_BENCHMARK_TARGETS} ${RL_EXAMPLES})

# Add targets for running clang-tidy, cppcheck, ...
include(Analysis)
analyze_targets(${RL} ${RL_TEST_LIB} ${RL_TEST_RUNNER} ${RL_BENCHMARK_RUNNER} ${RL_EXAMPLES})
