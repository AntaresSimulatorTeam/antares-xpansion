add_library(dummy_micro_iterations_plugin SHARED
    DummyMicroIterationsPlugin.cpp
)

set_target_properties(dummy_micro_iterations_plugin PROPERTIES
    OUTPUT_NAME "dummy_micro_iterations_plugin"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
