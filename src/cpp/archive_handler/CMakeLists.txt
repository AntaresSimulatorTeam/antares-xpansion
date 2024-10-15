add_library(archive_handler)
target_sources(archive_handler PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ArchiveReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ArchiveWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileInBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/archive_handler/ArchiveIO.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/archive_handler/ArchiveReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/archive_handler/ArchiveWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/archive_handler/FileInBuffer.h
)
add_library(antaresXpansion::archive_handler ALIAS archive_handler)

target_include_directories(archive_handler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(archive_handler
        PUBLIC
        MINIZIP::minizip-ng
        PRIVATE
        ${PROJECT_NAME}::xpansion_interfaces
)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)