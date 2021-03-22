cmake_minimum_required(VERSION 3.10)

project(unit_tests_antares_xpansion NONE )

#check if pytest and numpy are installed
find_python_module(pytest)
find_python_module(numpy)

if (PYTHON_MODULE_pytest_FOUND AND PYTHON_MODULE_numpy_FOUND)

	# Python unit test
	add_test(
        NAME unit_launcher
        COMMAND Python3::Interpreter -m pytest test_ini_reader.py
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/../../src_python/antares_xpansion/test/"
    )
    set_property(TEST unit_launcher PROPERTY LABELS unit) 
	
    # Examples tests
    add_test(
        NAME examples_medium
        COMMAND Python3::Interpreter -m pytest -m medium --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples"
    )
    set_property(TEST examples_medium PROPERTY LABELS medium)
	
	add_test(
        NAME examples_long
        COMMAND Python3::Interpreter -m pytest -m long --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples"
    )
    set_property(TEST examples_long PROPERTY LABELS long)
    
else()   
  message(FATAL "Module pytest or numpy not installed : can't run python scripts for end to end tests" )
endif()
