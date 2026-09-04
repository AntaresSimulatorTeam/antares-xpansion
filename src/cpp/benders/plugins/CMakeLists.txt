add_library(plugins)
target_sources(plugins PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/Benders_MICRO_ITERS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/NoOperationPlugin.h
        Benders_MICRO_ITERS.cpp
        NoOperationPlugin.cpp

)

target_link_libraries(plugins
            PUBLIC 
            antaresXpansion::benders_core
            antaresXpansion::logger_lib
)

target_include_directories(plugins
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::plugins ALIAS plugins)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)