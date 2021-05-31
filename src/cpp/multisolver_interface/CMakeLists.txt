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
# CPLEX
IF( CPLEX )
	list(APPEND Solver_sources
		${CMAKE_CURRENT_LIST_DIR}/SolverCplex.cpp
	)
ENDIF( CPLEX )

# XPRESS
IF( XPRESS )
	list(APPEND Solver_sources
		${CMAKE_CURRENT_LIST_DIR}/SolverXpress.cpp
	)
ENDIF( XPRESS )

#Clp - CBc
IF(COIN_OR)
	list(APPEND Solver_sources
		${CMAKE_CURRENT_LIST_DIR}/SolverCbc.cpp
		${CMAKE_CURRENT_LIST_DIR}/SolverClp.cpp
	)
ENDIF(COIN_OR)


# ---------------------------------------------------------------------------
# Solver library creation and conditionnal link to internal solvers libraries
# ---------------------------------------------------------------------------
add_library (solvers STATIC
	${Solver_sources}
)

target_include_directories (solvers
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}
)

# CPLEX
if(CPLEX)
	target_include_directories (solvers
		PUBLIC
			${CPLEX_INCLUDE_FILES}
	)
	target_link_libraries (solvers
		PUBLIC
			debug		${CPLEX_DEBUG_LIB}
			optimized	${CPLEX_OPT_LIB}
	)
endif()

# XPRESS
if(XPRESS)
	target_include_directories (solvers
		PUBLIC
			${XPRESS_INCLUDE_FILES}
	)
	target_link_libraries (solvers
		PUBLIC
			debug		${XPRESS_DEBUG_LIB}
			optimized	${XPRESS_OPT_LIB}
	)
endif()

#CLP-CBC
if(COIN_OR)
	target_link_libraries (solvers
		PUBLIC
			Coin::Clp
			Coin::Cgl
			Coin::CoinUtils
			Coin::Osi
			Coin::Cbc
	)
endif()
