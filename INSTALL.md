# antares-xpansion CMake Build Instructions

 [Supported OS](#supported-os)| [CMake version](#cmake-version) | [Git version](#git-version) | [Dependencies](#dependencies) | [Building](#building-antares-solution) |

## [Supported OS](#supported-os)
antares-xpansion compilation is tested on :
- Windows see (INSTALL-windows.md)
- Ubuntu see (INSTALL-ubuntu.md)
- Centos7 see (INSTALL-windows.md)

## [CMake version](#cmake-version)
CMake 3.x must be used.

## [Git version](#git-version)
Git version must be above 2.15 for external dependencies build because `--ignore-whitespace` is not used by default and we have an issue with OR-Tools compilation of ZLib and application of patch on Windows (see https://github.com/google/or-tools/issues/1193).

## [Dependencies](#deps)
 ANTARES XPansion V2 depends on severals mandatory libraries. 
 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [Google Test](https://github.com/google/googletest)
 - [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius)
 - Boost : mpi serialization (Only for MPI benders compilation)
 - [Doxygen](https://www.doxygen.nl/index.html) for documentation generation
 - [GraphViz](https://graphviz.org/) for doxygen use
 
 Installation of these dependencies is described in OS specific antares-xpansion install procedure.