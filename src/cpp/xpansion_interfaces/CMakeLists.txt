# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(xpansion_interfaces INTERFACE)
target_sources(xpansion_interfaces
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/xpansion_interfaces/ILogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/xpansion_interfaces/IWriterLogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/xpansion_interfaces/LogUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/xpansion_interfaces/OutputWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/xpansion_interfaces/StringManip.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/xpansion_interfaces/Clock.h
)
target_include_directories(xpansion_interfaces
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
add_library(${PROJECT_NAME}::xpansion_interfaces ALIAS xpansion_interfaces)
