find_package(Python3 ${DNA_BUILD_PYTHON_WRAPPER} EXACT COMPONENTS Interpreter Development)
find_package(SWIG REQUIRED)
include(UseSWIG)

set(PYDNA py3dna)
set_property(SOURCE DNA.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE DNA.i PROPERTY SWIG_MODULE_NAME dna)
set_property(SOURCE DNA.i PROPERTY SWIG_FLAGS "-doxygen")
# Plain identifier only: SWIG stringifies this into the runtime capsule name and PyCapsule_Import
# splits that name on '.', so quotes or dots make every type lookup fail (null-deref on bad args).
set(PYDNA_TYPE_TABLE "${PYDNA}_${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")
set_property(SOURCE DNA.i PROPERTY SWIG_TYPE_TABLE "${PYDNA_TYPE_TABLE}")

option(TYPEMAP_DEBUG "Debug deducing of typemaps" OFF)
if(TYPEMAP_DEBUG)
    set_property(SOURCE DNA.i PROPERTY SWIG_FLAGS "-debug-tmsearch")
endif()

swig_add_library(${PYDNA}
    TYPE
        SHARED
    LANGUAGE
        python
    SOURCES
        DNA.i)
add_library(PyDNA::${PYDNA} ALIAS ${PYDNA})

list(APPEND SWIG_COMPILE_DEFINITIONS SWIG_TYPE_TABLE=${PYDNA_TYPE_TABLE})
if(DNA_BUILD_WITH_JSON_SUPPORT)
    list(APPEND SWIG_COMPILE_DEFINITIONS DNA_BUILD_WITH_JSON_SUPPORT=ON)
endif()
set_target_properties(${PYDNA} PROPERTIES
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
    SWIG_COMPILE_DEFINITIONS "${SWIG_COMPILE_DEFINITIONS}"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    OUTPUT_NAME "${PYDNA}${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")
target_compile_definitions(${PYDNA} PRIVATE ${SWIG_COMPILE_DEFINITIONS})
target_include_directories(${PYDNA} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(${PYDNA}
    PUBLIC
        ${PYDNA_DEPENDENCIES}
    PRIVATE
        Python3::Python)

set(component_name dna)  # Make sure py3dna and dna targets are under the same dna component name
get_property(wrapper_files TARGET ${PYDNA} PROPERTY SWIG_SUPPORT_FILES)

set(insert_copyright "${CMAKE_CURRENT_SOURCE_DIR}/insert_copyright.py")
add_custom_command(
    TARGET ${PYDNA}
    POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${insert_copyright} ${wrapper_files}
    COMMENT "Inserting copyright notice to generated python file.")

install(FILES ${wrapper_files} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${component_name})
install(TARGETS ${PYDNA}
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
sanitize_targets(${PYDNA})

if(WIN32)
    add_test(NAME py3tests
             COMMAND ${CMAKE_COMMAND} -E env "PATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR};$ENV{PATH}" PYTHONPATH=. ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/tests/test_dna.py"
             WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
else()
    if (ENABLE_SANITIZERS)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=libasan.so
            OUTPUT_VARIABLE ASAN_LIB
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    add_test(NAME py3tests
            COMMAND ${CMAKE_COMMAND} -E env LD_PRELOAD=${ASAN_LIB} LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} PYTHONPATH=. ${Python3_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}/tests/test_dna.py"
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
    "${CMAKE_CURRENT_LIST_DIR}/examples/demo.py" out.dna)

set_tests_properties(
    py3example_demo
    PROPERTIES
        PASS_REGULAR_EXPRESSION "Done\\.")
