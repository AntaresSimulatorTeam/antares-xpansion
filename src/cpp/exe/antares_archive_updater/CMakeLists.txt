# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# antares_archive_updater  Exe
# ---------------------------------------------------------------------------

add_executable (antares_archive_updater
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (antares_archive_updater
		antaresXpansion::helpers
)

install(TARGETS antares_archive_updater DESTINATION bin)