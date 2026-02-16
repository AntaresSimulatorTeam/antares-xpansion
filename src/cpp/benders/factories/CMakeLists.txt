add_library(factories)
target_sources(factories PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/factories/BendersApp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/factories/BendersFactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/factories/LoggerFactories.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/factories/WriterFactories.h
        BendersApp.cpp
        BendersFactory.cpp
        LoggerFactories.cpp
        WriterFactories.cpp
)

target_link_libraries(factories
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::benders_sequential_core
        antaresXpansion::benders_by_batch_core
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        ${PROJECT_NAME}::benders_mpi_core
        antaresXpansion::outer_loop_lib
)

# Link optional strategy lib only if enabled
if(ENABLE_BENDERS_STRATEGY)
    target_link_libraries(factories PUBLIC antaresXpansion::benders_strategy)
endif()

target_include_directories(factories
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::factories ALIAS factories)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)