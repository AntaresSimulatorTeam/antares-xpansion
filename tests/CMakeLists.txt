
if (MSVC)
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}")
endif()

#configure file to define xpansion installDir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml)

add_subdirectory(cpp)

#check if pytest and numpy are installed
find_python_module(pytest)
find_python_module(numpy)

if (PYTHON_MODULE_pytest_FOUND AND PYTHON_MODULE_numpy_FOUND)

	# Python unit test
	add_test(
        NAME unit_launcher
        COMMAND Python3::Interpreter -m pytest tests/python/test_ini_reader.py
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )
    set_property(TEST unit_launcher PROPERTY LABELS unit) 
	
    # Examples tests
    add_test(
        NAME examples_medium
        COMMAND Python3::Interpreter -m pytest -m medium --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_medium PROPERTY LABELS medium)
	
	add_test(
        NAME examples_long
        COMMAND Python3::Interpreter -m pytest -m long --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_long PROPERTY LABELS long)
	
	# benders end to end tests
    add_test(
        NAME benders_end_to_end
        COMMAND Python3::Interpreter -m pytest --installDir=$<TARGET_FILE_DIR:lp_namer> test_bendersEndToEnd.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST benders_end_to_end PROPERTY LABELS benders)
    
else()   
  message(FATAL "Module pytest or numpy not installed : can't run python scripts for end to end tests" )
endif()
