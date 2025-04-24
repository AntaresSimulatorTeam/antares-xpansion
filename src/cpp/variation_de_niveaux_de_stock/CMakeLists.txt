# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(variation_de_niveaux_de_stock_lib STATIC)

target_sources(variation_de_niveaux_de_stock_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/VariationDeNiveauxDeStock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/variation_de_niveaux_de_stock/VariationDeNiveauxDeStock.h
)

target_link_libraries(variation_de_niveaux_de_stock_lib
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::output_core
)

target_include_directories(variation_de_niveaux_de_stock_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/cpp/xpansion_interfaces/include>
)

add_library(${PROJECT_NAME}::variation_de_niveaux_de_stock_lib ALIAS variation_de_niveaux_de_stock_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
