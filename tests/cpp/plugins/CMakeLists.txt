add_library(dummy_micro_iterations_plugin SHARED
    DummyMicroIterationsPlugin.cpp
)

set_target_properties(dummy_micro_iterations_plugin PROPERTIES
    OUTPUT_NAME "dummy_micro_iterations_plugin"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(TARGET dummy_micro_iterations_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:dummy_micro_iterations_plugin>
        ${CMAKE_SOURCE_DIR}/data_test/micro_it_mem_optim_study/plugin_inputs/
)
