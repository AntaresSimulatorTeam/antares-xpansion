#####################
##  ORTOOLS  ##
#####################
if(BUILD_ortools)

set(REPOSITORY "https://github.com/google/or-tools.git")
set(TAG "stable")
set(CMAKE_ARGS "")

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



