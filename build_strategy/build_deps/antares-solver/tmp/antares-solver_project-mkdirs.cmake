# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/source")
  file(MAKE_DIRECTORY "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/source")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/build"
  "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/project_build/antares-solver_project-prefix"
  "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/tmp"
  "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/stamp"
  "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/download"
  "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/antares-xpansion/antares-xpansion/build_strategy/build_deps/antares-solver/stamp${cfgdir}") # cfgdir has leading slash
endif()
