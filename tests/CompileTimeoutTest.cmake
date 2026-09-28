# Compiler launcher for rlcompiletimeprobe: runs the compile command (everything after "--") and fails the build
# if it exceeds RL_COMPILE_TIMEOUT_TEST_SECONDS, killing the compiler process. The "--" is load-bearing: cmake
# script mode otherwise parses the compile command's own -D defines.

set(RL_TIMEOUT_TEST_ARGS "")
set(RL_TIMEOUT_TEST_COLLECT FALSE)
math(EXPR RL_TIMEOUT_TEST_LAST "${CMAKE_ARGC} - 1")
foreach(RL_TIMEOUT_TEST_I RANGE 0 ${RL_TIMEOUT_TEST_LAST})
    if(RL_TIMEOUT_TEST_COLLECT)
        string(APPEND RL_TIMEOUT_TEST_ARGS " \"\${CMAKE_ARGV${RL_TIMEOUT_TEST_I}}\"")
    elseif("${CMAKE_ARGV${RL_TIMEOUT_TEST_I}}" STREQUAL "--")
        set(RL_TIMEOUT_TEST_COLLECT TRUE)
    endif()
endforeach()

if(RL_TIMEOUT_TEST_ARGS STREQUAL "")
    message(FATAL_ERROR "CompileTimeoutTest.cmake: no compile command received")
endif()

cmake_language(EVAL CODE "
execute_process(COMMAND${RL_TIMEOUT_TEST_ARGS}
                TIMEOUT ${RL_COMPILE_TIMEOUT_TEST_SECONDS}
                RESULT_VARIABLE RL_TIMEOUT_TEST_RESULT)
")
if(NOT RL_TIMEOUT_TEST_RESULT EQUAL 0)
    message(FATAL_ERROR "rlcompiletimeprobe: BPCM kernel TU compile failed or exceeded "
                        "${RL_COMPILE_TIMEOUT_TEST_SECONDS}s (${RL_TIMEOUT_TEST_RESULT}) - compile-time blowup regression")
endif()
