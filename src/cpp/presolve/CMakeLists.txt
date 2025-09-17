# ===========================================================================
# CMake configuration for presolve module
# ==========================================================================='

add_library(presolvecore)
add_library(${PROJECT_NAME}::presolvecore ALIAS presolvecore)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
# TODO: Ajouter les fichiers sources réels, ex. :
target_sources(presolvecore PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/presolve.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/presolve/presolve.h
)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(presolvecore PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# On anticipe l'utilisation de helpers et des interfaces de logging.
# Ajuster selon les besoins réels une fois les sources implémentées.
find_package(fmt REQUIRED) # probables logs / formats

target_link_libraries(presolvecore
        PUBLIC
        ${PROJECT_NAME}::xpansion_interfaces
        ${PROJECT_NAME}::benders_core
        fmt::fmt
)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion)
    install(DIRECTORY include/antares-xpansion
            DESTINATION include)
endif ()
