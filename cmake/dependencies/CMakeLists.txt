#####################
##  OR-TOOLS       ##
#####################
if(BUILD_ortools)

if (USE_ORTOOLS_STABLE)
	set(REPOSITORY "https://github.com/google/or-tools.git")
	set(TAG "stable")
	set(CMAKE_ARGS "")
else()
    set(REPOSITORY "https://github.com/AntaresSimulatorTeam/or-tools.git")
    set(TAG "rte_dev_sirius")
    set(CMAKE_ARGS "-DUSE_SIRIUS=OFF")
endif()

build_git_dependency(
    NAME
      or-tools
    REPOSITORY
      ${REPOSITORY}
    TAG
      ${TAG}
    CMAKE_ARGS
      "${CMAKE_ARGS} -DUSE_COINOR=ON -DBUILD_PYTHON=OFF -DBUILD_TESTING=OFF -DBUILD_DEPS=ON -DBUILD_SAMPLES=OFF -DBUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
  )
  
endif()

#####################
##  Antares solver ##
#####################
if(BUILD_antares_solver)

set(REPOSITORY "https://github.com/AntaresSimulatorTeam/Antares_Simulator.git")
set(TAG "develop")
set(CMAKE_ARGS "-DBUILD_UI=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")

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
      "${CMAKE_ARGS} -DCMAKE_BUILD_TYPE=${ANTARES_BUILD_TYPE} -DBUILD_sirius=ON"
    SOURCE_SUBDIR
      "src"
  )
  
endif()
