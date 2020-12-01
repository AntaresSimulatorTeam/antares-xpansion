#####################
##  Antares solver  ##
#####################
if(BUILD_antares_solver)

set(REPOSITORY "https://github.com/AntaresSimulatorTeam/Antares_Simulator.git")
set(TAG "develop")
set(CMAKE_ARGS "-DBUILD_sirius=ON -DBUILD_UI=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DDEPS_INSTALL_DIR=${DEPS_INSTALL_DIR}")

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
