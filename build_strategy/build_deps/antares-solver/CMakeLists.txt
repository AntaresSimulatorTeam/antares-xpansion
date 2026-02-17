cmake_minimum_required(VERSION 3.5)

project(antares-solver NONE)

include(ExternalProject)

ExternalProject_Add(${PROJECT_NAME}_project
  TMP_DIR "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/${PROJECT_NAME}/tmp"
  STAMP_DIR "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/${PROJECT_NAME}/stamp"
  DOWNLOAD_DIR "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/${PROJECT_NAME}/download"
  SOURCE_DIR "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/${PROJECT_NAME}/source"  
  BINARY_DIR /home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/build

  SOURCE_SUBDIR src

  GIT_REPOSITORY "https://github.com/AntaresSimulatorTeam/Antares_Simulator.git"
  GIT_TAG "v9.3.6"
  LOG_DOWNLOAD FALSE
  GIT_SUBMODULES vcpkg

  # UPDATE_COMMAND ""
  LOG_UPDATE FALSE

  PATCH_COMMAND ""
  #LOG_PATCH FALSE

  #CONFIGURE_COMMAND
  CMAKE_ARGS
    -DCMAKE_PREFIX_PATH=/home/runner/work/antares-xpansion/antares-xpansion/../rte-antares-deps-Release|/home/runner/work/antares-xpansion/antares-xpansion/../rte-antares-deps-release
    -DCMAKE_INSTALL_PREFIX=/home/runner/work/antares-xpansion/antares-xpansion/../rte-antares-deps-Release
    -DBUILD_SHARED_LIBS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DBUILD_TESTING=OFF
    -DBUILD_UI=OFF -DCMAKE_TOOLCHAIN_FILE= -DDEPS_INSTALL_DIR=/home/runner/work/antares-xpansion/antares-xpansion/../rte-antares-deps-Release -DBUILD_not_system=OFF -DBUILD_ortools=ON -DCMAKE_BUILD_TYPE=Release
    LOG_CONFIGURE FALSE
	
  LIST_SEPARATOR |

  #BUILD_COMMAND
  LOG_BUILD FALSE

  #INSTALL_COMMAND
  LOG_INSTALL FALSE

  TEST_COMMAND ""
  LOG_TEST FALSE
)
