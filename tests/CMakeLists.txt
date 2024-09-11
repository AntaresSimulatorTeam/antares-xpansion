
if (MSVC)
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
else ()
    set(XPANSION_INSTALL_DIR "${CMAKE_BINARY_DIR}")
endif ()

#configure file to define xpansion installDir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/build_config.yaml)
#check if pytest and numpy are installed
find_python_module(pytest)
find_python_module(numpy)


# for centos docker to run MPI tests as root

set(allow_run_as_root_option "")
if (ALLOW_RUN_AS_ROOT)
    set(allow_run_as_root_option "--allow_run_as_root")
endif ()

if (PYTHON_MODULE_pytest_FOUND AND PYTHON_MODULE_numpy_FOUND)
        set(xpress_available "")
        if(${XPRESS})
	        set(xpress_available "--xpress")
        endif() 
    # Python unit test
    add_test(
            NAME unit_launcher
            COMMAND Python3::Interpreter -m pytest --cov=antares_xpansion --cov-report xml:coverage-reports/coverage-antares_xpansion.xml
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    )
    set_property(TEST unit_launcher PROPERTY LABELS unit)

    # Examples tests
    add_test(
            NAME examples_short_sequential
            COMMAND Python3::Interpreter -m pytest -m short_sequential ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_short_sequential PROPERTY LABELS short short_sequential end_to_end)

        add_test(
                NAME examples_short_memory
                COMMAND Python3::Interpreter -m pytest -m short_memory --allow_run_as_root=${ALLOW_RUN_AS_ROOT} --installDir=${XPANSION_INSTALL_DIR} example_test.py
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
        )
        set_property(TEST examples_short_memory PROPERTY LABELS short short_memory end_to_end)

        add_test(
            NAME examples_short_mpi
            COMMAND Python3::Interpreter -m pytest -m short_mpi ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_short_mpi PROPERTY LABELS short short_mpi end_to_end)

    add_test(
        NAME examples_short_benders_by_batch_mpi
            COMMAND Python3::Interpreter -m pytest -m short_benders_by_batch_mpi ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_short_benders_by_batch_mpi PROPERTY LABELS short short_mpi end_to_end)

    add_test(
            NAME examples_medium_sequential
            COMMAND Python3::Interpreter -m pytest -m medium_sequential ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_medium_sequential PROPERTY LABELS medium medium_sequential end_to_end)

    add_test(
            NAME examples_medium_mpi
            COMMAND Python3::Interpreter -m pytest -m medium_mpi ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_medium_mpi PROPERTY LABELS medium medium_mpi end_to_end)
   
    add_test(
            NAME example_medium_benders_by_batch_mpi
            COMMAND Python3::Interpreter -m pytest -m medium_benders_by_batch_mpi ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST example_medium_benders_by_batch_mpi PROPERTY LABELS medium medium_mpi end_to_end )

    add_test(
            NAME examples_long_sequential
            COMMAND Python3::Interpreter -m pytest -m long_sequential ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_long_sequential PROPERTY LABELS long long_sequential end_to_end)

    add_test(
            NAME examples_long_mpi
            COMMAND Python3::Interpreter -m pytest -m long_mpi ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_long_mpi PROPERTY LABELS long long_mpi end_to_end)
    add_test(
        NAME examples_long_benders_by_batch_mpi
            COMMAND Python3::Interpreter -m pytest -m long_benders_by_batch_mpi ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} example_test.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/examples
    )
    set_property(TEST examples_long_benders_by_batch_mpi PROPERTY LABELS long long_mpi end_to_end )
    # benders end to end tests
    add_test(
            NAME sequential
            COMMAND Python3::Interpreter -m pytest --installDir=${XPANSION_INSTALL_DIR} ${xpress_available} test_bendersSequentialEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST sequential PROPERTY LABELS benders benders-sequential end_to_end)
    add_test(
            NAME sequential_restart
            COMMAND Python3::Interpreter -m pytest ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} test_restartBendersSequentialEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/restart
    )
    set_property(TEST sequential_restart PROPERTY LABELS benders benders-sequential end_to_end restart)
    add_test(
            NAME mpibenders
            COMMAND Python3::Interpreter -m pytest ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} ${xpress_available} test_bendersmpibendersEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST mpibenders PROPERTY LABELS benders benders-mpi end_to_end)
    add_test(
            NAME mpibenders_restart
            COMMAND Python3::Interpreter -m pytest ${allow_run_as_root_option} --installDir=${XPANSION_INSTALL_DIR} test_restartBendersmpibendersEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/restart
    )
    set_property(TEST mpibenders_restart PROPERTY LABELS benders benders-mpi end_to_end restart)
    add_test(
            NAME merge_mps
            COMMAND Python3::Interpreter -m pytest --installDir=${XPANSION_INSTALL_DIR} test_bendersMergeMpsEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/benders
    )
    set_property(TEST merge_mps PROPERTY LABELS benders merge-mps end_to_end)

    add_test(
            NAME lpnamer_end_to_end
            COMMAND Python3::Interpreter -m pytest --installDir=${XPANSION_INSTALL_DIR} test_lpnamerEndToEnd.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end/lpnamer
    )
    set_property(TEST lpnamer_end_to_end PROPERTY LABELS lpnamer end_to_end)

else ()
    message(FATAL_ERROR "Module pytest or numpy not installed : can't run python scripts for end to end tests")
endif ()

add_subdirectory(cpp)
