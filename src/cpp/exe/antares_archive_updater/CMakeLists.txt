add_library(archive_updater)
target_sources(archive_updater PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdaterExeOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/archive_updater/AntaresArchiveUpdater.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/archive_updater/AntaresArchiveUpdaterExeOptions.h
)
add_library(antaresXpansion::archive_updater ALIAS archive_updater)

target_include_directories(archive_updater
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(archive_updater
        PRIVATE
        antaresXpansion::archive_handler
        PUBLIC
        antaresXpansion::helpers
)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)

######################

add_executable(antares_archive_updater)
target_sources(antares_archive_updater PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(antares_archive_updater
        antaresXpansion::archive_updater
        antaresXpansion::archive_handler
)

install(TARGETS antares_archive_updater DESTINATION bin)