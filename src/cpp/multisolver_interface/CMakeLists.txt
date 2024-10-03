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
)

list(APPEND XPRESS_LOAD
	${CMAKE_CURRENT_LIST_DIR}/environment.cc
)

# XPRESS
	list(APPEND Solver_sources
		${CMAKE_CURRENT_LIST_DIR}/SolverXpress.cpp
	)

#Clp - CBc
IF(COIN_OR)
	list(APPEND Solver_sources
		${CMAKE_CURRENT_LIST_DIR}/SolverCbc.cpp
		${CMAKE_CURRENT_LIST_DIR}/SolverClp.cpp
		${CMAKE_CURRENT_LIST_DIR}/COIN_common_functions.cpp
	)
ENDIF(COIN_OR)


# ---------------------------------------------------------------------------
# Solver library creation and conditionnal link to internal solvers libraries
# ---------------------------------------------------------------------------
add_library (solvers STATIC
	${Solver_sources}
	${XPRESS_LOAD}
)
get_target_property(xpansion_interfaces_path xpansion_interfaces INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories (solvers
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}
		public
		${xpansion_interfaces_path}	
)

target_link_libraries(solvers
		PUBLIC
		${CMAKE_DL_LIBS}
)

#CLP-CBC
if(COIN_OR)
	target_link_libraries (solvers
		PRIVATE
			Coin::Clp
			Coin::Cgl
			Coin::CoinUtils
			Coin::Osi
			Coin::Cbc
	)
endif()
