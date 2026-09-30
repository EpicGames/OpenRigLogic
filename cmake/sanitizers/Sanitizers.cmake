# Enable sanitizers
#
# Usage:
#   include(Sanitizers)
#   sanitize_targets(target1 target2 ...)
#
# Then invoke CMake with the list of needed sanitizers, separated by
# semicolons or commas (the comma form survives shells and CI command
# lines that would eat a semicolon):
#   -DENABLE_SANITIZERS='address;leak;undefined'
#   -DENABLE_SANITIZERS=address,leak,undefined
#
# The list of supported sanitizers are:
# - address
# - memory
# - thread
# - leak
# - undefined
# - fuzzer
#
# among which "address", "memory" and "thread" are mutually exclusive and cannot
# be used at the same time. "leak" requires the "address" sanitizer.
#
# - MSVC cl supports address+fuzzer only
# - clang-cl supports address+fuzzer+undefined
# - GNU-style supports the full list
#
# "fuzzer" (Clang / MSVC only) applies libFuzzer COVERAGE INSTRUMENTATION to every
# sanitized target - safe on libraries and ordinary executables alike, and required
# on the whole dependency closure for coverage-guided fuzzing to see past the harness.
# The libFuzzer RUNTIME supplies its own main(), so it must be linked into the fuzz
# harness executable ONLY, via the additional per-target call:
#   fuzzer_targets(harness_target ...)
# which applies UNCONDITIONALLY (independent of ENABLE_SANITIZERS): a target passed
# to it IS a libFuzzer harness, and a front-end that cannot produce one is a fatal
# configuration error. The "fuzzer" list element stays the coverage-closure knob.
# Non-harness executables get only the sancov callback stubs at link; on the harness
# the full runtime satisfies those symbols first, so the stub library is never pulled.
# Typically combined with "address" so coverage-guided inputs also trap memory errors.
#
# Since the sanitizers carry a significant performance overhead, it is
# recommended to use the `-O1` optimization whenever using them.

function(_sanitizer_frontend out_var)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(${out_var} "msvc" PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
        set(${out_var} "clang-cl" PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(${out_var} "gnu" PARENT_SCOPE)
    else()
        set(${out_var} "unsupported" PARENT_SCOPE)
    endif()
endfunction()

# Full path of a clang runtime library (clang-cl only): CMake invokes the linker directly, so
# the driver never adds these itself. Handles both the per-target and legacy runtime layouts.
function(_clang_runtime_lib name out_var)
    if(NOT DEFINED _CLANG_RUNTIME_DIR)
        execute_process(
            COMMAND "${CMAKE_CXX_COMPILER}" /clang:--print-runtime-dir
            OUTPUT_VARIABLE runtime_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE status)
        if(NOT status EQUAL 0)
            message(FATAL_ERROR "Cannot query the clang runtime directory (--print-runtime-dir).")
        endif()
        file(TO_CMAKE_PATH "${runtime_dir}" runtime_dir)
        set(_CLANG_RUNTIME_DIR "${runtime_dir}" CACHE INTERNAL "clang runtime library directory")
    endif()
    file(GLOB candidates
        "${_CLANG_RUNTIME_DIR}/clang_rt.${name}.lib"
        "${_CLANG_RUNTIME_DIR}/../windows/clang_rt.${name}-*.lib")
    if(NOT candidates)
        message(FATAL_ERROR "clang runtime library clang_rt.${name} not found under ${_CLANG_RUNTIME_DIR}.")
    endif()
    list(GET candidates 0 lib)
    set(${out_var} "${lib}" PARENT_SCOPE)
endfunction()

function(_require_static_crt sanitizer)
    if(NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "^MultiThreaded$")
        message(FATAL_ERROR
            "The ${sanitizer} sanitizer under clang-cl needs the static CRT: configure with \
            -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded.")
    endif()
endfunction()

function(check_compiler_version gcc_min_version clang_min_version msvc_min_version)
    set(compiler_ver "${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        if(NOT gcc_min_version)
            message(FATAL_ERROR
                "${compiler_ver} detected, which doesn't support the \
                ${sanitizer} sanitizer.")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${gcc_min_version})
            message(FATAL_ERROR
                "${compiler_ver} detected, but the ${sanitizer} sanitizer is \
                supported since v${gcc_min_version} only.")
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(NOT clang_min_version)
            message(FATAL_ERROR
                "${compiler_ver} detected, which doesn't support the \
                ${sanitizer} sanitizer.")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${clang_min_version})
            message(FATAL_ERROR
                "${compiler_ver} detected, but the ${sanitizer} sanitizer is \
                supported since v${clang_min_version} only.")
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        if(NOT msvc_min_version)
            message(FATAL_ERROR
                "${compiler_ver} detected, which doesn't support the \
                ${sanitizer} sanitizer.")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${msvc_min_version})
            message(FATAL_ERROR
                "${compiler_ver} detected, but the ${sanitizer} sanitizer is \
                supported since v${msvc_min_version} only.")
        endif()
    endif()
endfunction()

# Pure flag mapping for known-valid (sanitizer, front-end) combinations - callers filter through
# _supported_sanitizer_list, the single authority on which combinations exist.
function(enable_sanitizer_flags sanitize_option compile_flags_var link_flags_var)
    _sanitizer_frontend(frontend)
    if(${sanitize_option} STREQUAL "address")
        check_compiler_version("4.8" "3.1" "19.29")
        if(frontend STREQUAL "msvc")
            # NOICF: COMDAT folding can alias a global into another's ASan redzone.
            set(${compile_flags_var} "/fsanitize=address /Oy- /wd5072" PARENT_SCOPE)
            set(${link_flags_var} "/ignore:4300,4302 /OPT:NOICF" PARENT_SCOPE)
        elseif(frontend STREQUAL "clang-cl")
            _clang_runtime_lib("asan_dynamic" asan_lib)
            if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
                _clang_runtime_lib("asan_dynamic_runtime_thunk" thunk_lib)
            else()
                _clang_runtime_lib("asan_static_runtime_thunk" thunk_lib)
            endif()
            get_filename_component(asan_dll_dir "${asan_lib}" DIRECTORY)
            get_property(notified GLOBAL PROPERTY _SANITIZER_ASAN_PATH_NOTIFIED)
            if(NOT notified)
                message(STATUS "ASan (clang-cl): add ${asan_dll_dir} to PATH when running sanitized binaries.")
                set_property(GLOBAL PROPERTY _SANITIZER_ASAN_PATH_NOTIFIED TRUE)
            endif()
            # Annotations off so the prebuilt clang_rt libs' failifmismatch directives agree.
            set(${compile_flags_var}
                "/fsanitize=address /Oy- /wd5072 -D_DISABLE_STRING_ANNOTATION -D_DISABLE_VECTOR_ANNOTATION"
                PARENT_SCOPE)
            set(${link_flags_var} "/ignore:4300,4302 /OPT:NOICF \"${asan_lib}\" \"${thunk_lib}\"" PARENT_SCOPE)
        else()
            set(${compile_flags_var}
                "-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls"
                PARENT_SCOPE)
            set(${link_flags_var} "-fsanitize=address" PARENT_SCOPE)
        endif()
    elseif(${sanitize_option} STREQUAL "thread")
        check_compiler_version("4.8" "3.1" "")
        set(${compile_flags_var} "-fsanitize=thread" PARENT_SCOPE)
        set(${link_flags_var} "-fsanitize=thread" PARENT_SCOPE)
    elseif(${sanitize_option} STREQUAL "memory")
        check_compiler_version("" "3.1" "")
        set(${compile_flags_var} "-fsanitize=memory" PARENT_SCOPE)
        set(${link_flags_var} "-fsanitize=memory" PARENT_SCOPE)
    elseif(${sanitize_option} STREQUAL "leak")
        check_compiler_version("4.9" "3.4" "")
        set(${compile_flags_var} "-fsanitize=leak" PARENT_SCOPE)
        set(${link_flags_var} "-fsanitize=leak" PARENT_SCOPE)
    elseif(${sanitize_option} STREQUAL "undefined")
        check_compiler_version("4.9" "3.1" "")
        if(frontend STREQUAL "clang-cl")
            # GNU-only spellings ride /clang:; the /MT-only runtime is named for the linker.
            _require_static_crt("undefined")
            _clang_runtime_lib("ubsan_standalone" ubsan_lib)
            _clang_runtime_lib("ubsan_standalone_cxx" ubsan_cxx_lib)
            set(${compile_flags_var}
                "-fsanitize=undefined /Oy- /clang:-fno-optimize-sibling-calls"
                PARENT_SCOPE)
            set(${link_flags_var} "\"${ubsan_lib}\" \"${ubsan_cxx_lib}\"" PARENT_SCOPE)
        else()
            set(${compile_flags_var}
                "-fsanitize=undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls"
                PARENT_SCOPE)
            set(${link_flags_var} "-fsanitize=undefined" PARENT_SCOPE)
        endif()
    elseif(${sanitize_option} STREQUAL "fuzzer")
        check_compiler_version("" "6.0" "19.29")
        if(frontend STREQUAL "msvc")
            # The same coverage set MSVC's /fsanitize=fuzzer driver flag implies, without the runtime.
            set(${compile_flags_var}
                "/fsanitize-coverage=inline-8bit-counters /fsanitize-coverage=edge /fsanitize-coverage=trace-cmp /fsanitize-coverage=trace-div"
                PARENT_SCOPE)
            if(CMAKE_BUILD_TYPE MATCHES "Debug")
                set(${link_flags_var} "libsancovd.lib" PARENT_SCOPE)
            else()
                set(${link_flags_var} "libsancov.lib" PARENT_SCOPE)
            endif()
        elseif(frontend STREQUAL "clang-cl")
            _clang_runtime_lib("fuzzer_no_main" stub_lib)
            set(${compile_flags_var} "-fsanitize=fuzzer-no-link" PARENT_SCOPE)
            set(${link_flags_var} "\"${stub_lib}\"" PARENT_SCOPE)
        else()
            set(${compile_flags_var} "-fsanitize=fuzzer-no-link" PARENT_SCOPE)
            set(${link_flags_var} "-fsanitize=fuzzer-no-link" PARENT_SCOPE)
        endif()
    else()
        message(FATAL_ERROR "Compiler sanitizer option `${sanitize_option}` not supported.")
    endif()
endfunction()

# Normalized ENABLE_SANITIZERS: commas become list separators, entries lowercased, empties dropped.
function(_sanitizer_list out_var)
    string(REPLACE "," ";" sanitizers "${ENABLE_SANITIZERS}")
    set(normalized "")
    foreach(sanitizer IN LISTS sanitizers)
        string(STRIP "${sanitizer}" sanitizer)
        if(sanitizer)
            string(TOLOWER "${sanitizer}" sanitizer)
            list(APPEND normalized "${sanitizer}")
        endif()
    endforeach()
    set(${out_var} "${normalized}" PARENT_SCOPE)
endfunction()

function(_supported_sanitizer_list frontend out_var)
    _sanitizer_list(sanitizers)
    if(frontend STREQUAL "msvc")
        set(available "address;fuzzer")
    elseif(frontend STREQUAL "clang-cl")
        set(available "address;undefined;fuzzer")
    else()
        set(available "address;memory;thread;leak;undefined;fuzzer")
    endif()
    set(supported "")
    foreach(sanitizer IN LISTS sanitizers)
        if(sanitizer IN_LIST available)
            list(APPEND supported "${sanitizer}")
        else()
            # Once per configure: every dependency's sanitize_targets() call runs this filter.
            get_property(notified GLOBAL PROPERTY _SANITIZER_SKIP_NOTIFIED_${sanitizer})
            if(NOT notified)
                message(STATUS "Sanitizer '${sanitizer}' is not supported by ${frontend} - skipped.")
                set_property(GLOBAL PROPERTY _SANITIZER_SKIP_NOTIFIED_${sanitizer} TRUE)
            endif()
        endif()
    endforeach()
    set(${out_var} "${supported}" PARENT_SCOPE)
endfunction()

function(_sanitize_target target_name sanitizers)
    foreach(sanitizer IN LISTS sanitizers)
        enable_sanitizer_flags(${sanitizer} san_compile_flags san_link_flags)
        set_property(TARGET ${target_name} APPEND_STRING
            PROPERTY COMPILE_FLAGS " ${san_compile_flags}")
        set_property(TARGET ${target_name} APPEND_STRING
            PROPERTY LINK_FLAGS " ${san_link_flags}")
    endforeach()
endfunction()

# TRUE when <sanitizer> is requested in ENABLE_SANITIZERS and the front-end supports it - i.e.
# sanitize_targets will actually apply it rather than skip it. For gating features (like a fuzz
# harness) that are useless without a given sanitizer in effect.
function(sanitizer_effective sanitizer out_var)
    set(${out_var} FALSE PARENT_SCOPE)
    _sanitizer_frontend(frontend)
    if(frontend STREQUAL "unsupported")
        return()
    endif()
    _supported_sanitizer_list(${frontend} supported)
    string(TOLOWER "${sanitizer}" sanitizer)
    if(sanitizer IN_LIST supported)
        set(${out_var} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(sanitize_targets)
    if(NOT ENABLE_SANITIZERS)
        return()
    endif()

    _sanitizer_frontend(frontend)
    if(frontend STREQUAL "unsupported")
        message(STATUS "Compiler (${CMAKE_CXX_COMPILER_ID}) does not have sanitizer support.")
        return()
    endif()

    _supported_sanitizer_list(${frontend} sanitizers)
    foreach(target_name IN LISTS ARGN)
        _sanitize_target(${target_name} "${sanitizers}")
    endforeach()
endfunction()

function(fuzzer_targets)
    _sanitizer_frontend(frontend)
    if(frontend STREQUAL "unsupported" OR (frontend STREQUAL "gnu" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        message(FATAL_ERROR "fuzzer_targets: ${CMAKE_CXX_COMPILER_ID} cannot build a libFuzzer harness (Clang or MSVC required).")
    endif()

    foreach(target_name IN LISTS ARGN)
        if(frontend STREQUAL "msvc")
            set_property(TARGET ${target_name} APPEND_STRING
                PROPERTY COMPILE_FLAGS " /fsanitize=fuzzer")
        elseif(frontend STREQUAL "clang-cl")
            # The full libFuzzer runtime (with main) on the harness only.
            _clang_runtime_lib("fuzzer" fuzzer_lib)
            set_property(TARGET ${target_name} APPEND_STRING
                PROPERTY COMPILE_FLAGS " -fsanitize=fuzzer")
            set_property(TARGET ${target_name} APPEND_STRING
                PROPERTY LINK_FLAGS " \"${fuzzer_lib}\"")
        else()
            set_property(TARGET ${target_name} APPEND_STRING
                PROPERTY COMPILE_FLAGS " -fsanitize=fuzzer")
            set_property(TARGET ${target_name} APPEND_STRING
                PROPERTY LINK_FLAGS " -fsanitize=fuzzer")
        endif()
    endforeach()
endfunction()
