add_subdirectory (src_cpp)

#generate config.yaml with executable names in current source directory
SET(ANTARES_VERSION 7.2)
SET(ANTARES_SOLVER_APP "antares-${ANTARES_VERSION}-solver")

if (MSVC)
    SET(ANTARES_SOLVER_APP "${ANTARES_SOLVER_APP}.exe")
endif()

#configure file to define antares-solver executable
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml.in  ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml)

#use file generator to define xpansion executable name with $<TARGET_FILE_NAME:exe> : need to be done with generate instead of configure because values are not available at configure time
file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src_python/" 		DESTINATION xpansion-launcher)