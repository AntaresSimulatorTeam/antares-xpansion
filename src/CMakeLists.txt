add_subdirectory (cpp)

set(BENDERS $<TARGET_FILE_NAME:benders>)    

if (COIN_OR)
    SET(AVAILABLE_SOLVER_YML_LIST "${AVAILABLE_SOLVER_YML_LIST}- Cbc\n")
    SET(AVAILABLE_SOLVER_YML_LIST "${AVAILABLE_SOLVER_YML_LIST}- Coin\n")
endif()
    SET(AVAILABLE_SOLVER_YML_LIST "${AVAILABLE_SOLVER_YML_LIST}- Xpress\n")


#configure file to define antares-solver executable
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/python/config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/python/config.yaml)

#use file generator to define xpansion executable name with $<TARGET_FILE_NAME:exe> : need to be done with generate instead of configure because values are not available at configure time
file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/python/config.yaml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/python/config.yaml)

find_program(PYINSTALLER "pyinstaller")

#if pyinstaller available add compiled version of python scripts
if (PYINSTALLER)

    if (MSVC)
        install(CODE "execute_process(COMMAND ${PYINSTALLER} -F ${CMAKE_CURRENT_SOURCE_DIR}/python/launch.py -n antares-xpansion-launcher --add-data \"${CMAKE_CURRENT_SOURCE_DIR}/python/config.yaml;.\")" )
    else()
        install(CODE "execute_process(COMMAND ${PYINSTALLER} -F ${CMAKE_CURRENT_SOURCE_DIR}/python/launch.py -n antares-xpansion-launcher --add-data \"${CMAKE_CURRENT_SOURCE_DIR}/python/config.yaml:.\")" )
    endif()

    #code is executed in CMAKE_BINARY_DIR
    install(DIRECTORY ${CMAKE_BINARY_DIR}/dist/
            DESTINATION .
            USE_SOURCE_PERMISSIONS)
else ()
    
    message(STATUS "PyInstaller was not found : python scripts are use for install target")
    
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python/"
    	    DESTINATION xpansion-launcher
    	    USE_SOURCE_PERMISSIONS)

endif()
