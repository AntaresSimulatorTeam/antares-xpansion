# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================
find_package(yaml-cpp CONFIG REQUIRED)

find_package (MPI REQUIRED COMPONENTS C)
message ("MPI_C_INCLUDE_PATH ${MPI_C_INCLUDE_PATH}")
message ("MPI_C_INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS}")
message ("MPI_C_LIBRARIES ${MPI_C_LIBRARIES}")

find_package(MPI REQUIRED)

if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()

add_library (outer_loop_lib STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/OuterLoopBiLevel.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/VariablesGroup.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/OuterLoopInputDataReader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/OuterLoop.cpp
	)




target_include_directories (outer_loop_lib
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
		)

target_link_libraries (outer_loop_lib
	PUBLIC
		helpers
		yaml-cpp::yaml-cpp
)

add_library (${PROJECT_NAME}::outer_loop_lib ALIAS outer_loop_lib)
