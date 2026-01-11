add_library(plugins)
target_sources(plugins PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/BendersPlugin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/BendersPluginTST.h
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersPluginTST.cpp

)
target_link_libraries(plugins
            PUBLIC 
            antaresXpansion::benders_core
)

target_include_directories(plugins
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::plugins ALIAS plugins)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)