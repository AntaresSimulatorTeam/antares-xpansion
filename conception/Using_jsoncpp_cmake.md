# How to use jsoncpp in cmake [Feb. 2024]

The jsoncpp library doesn't follow modern Cmake convention.

Following the library documentation https://github.com/open-source-parsers/jsoncpp/wiki/Building we need to do:

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${JSON_INC_PATH})
target_link_libraries(${PROJECT_NAME} jsoncpp_lib)

We can at least use target_include_directories instead of include_directories but don't try to use find package or
namespaced targets.
Some issues are opened on jsoncpp repository but not worked on