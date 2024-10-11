# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Conditionnal settings of solver source files
# ---------------------------------------------------------------------------
list(APPEND Solver_sources
        ${CMAKE_CURRENT_LIST_DIR}/SolverFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/multisolver_interface/Solver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/multisolver_interface/SolverAbstract.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/multisolver_interface/SolverFactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/multisolver_interface/dynamic_library.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/multisolver_interface/environment.h

)

list(APPEND XPRESS_LOAD
        ${CMAKE_CURRENT_LIST_DIR}/environment.cc
)

# XPRESS
list(APPEND Solver_sources
        ${CMAKE_CURRENT_LIST_DIR}/SolverXpress.cpp
        ${CMAKE_CURRENT_LIST_DIR}/private/SolverXpress.h
)

#Clp - CBc
IF (COIN_OR)
    list(APPEND Solver_sources
            ${CMAKE_CURRENT_LIST_DIR}/SolverCbc.cpp
            ${CMAKE_CURRENT_LIST_DIR}/SolverClp.cpp
            ${CMAKE_CURRENT_LIST_DIR}/COIN_common_functions.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/private/COIN_common_functions.h
            ${CMAKE_CURRENT_SOURCE_DIR}/private/SolverCbc.h
            ${CMAKE_CURRENT_SOURCE_DIR}/private/SolverClp.h
    )
ENDIF (COIN_OR)


# ---------------------------------------------------------------------------
# Solver library creation and conditionnal link to internal solvers libraries
# ---------------------------------------------------------------------------
add_library(solvers)
target_sources(solvers PRIVATE
        ${Solver_sources}
        ${XPRESS_LOAD}
)

add_library(antaresXpansion::solvers ALIAS solvers)

get_target_property(xpansion_interfaces_path xpansion_interfaces INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(solvers
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${xpansion_interfaces_path}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
)

target_link_libraries(solvers
        PUBLIC
        ${CMAKE_DL_LIBS}
)

#CLP-CBC
if (COIN_OR)
    target_link_libraries(solvers
            PRIVATE
            Coin::Clp
            Coin::Cgl
            Coin::CoinUtils
            Coin::Osi
            Coin::Cbc
    )
endif ()

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)