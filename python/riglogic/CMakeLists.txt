find_package(Python3 ${RL_BUILD_PYTHON_WRAPPER} EXACT COMPONENTS Interpreter Development)
find_package(SWIG REQUIRED)
include(UseSWIG)

set(PYRL py3riglogic)
set_property(SOURCE RigLogic.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE RigLogic.i PROPERTY SWIG_MODULE_NAME riglogic)
set_property(SOURCE RigLogic.i PROPERTY SWIG_FLAGS "-doxygen")
# Plain identifier only: SWIG stringifies this into the runtime capsule name and PyCapsule_Import
# splits that name on '.', so quotes or dots make every type lookup fail (null-deref on bad args).
set(PYRL_TYPE_TABLE "${PYRL}_${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")
set_property(SOURCE RigLogic.i PROPERTY SWIG_TYPE_TABLE "${PYRL_TYPE_TABLE}")

option(TYPEMAP_DEBUG "Debug deducing of typemaps" OFF)
if(TYPEMAP_DEBUG)
    set_property(SOURCE RigLogic.i PROPERTY SWIG_FLAGS "-debug-tmsearch")
endif()

swig_add_library(${PYRL}
    TYPE
        SHARED
    LANGUAGE
        python
    SOURCES
        RigLogic.i)
add_library(PyRigLogic::${PYRL} ALIAS ${PYRL})

function(get_swig_type_table_name target_name output_var)
    get_property(compile_defs TARGET ${target_name} PROPERTY COMPILE_DEFINITIONS)
    list(FILTER compile_defs INCLUDE REGEX "SWIG_TYPE_TABLE=.+")
    string(REPLACE "=" ";" type_table_name "${compile_defs}")
    list(GET type_table_name 1 type_table_name)
    set(${output_var} ${type_table_name} PARENT_SCOPE)
endfunction()

get_swig_type_table_name(PyDNA::py3dna pydna_type_table)
list(APPEND SWIG_COMPILE_DEFINITIONS SWIG_TYPE_TABLE=${PYRL_TYPE_TABLE} PYDNA_SWIG_TYPE_TABLE=${pydna_type_table})
set_target_properties(${PYRL} PROPERTIES
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
    SWIG_COMPILE_DEFINITIONS "${SWIG_COMPILE_DEFINITIONS}"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    OUTPUT_NAME "${PYRL}${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")
target_compile_definitions(${PYRL} PRIVATE ${SWIG_COMPILE_DEFINITIONS})
target_include_directories(${PYRL} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(${PYRL}
    PUBLIC
        ${PYRL_DEPENDENCIES}
    PRIVATE
        Python3::Python)

set(component_name riglogic4)  # Make sure py3riglogic and riglogic4 targets are under the same riglogic4 component name
get_property(wrapper_files TARGET ${PYRL} PROPERTY SWIG_SUPPORT_FILES)
set(insert_copyright "${CMAKE_CURRENT_SOURCE_DIR}/insert_copyright.py")
add_custom_command(
    TARGET ${PYRL}
    POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${insert_copyright} ${wrapper_files}
    COMMENT "Inserting copyright notice to generated python file.")

install(FILES ${wrapper_files} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${component_name})
install(TARGETS ${PYRL}
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT ${component_name}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT ${component_name}
        NAMELINK_COMPONENT ${component_name}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT ${component_name})

include(Sanitizers)
sanitize_targets(${PYRL})

if(WIN32)
    add_test(NAME py3tests
             COMMAND ${CMAKE_COMMAND} -E env "PATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR};$ENV{PATH}" PYTHONPATH=. ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/tests/test_riglogic.py"
             WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
else()
    if (ENABLE_SANITIZERS)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=libasan.so
            OUTPUT_VARIABLE ASAN_LIB
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    add_test(NAME py3tests
            COMMAND ${CMAKE_COMMAND} -E env LD_PRELOAD=${ASAN_LIB} LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} PYTHONPATH=. ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/tests/test_riglogic.py"
            WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
endif()

macro(add_py3example_test test_name)
    if(WIN32)
        add_test(NAME ${test_name}
                 COMMAND ${CMAKE_COMMAND} -E env "PATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR};$ENV{PATH}" PYTHONPATH=. ${Python3_EXECUTABLE} ${ARGN}
                 WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
    else()
        add_test(NAME ${test_name}
                 COMMAND ${CMAKE_COMMAND} -E env LD_PRELOAD=${ASAN_LIB} LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} PYTHONPATH=. ${Python3_EXECUTABLE} ${ARGN}
                 WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
    endif()
endmacro()

add_py3example_test(py3example_demo
    "${CMAKE_CURRENT_LIST_DIR}/examples/demo.py" "${CMAKE_CURRENT_LIST_DIR}/../../examples/Sample.dna")

set_tests_properties(
    py3example_demo
    PROPERTIES
        PASS_REGULAR_EXPRESSION "Done\\.")
