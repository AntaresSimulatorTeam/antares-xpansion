
if (MSVC)
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else()
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}")
endif()

#configure file to define xpansion installDir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml )
file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml )
#check if pytest and numpy are installed
find_python_module(pytest)
find_python_module(numpy)

if (PYTHON_MODULE_pytest_FOUND AND PYTHON_MODULE_numpy_FOUND)

	# Python unit test
	add_test(
        NAME unit_launcher
        COMMAND Python3::Interpreter -m pytest --cov=antares_xpansion --cov-report xml:coverage-reports/coverage-antares_xpansion.xml
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    )
    set_property(TEST unit_launcher PROPERTY LABELS unit) 
	
    # Examples tests
    add_test(
        NAME examples_medium_sequential
        COMMAND Python3::Interpreter -m pytest -m medium_sequential --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_medium_sequential PROPERTY LABELS "medium;medium_sequential")

    add_test(
        NAME examples_medium_mpi
        COMMAND Python3::Interpreter -m pytest -m medium_mpi --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_medium_mpi PROPERTY LABELS "medium;medium_mpi")

    add_test(
        NAME examples_long
        COMMAND Python3::Interpreter -m pytest -m long --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_long PROPERTY LABELS long)
	
	# benders end to end tests
    add_test(
        NAME sequential
        COMMAND Python3::Interpreter -m pytest --installDir=$<TARGET_FILE_DIR:lp_namer> test_bendersSequentialEndToEnd.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST sequential PROPERTY LABELS "benders;benders-sequential")
    add_test(
        NAME mpibenders
        COMMAND Python3::Interpreter -m pytest --installDir=$<TARGET_FILE_DIR:lp_namer> test_bendersmpibendersEndToEnd.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST mpibenders PROPERTY LABELS "benders;benders-mpi")
    add_test(
        NAME merge_mps
        COMMAND Python3::Interpreter -m pytest --installDir=$<TARGET_FILE_DIR:lp_namer> test_bendersMergeMpsEndToEnd.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST merge_mps PROPERTY LABELS "benders;merge-mps")

    add_test(
            NAME lpnamer_end_to_end
            COMMAND Python3::Interpreter -m pytest  --installDir=$<TARGET_FILE_DIR:lp_namer> test_lpnamerEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/lpnamer
    )
    set_property(TEST lpnamer_end_to_end PROPERTY LABELS lpnamer)

else()   
  message(FATAL "Module pytest or numpy not installed : can't run python scripts for end to end tests" )
endif()

add_subdirectory(cpp)