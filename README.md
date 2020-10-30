# Antares XPansion
[![Status][linux_system_svg]][linux_system_link]  [![Status][windows_vcpkg_svg]][windows_vcpkg_link]

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

TODO : add description

[linux_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Linux%20CI%20(system%20libs)/badge.svg

[linux_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Linux%20CI%20(system%20libs)"

[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Windows%20CI%20(VCPKG)/badge.svg

[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Windows%20CI%20(VCPKG)"


# Links:

- RTE web site  : http://www.rte-france.com/

# Installation

This software suite has been tested under:

*   Ubuntu 16.04 and up (64-bit) [![Status][linux_system_svg]][linux_system_link] 
*   Microsoft Windows with Visual Studio 2019 (64-bit) [![Status][windows_vcpkg_svg]][windows_vcpkg_link]

Antares XPansion is built using CMake.
For installation instructions, please visit [INSTALL.md](INSTALL.md)

# Source Code Content

* [INSTALL](INSTALL.md)           - Installation and building instructions.
* [README](README.md)             - This file.
* [cmake/](cmake)        - files for initializing a solution ready for compilation. 
* [conception/](conception)        - json output description 
* [data_test/](data_test)	 - Free sample data sets. 
* [documentation/](documentation)	 - Documentation generation with doxygen
* [src/src_cpp/](src/src_cpp)      - source code for cpp application (lpnamer, benders with mpi, benders without MPI, merge)
* [src/src_python/](src/scr_python)       - python script for Antares XPansion launch.