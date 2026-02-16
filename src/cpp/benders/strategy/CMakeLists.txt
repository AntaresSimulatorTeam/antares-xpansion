# CMake for benders_strategy

# Header-only / interface library: adapters are inline in headers to remain non-invasive
add_library(benders_strategy INTERFACE)

target_include_directories(benders_strategy
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Do not link heavy dependencies here; this is a header-only interface library
# Users can link implementation dependencies at top-level when ready.

add_library(antaresXpansion::benders_strategy ALIAS benders_strategy)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include")
