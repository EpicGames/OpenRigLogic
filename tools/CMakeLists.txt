set(SOURCES
    DnaInfo.cpp)

set(TOOL_TARGETS)

foreach(tool IN LISTS SOURCES)
    get_filename_component(filename ${tool} NAME_WE)
    string(TOLOWER ${filename} tool_target_name)
    add_executable(${tool_target_name} ${tool})
    target_link_libraries(${tool_target_name} PRIVATE ${RL})
    set_target_properties(${tool_target_name} PROPERTIES
                          CXX_STANDARD ${CMAKE_CXX_STANDARD}
                          CXX_STANDARD_REQUIRED NO
                          CXX_EXTENSIONS NO
                          FOLDER tools)
    list(APPEND TOOL_TARGETS ${tool_target_name})
endforeach()

set(RL_TOOL_TARGETS ${TOOL_TARGETS} PARENT_SCOPE)
