#####################
##  ORTOOLS  ##
#####################
if(BUILD_ortools)

set(REPOSITORY "https://github.com/AntaresSimulatorTeam/or-tools.git")
set(TAG "rte_dev_sirius")
set(CMAKE_ARGS "-DUSE_SIRIUS=OFF")

if (USE_ORTOOLS_STABLE)
	set(REPOSITORY "https://github.com/google/or-tools.git")
	set(TAG "stable")
	set(CMAKE_ARGS "")
endif()

build_git_dependency(
    NAME
      ortools
    REPOSITORY
      ${REPOSITORY}
    TAG
      ${TAG}
    CMAKE_ARGS
      "${CMAKE_ARGS} -DUSE_COINOR=ON -DBUILD_PYTHON=OFF -DBUILD_TESTING=OFF -DBUILD_DEPS=ON -DBUILD_SAMPLES=OFF -DBUILD_EXAMPLES=OFF"
  )
  
endif()



