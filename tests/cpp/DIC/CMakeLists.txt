add_executable(DIC dependency_injection_container.cpp)

target_link_libraries(DIC
        PRIVATE
        antaresXpansion::DependencyInjectionContainer
        GTest::Main)

add_test(NAME dic COMMAND DIC)
set_property(TEST dic PROPERTY LABELS unit)
