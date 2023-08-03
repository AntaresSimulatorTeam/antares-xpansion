
if (MSVC)
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}")
endif()

#configure file to define xpansion installDir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml )
#check if pytest and numpy are installed
find_python_module(pytest)
find_python_module(numpy)

if (PYTHON_MODULE_pytest_FOUND AND PYTHON_MODULE_numpy_FOUND)

	# Python unit test
	add_test(
        NAME unit_launcher
            COMMAND Python3::Interpreter -m pytest -s --cov=antares_xpansion --cov-report xml:coverage-reports/coverage-antares_xpansion.xml
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    )
    set_property(TEST unit_launcher PROPERTY LABELS unit) 

else()   
  message(FATAL_ERROR "Module pytest or numpy not installed : can't run python scripts for end to end tests" )
endif()

add_subdirectory(cpp)
