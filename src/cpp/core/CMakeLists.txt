# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================

find_package(fmt REQUIRED)

# ===========================================================================
# Targets
# ===========================================================================

add_library(core)
add_library(antaresXpansion::core ALIAS core)
target_sources(core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemFormatStream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/core/ProblemFormat.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/core/ProblemFormatStream.h
)

target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC
        fmt::fmt
)

install(DIRECTORY include/antares-xpansion
        DESTINATION include
)
