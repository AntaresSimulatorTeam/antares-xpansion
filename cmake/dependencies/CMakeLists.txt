#####################
##  Antares solver  ##
#####################
if(BUILD_antares_solver)

find_package(antares-solver QUIET)

if (NOT antares-solver_FOUND)

    set(REPOSITORY "https://github.com/AntaresSimulatorTeam/Antares_Simulator.git")
    set(TAG "v${ANTARES_VERSION_TAG}")
    set(CMAKE_ARGS "-DBUILD_UI=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DBUILD_not_system=OFF -DBUILD_ortools=ON ")

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

