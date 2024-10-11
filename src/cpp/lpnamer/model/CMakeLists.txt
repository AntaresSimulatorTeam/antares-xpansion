# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(lp_namer_model)
target_sources(lp_namer_model PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ActiveLinks.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Candidate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ChronicleMapProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ChronicleMapReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkProfile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Problem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemNameParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/ActiveLinks.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/Candidate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/ChronicleMapProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/ChronicleMapReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/LinkProfile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/Problem.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/ProblemNameParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/model/SimulationInputMode.h
)

target_include_directories(lp_namer_model
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(lp_namer_model
        PUBLIC
        antaresXpansion::lp_namer_helper
        antaresXpansion::helpers
        solvers
)

add_library(${PROJECT_NAME}::lp_namer_model ALIAS lp_namer_model)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)