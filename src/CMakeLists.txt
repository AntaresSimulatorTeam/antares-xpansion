add_subdirectory (src_cpp)

#generate config.yaml with executable names in current source directory
SET(ANTARES_VERSION 8.0)

if (MSVC)
    SET(ANTARES_SOLVER_APP "antares-${ANTARES_VERSION}-solver.exe")
elseif(UNIX)
    SET(ANTARES_SOLVER_APP "antares-${ANTARES_VERSION}-solver")
endif()

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()


#configure file to define antares-solver executable
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml.in  ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml)

#use file generator to define xpansion executable name with $<TARGET_FILE_NAME:exe> : need to be done with generate instead of configure because values are not available at configure time
file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml INPUT ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src_python/config.yaml 
        DESTINATION xpansion-launcher)

find_program(PYINSTALLER "pyinstaller")

#if pyinstaller available add compiled version of python scripts
if (PYINSTALLER)

    install(CODE "execute_process(COMMAND ${PYINSTALLER} -F ${CMAKE_CURRENT_SOURCE_DIR}/src_python/launch.py -n antares-xpansion-launcher)")
    
    #code is executed in CMAKE_BINARY_DIR
    install(DIRECTORY ${CMAKE_BINARY_DIR}/dist/
            DESTINATION xpansion-launcher
            USE_SOURCE_PERMISSIONS)
else ()
    
    message(STATUS "PyInstaller was not found : python scripts are use for install target")
    
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src_python/"	
    	    DESTINATION xpansion-launcher
    	    USE_SOURCE_PERMISSIONS)
    
endif()
