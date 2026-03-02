# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(lp_namer_input_reader)
target_sources(lp_namer_input_reader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CandidatesINIReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GeneralDataReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkProfileReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LpFilesExtractor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MpsTxtWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SettingsReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/VariableFileReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WeightsFileReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WeightsFileWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/AdditionalConstraintsReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/CandidatesINIReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/Exceptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/GeneralDataReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/INIReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/LinkProfileReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/LpFilesExtractor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/SettingsReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/VariableFileReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/WeightsFileReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h
)

target_include_directories(lp_namer_input_reader
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(lp_namer_input_reader
        PUBLIC
        antaresXpansion::lp_namer_model
        antaresXpansion::helpers
        antaresXpansion::solvers
)

add_library(${PROJECT_NAME}::lp_namer_input_reader ALIAS lp_namer_input_reader)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)