# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Grid Search Exe
# ---------------------------------------------------------------------------

add_executable (variation_de_niveaux_de_stock_exe
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories (variation_de_niveaux_de_stock_exe PUBLIC
)


#IF (WIN32)
#	target_link_libraries (variation_de_niveaux_de_stock
#			${PROJECT_NAME}::variation_de_niveaux_de_stock_sequential_core ${PROJECT_NAME}::variation_de_niveaux_de_stock_mpi_core
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (variation_de_niveaux_de_stock_exe
			antaresXpansion::variation_de_niveaux_de_stock_lib
			antaresXpansion::factories
			antaresXpansion::output_core
			)
#ENDIF (WIN32)

install(TARGETS variation_de_niveaux_de_stock_exe DESTINATION bin)
