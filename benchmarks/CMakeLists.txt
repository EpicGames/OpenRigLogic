include(GoogleBenchmark)

set(SOURCES rlbenchmarks/BenchmarkRunner.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(${RL_BENCHMARK_RUNNER} ${SOURCES})
target_link_libraries(${RL_BENCHMARK_RUNNER} PRIVATE ${RL} benchmark)
if(MSVC)
    target_compile_options(${RL_BENCHMARK_RUNNER} PRIVATE "/EHsc" "/wd4530")
endif()
get_target_property(RL_SOURCE_ROOT ${RL} SOURCE_ROOT)
target_include_directories(${RL_BENCHMARK_RUNNER} PRIVATE ${RL_SOURCE_ROOT})

set_target_properties(${RL_BENCHMARK_RUNNER} PROPERTIES
                      CXX_STANDARD ${CMAKE_CXX_STANDARD}
                      CXX_STANDARD_REQUIRED NO
                      CXX_EXTENSIONS NO
                      FOLDER benchmarks
                      SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
