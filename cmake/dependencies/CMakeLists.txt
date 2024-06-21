#####################
##  Antares solver  ##
#####################
if(BUILD_antares_solver)

find_package(Antares QUIET)

if (NOT Antares_FOUND)
    message(STATUS "Antares not found, building it from source")
    #Define deps install directory
    if (NOT DEPS_INSTALL_DIR)
        SET(DEPS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rte-antares-deps-${CMAKE_BUILD_TYPE})
    else()
        if(NOT IS_ABSOLUTE ${DEPS_INSTALL_DIR})
            SET(DEPS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${DEPS_INSTALL_DIR})
        endif()
    endif()

    # Build Needed dependencies
    list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR})

    # Add DEPS_INSTALL_DIR with antares-simulator CMAKE_BUILD_TYPE special case
    if (${CMAKE_BUILD_TYPE} STREQUAL  "Release")

        string(REPLACE ${CMAKE_BUILD_TYPE} release
                DEPS_INSTALL_DIR_ANTARES
                ${DEPS_INSTALL_DIR})

        list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR_ANTARES})

    elseif(${CMAKE_BUILD_TYPE} STREQUAL  "Debug")

        string(REPLACE ${CMAKE_BUILD_TYPE} debug
                DEPS_INSTALL_DIR_ANTARES
                ${DEPS_INSTALL_DIR})

        list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR_ANTARES})

    endif()
    set(REPOSITORY "https://github.com/AntaresSimulatorTeam/Antares_Simulator.git")
    set(TAG "v${ANTARES_VERSION_TAG}")
    set(CMAKE_ARGS "-DBUILD_UI=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DDEPS_INSTALL_DIR=${DEPS_INSTALL_DIR} -DBUILD_not_system=OFF -DBUILD_ortools=ON")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ANTARES_BUILD_TYPE "Debug")
    else ()
        set(ANTARES_BUILD_TYPE "Release")
    endif ()
    #Don't use CMAKE_BUILD_TYPE directly to support exotic build type in xpansion that may not exist in Simulator

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

