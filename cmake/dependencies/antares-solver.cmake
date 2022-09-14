#####################
##  Antares solver  ##
#####################
message("DEPENDENCY ANTARES " ${BUILD_antares_solver})
message("DEPENDENCY ANTARES " ${DOWNLOAD_antares_solver})
if(BUILD_antares_solver OR DOWNLOAD_antares_solver)
    find_package(antares-solver QUIET)
    if (NOT antares-solver_FOUND)
        if (DOWNLOAD_antares_solver)
            message(${DEPS_INSTALL_DIR}/..)
            if (MSVC)
            message(STATUS "Will download Antares solver windows assets")
                set(PLATFORM_TYPE installer-64bits.zip)
            else()
                execute_process(COMMAND bash "-c" "lsb_release -d | awk -F\"\t\" '{print $2}'" OUTPUT_VARIABLE SYSTEM_FLAVOUR)
                string(FIND ${SYSTEM_FLAVOUR} "Ubuntu" UBUNTU_FOUND)
                if (UBUNTU_FOUND EQUAL -1)
                    message(STATUS "Will download Antares solver CentOS assets")
                    set(PLATFORM_TYPE CentOS-7.9.2009-ortools-xpress.tar.gz)
                else()
                    message(STATUS "Will download Antares solver Ubuntu assets")
                    set(PLATFORM_TYPE ubuntu-20.04.tar.gz)
                endif()
            endif()
            execute_process(COMMAND rm -rf antares-${ANTARES_VERSION_TAG}-${PLATFORM_TYPE}) #Remove old version, just in case
            execute_process(COMMAND
                    wget https://github.com/AntaresSimulatorTeam/Antares_Simulator/releases/download/v${ANTARES_VERSION_TAG}/antares-${ANTARES_VERSION_TAG}-${PLATFORM_TYPE}
                    )
            execute_process(COMMAND tar -xvf antares-${ANTARES_VERSION_TAG}-${PLATFORM_TYPE} -C ${DEPS_INSTALL_DIR} --strip-components=1)
            execute_process(COMMAND rm -rf antares-${ANTARES_VERSION_TAG}-${PLATFORM_TYPE})
        endif()
    endif()
    if (NOT antares-solver_FOUND)
        if (BUILD_antares_solver)

            set(BUILD_sirius ON)
            set(BUILD_ortools ON)

            set(REPOSITORY "https://github.com/AntaresSimulatorTeam/Antares_Simulator.git")
            set(TAG "v${ANTARES_VERSION_TAG}")
            set(CMAKE_ARGS "-DBUILD_UI=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DDEPS_INSTALL_DIR=${DEPS_INSTALL_DIR} -DBUILD_not_system=OFF")

            if (CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(ANTARES_BUILD_TYPE "debug")
            else ()

                set(ANTARES_BUILD_TYPE "release")
            endif ()

            build_git_dependency(
                    NAME
                    antares-solver
                    REPOSITORY
                    ${REPOSITORY}
                    TAG
                    ${TAG}
                    CMAKE_ARGS
                    "${CMAKE_ARGS} -DCMAKE_BUILD_TYPE=${ANTARES_BUILD_TYPE}"
                    SOURCE_SUBDIR
                    "src"
            )
        endif()
    endif()
endif()
