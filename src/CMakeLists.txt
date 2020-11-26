add_subdirectory (src_cpp)

#generate config.yaml with executable names in current source directory
file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml.in)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src_python/" 		DESTINATION xpansion-launcher)