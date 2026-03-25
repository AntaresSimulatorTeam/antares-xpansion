add_library(plugins)
target_sources(plugins PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/BendersPlugin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/NoOperationPlugin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/IMicroIterationOracle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/MicroIterationOracleCABI.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/JuliaMicroIterOracle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/plugins/BendersMicroIterPlugin.h
        Benders_Jl_MICRO_ITERS.cpp
        NoOperationPlugin.cpp
        JuliaMicroIterOracle.cpp
        BendersMicroIterPlugin.cpp

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