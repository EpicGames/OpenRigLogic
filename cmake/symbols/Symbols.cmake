# Generate header file that contains preprocessor definitions for exporting symbols from shared libraries.
#
# One macro covers the common case and two complementary pairs cover the constructs that need an attribute in
# more than one place:
#   EXPORT_ATTR_NAME                   - free functions, and members of a class with no type attribute
#   EXPORT_ATTR_NAME + _TYPE           - the definition of a polymorphic class whose vtable and typeinfo have to
#   EXPORT_ATTR_NAME + _MEMBER           cross a shared-object boundary, and its members
#   EXPORT_ATTR_NAME + _TEMPLATE_DECL  - the `extern template` declaration of an explicit class-template
#   EXPORT_ATTR_NAME + _TEMPLATE_DEFN    instantiation, and its matching definition
# Within a pair, whichever half would conflict on the target ABI expands to nothing, so both are always spelled:
# `class X_TYPE C { X_MEMBER void m(); };` compiles everywhere, and so does an instantiation carrying
# _TEMPLATE_DECL in the header and _TEMPLATE_DEFN in the one translation unit that defines it. An explicitly
# instantiated template uses the second pair instead of annotating its members.
#
# Usage:
#   include(Symbols)
#   generate_export_definitions(
#       OUTPUT_FILE /abs/path/to/include/mylib/Defs.h
#       EXPORT_ATTR_NAME MLAPI
#       BUILD_SHARED_NAME ML_BUILD_SHARED
#       USE_SHARED_NAME ML_SHARED)
#
# Defines MLAPI, MLAPI_TYPE, MLAPI_MEMBER, MLAPI_TEMPLATE_DECL and MLAPI_TEMPLATE_DEFN.
#
# Module dependencies:
#   CMakeParseArguments

set(SYMBOLS_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(generate_export_definitions)
    set(options)
    set(one_value_args OUTPUT_FILE EXPORT_ATTR_NAME BUILD_SHARED_NAME USE_SHARED_NAME)
    set(multi_value_args)
    cmake_parse_arguments(DEF "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
    configure_file("${SYMBOLS_SOURCE_DIR}/Defs.h.in"
                   "${DEF_OUTPUT_FILE}"
                   @ONLY
                   NEWLINE_STYLE LF)
endfunction()
