add_library(DependencyInjectionContainer)
add_library(antaresXpansion::DependencyInjectionContainer ALIAS DependencyInjectionContainer)
target_sources(DependencyInjectionContainer
        PRIVATE
        DependencyInjectionContainer.cpp
        include/antares-xpansion/dic/DependencyInjectionContainer.h
)
target_include_directories(DependencyInjectionContainer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)