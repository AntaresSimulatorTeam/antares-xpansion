# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation Exe
# ---------------------------------------------------------------------------

add_executable(multiple_problem_generation
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(multiple_problem_generation
        PRIVATE
        ${PROJECT_NAME}::multiple_problem_generation_lib
)

install(TARGETS multiple_problem_generation DESTINATION bin)
