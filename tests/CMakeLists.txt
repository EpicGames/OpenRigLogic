include(GoogleTest)

if(DEFINED TESTS)
    set(TEST_RUNNER TestRunner.cpp)
    set(SOURCES)
    foreach(path IN LISTS TESTS)
        file(RELATIVE_PATH relpath "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
        if(NOT ${relpath} MATCHES ${TEST_RUNNER})
            list(APPEND SOURCES ${relpath})
        endif()
    endforeach()
else()
    set(TEST_RUNNER rltests/TestRunner.cpp)

    set(SOURCES
        rltests/Defs.h
        rltests/StorageValueType.h

        rltests/animatedmaps/AnimatedMapFixtures.cpp
        rltests/animatedmaps/AnimatedMapFixtures.h
        rltests/animatedmaps/TestAnimatedMaps.cpp
        rltests/animatedmaps/TestAnimatedMapsValidator.cpp

        rltests/blendshapes/BlendShapeFixtures.cpp
        rltests/blendshapes/BlendShapeFixtures.h
        rltests/blendshapes/TestBlendShapes.cpp
        rltests/blendshapes/TestBlendShapesValidator.cpp

        rltests/conditionaltable/ConditionalTableFixtures.cpp
        rltests/conditionaltable/ConditionalTableFixtures.h
        rltests/conditionaltable/TestConditionalTable.cpp

        rltests/controls/ControlFixtures.cpp
        rltests/controls/ControlFixtures.h
        rltests/controls/TestControls.cpp
        rltests/controls/TestControlsValidator.cpp

        rltests/dna/DNAFixtures.cpp
        rltests/dna/DNAFixtures.h
        rltests/dna/FakeReader.cpp
        rltests/dna/FakeReader.h
        rltests/dna/SyntheticFullReader.cpp
        rltests/dna/SyntheticFullReader.h

        rltests/joints/Helpers.h
        rltests/joints/TestJointsFactory.cpp
        rltests/joints/TestJointsValidator.cpp
        rltests/joints/TestStorageWalkValidation.cpp
        rltests/joints/TestStorageValidator.cpp
        rltests/joints/bpcm/Assertions.h
        rltests/joints/bpcm/BPCMFixturesBlock4.cpp
        rltests/joints/bpcm/BPCMFixturesBlock4.h
        rltests/joints/bpcm/BPCMFixturesBlock8.cpp
        rltests/joints/bpcm/BPCMFixturesBlock8.h
        rltests/joints/bpcm/Helpers.h
        rltests/joints/bpcm/TestBlock4CalculationStrategy.cpp
        rltests/joints/bpcm/TestBlock4StorageBuilder.cpp
        rltests/joints/bpcm/TestBlock8CalculationStrategy.cpp
        rltests/joints/bpcm/TestBlock8StorageBuilder.cpp
        rltests/joints/bpcm/TestCalculationTypeConsistency.cpp
        rltests/joints/quaternions/QuaternionFixtures.cpp
        rltests/joints/quaternions/QuaternionFixtures.h
        rltests/joints/quaternions/TestQuaternionCalculationTypeConsistency.cpp
        rltests/joints/quaternions/TestQuaternionEvaluator.cpp
        rltests/joints/quaternions/TestQuaternionStorageBuilder.cpp
        rltests/joints/twistswing/TwistSwingFixtures.cpp
        rltests/joints/twistswing/TwistSwingFixtures.h
        rltests/joints/twistswing/TestTwistSwingEvaluator.cpp
        rltests/joints/twistswing/TestTwistSwingStorageBuilder.cpp
        rltests/joints/twistswing/TestTwistSwingValidator.cpp
        rltests/joints/utils/TestLODRegion.cpp

        rltests/ml/cpu/FixturesBlock4.cpp
        rltests/ml/cpu/FixturesBlock4.h
        rltests/ml/cpu/FixturesMLBChained.cpp
        rltests/ml/cpu/FixturesMLBChained.h
        rltests/ml/cpu/TestInference.cpp
        rltests/ml/cpu/TestMLBChained.cpp
        rltests/ml/cpu/TestMLBWideGraphs.cpp
        rltests/ml/cpu/TestMLBehaviorValidator.cpp
        rltests/ml/cpu/TestStorageBuilder.cpp

        rltests/psdnet/TestPSDNet.cpp

        rltests/rbf/cpu/RBFFixtures.cpp
        rltests/rbf/cpu/RBFFixtures.h
        rltests/rbf/cpu/TestRBFEvaluator.cpp
        rltests/rbf/cpu/TestRBFSolver.cpp
        rltests/rbf/cpu/TestRBFStorageBuilder.cpp

        rltests/riglogic/TestRigLogic.cpp
        rltests/riglogic/TestRigMetadata.cpp
        rltests/riglogic/TestSyntheticDNACoverage.cpp)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_library(${RL_TEST_LIB} OBJECT ${SOURCES})
add_library(RigLogic::tests ALIAS ${RL_TEST_LIB})
get_target_property(RL_SOURCE_ROOT ${RL} SOURCE_ROOT)
target_include_directories(${RL_TEST_LIB} PRIVATE ${RL_SOURCE_ROOT})
target_include_directories(${RL_TEST_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${RL_TEST_LIB} PRIVATE ${RL} ${RL_PRIVATE_DEPENDENCIES} gtest)

add_executable(${RL_TEST_RUNNER} $<TARGET_OBJECTS:${RL_TEST_LIB}> ${TEST_RUNNER})
target_include_directories(${RL_TEST_RUNNER} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${RL_TEST_RUNNER} PRIVATE ${RL} gtest)
add_test(NAME unittests COMMAND ${CMAKE_COMMAND} -E env "PATH=$ENV{PATH}" $<TARGET_FILE:${RL_TEST_RUNNER}>)

set_target_properties(${RL_TEST_LIB} ${RL_TEST_RUNNER} PROPERTIES
                      CXX_STANDARD ${CMAKE_CXX_STANDARD}
                      CXX_STANDARD_REQUIRED NO
                      CXX_EXTENSIONS NO
                      FOLDER tests
                      SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Compile-time test for the BPCM multi-width kernel with the synthesized fallback widths force-inlined
# If all kernels (even fallback paths) are force-inlined, compilation time extends significantly (MSVC).
if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(RL_COMPILE_TIMEOUT_TEST_SECONDS 600 CACHE STRING
        "Seconds before the compile-timeout test declares the BPCM kernel TU compile hung")
    add_library(rlcompiletimeprobe OBJECT
        ${PROJECT_SOURCE_DIR}/src/riglogic/joints/cpu/bpcm/BPCMJointsBuilderPrecise.cpp)
    target_include_directories(rlcompiletimeprobe PRIVATE $<TARGET_PROPERTY:${RL},INCLUDE_DIRECTORIES>)
    target_compile_definitions(rlcompiletimeprobe PRIVATE $<TARGET_PROPERTY:${RL},COMPILE_DEFINITIONS>)
    target_compile_options(rlcompiletimeprobe PRIVATE $<TARGET_PROPERTY:${RL},COMPILE_OPTIONS> "/fp:precise")
    set_target_properties(rlcompiletimeprobe PROPERTIES
                          CXX_STANDARD ${CMAKE_CXX_STANDARD}
                          CXX_STANDARD_REQUIRED NO
                          CXX_EXTENSIONS NO
                          UNITY_BUILD OFF
                          FOLDER tests)
    if(CMAKE_GENERATOR MATCHES "Ninja|Makefiles" AND NOT CMAKE_VERSION VERSION_LESS 3.18)
        set_target_properties(rlcompiletimeprobe PROPERTIES CXX_COMPILER_LAUNCHER
                              "${CMAKE_COMMAND};-DRL_COMPILE_TIMEOUT_TEST_SECONDS=${RL_COMPILE_TIMEOUT_TEST_SECONDS};-P;${CMAKE_CURRENT_SOURCE_DIR}/CompileTimeoutTest.cmake;--")
    endif()
endif()
