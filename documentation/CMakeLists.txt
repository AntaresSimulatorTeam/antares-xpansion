cmake_minimum_required(VERSION 3.14)

option(BUILD_DOC "Build documentation" ON)

if(BUILD_DOC)
    find_package(Doxygen)

    if(NOT DOXYGEN_FOUND)
        message(WARNING "Doxygen is needed to build the documentation.")
    else()
        #sample input config file
        set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        #doxyfile to use for doc generation
        set(doxyfile_out ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

        configure_file(${doxyfile_in} ${doxyfile_out} @ONLY)

        add_custom_target(doc ALL
                            COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_outdir}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                            COMMENT "Generating API documentation with Doxygen"
                            VERBATIM)

        message(STATUS "Doxygen target created!")
    endif()
else()
    message(STATUS "doc targtet will not be created!")
endif()