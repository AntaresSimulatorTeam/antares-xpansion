# *antares-xpansion* CMake Build Instructions

 [Supported OS](#supported-os)|[C++ version](#cpp-version)|[CMake version](#cmake-version) |[Python version](#python-version) | [Git version](#git-version) | [Dependencies](#dependencies) | [Building](#building-antares-solution) |

## C/I status
| OS     | Status|
|:-------|--------|
| Ubuntu  | [![Status][linux_system_svg]][linux_system_link] |
| Windows  | [![Status][windows_vcpkg_svg]][windows_vcpkg_link] |
| Centos7  | [![Status][centos_system_svg]][centos_system_link] |

[linux_system_svg]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/workflows/Linux%20CI%20(system%20libs)/badge.svg

[linux_system_link]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/actions?query=workflow%3A"Linux%20CI%20(system%20libs)"

[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/workflows/Windows%20CI%20(VCPKG)/badge.svg

[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/actions?query=workflow%3A"Windows%20CI%20(VCPKG)"

[centos_system_svg]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/workflows/Centos7%20CI%20(system%20libs)/badge.svg

[centos_system_link]: https://github.com/AntaresSimulatorTeam/Antares_Simulator/actions?query=workflow%3A"Centos7%20CI%20(system%20libs)"

## [Supported OS](#supported-os)
*antares-xpansion* compilation is tested on :
- Windows see [INSTALL-windows.md](INSTALL-windows.md)
- Ubuntu see [INSTALL-ubuntu.md](INSTALL-ubuntu.md)
- Centos7 see [INSTALL-centos.md](INSTALL-centos.md)

## [C++ version](#cpp-version)
The compilation of  *antares-xpansion* requires C++17 support.

## [CMake version](#cmake-version)
CMake 3.x must be used.

## [Python version](#python-version)
Python 3.x must be used.

## [Git version](#git-version)
Git version must be above 2.15 for external dependencies build because `--ignore-whitespace` is not used by default and we have an issue with OR-Tools compilation of ZLib and application of patch on Windows (see https://github.com/google/or-tools/issues/1193).

## [Dependencies](#deps)
*antares-xpansion* depends on severals mandatory libraries. 
 - [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
 - [Google Test](https://github.com/google/googletest)
 - [OR-Tools](https://github.com/AntaresSimulatorTeam/or-tools/tree/rte_dev_sirius)
 - Boost : mpi, serialization (Only for MPI benders compilation)
 - [Doxygen](https://www.doxygen.nl/index.html) for documentation generation
 - [GraphViz](https://graphviz.org/) for doxygen use
 - [OpenSSL](https://github.com/openssl/openssl)
 - [CURL](https://github.com/curl/curl)
 - [Sirius Solver](https://github.com/AntaresSimulatorTeam/sirius-solver/tree/Antares_VCPKG) (fork from [RTE](https://github.com/rte-france/sirius-solver/tree/Antares_VCPKG))
 
 Installation of these dependencies is described in OS specific *antares-xpansion* install procedure.