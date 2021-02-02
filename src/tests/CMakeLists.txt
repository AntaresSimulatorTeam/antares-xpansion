cmake_minimum_required(VERSION 3.10)

project(unit_tests_antares_xpansion NONE )

find_package(Python3 EXACT 3.8 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)

  #check if pytest and numpy are installed
  find_python_module(pytest)
  find_python_module(numpy)

  if (PYTHON_MODULE_pytest_FOUND AND PYTHON_MODULE_numpy_FOUND)
        # Examples tests
        add_test(
            NAME examples
            COMMAND Python3::Interpreter -m pytest -m medium --installDir=$<TARGET_FILE_DIR:lp_namer> example_test.py
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples"
        )
        set_property(TEST examples PROPERTY LABELS medium)    
        
  else()   
      message(FATAL "Module pytest or numpy not installed : can't run python scripts for end to end tests" )
  endif()
  
endif()
