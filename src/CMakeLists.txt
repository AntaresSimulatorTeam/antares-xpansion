add_subdirectory (src_cpp)

find_program(PYINSTALLER "pyinstaller")

#if pyinstaller available add compiled version of python scripts
if (PYINSTALLER)

    install(CODE "execute_process(COMMAND ${PYINSTALLER} -F ${CMAKE_CURRENT_SOURCE_DIR}/src_python/launch.py -n antares-xpansion-launcher)")
    
    #code is executed in CMAKE_BINARY_DIR
    install(DIRECTORY ${CMAKE_BINARY_DIR}/dist/
            DESTINATION xpansion-launcher)
else ()
    
    message(STATUS "PyInstaller was not found : python scripts are use for install target")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src_python/" 		DESTINATION xpansion-launcher)
    
endif()

