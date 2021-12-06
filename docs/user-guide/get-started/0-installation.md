# Installation

Antares-Xpansion is developed mainly in **C++** and uses a **Python** runner
to drive the execution of multiple executables.

This software suite has been tested under:

*   Ubuntu 20.04 [![Status][ubuntu_system_svg]][ubuntu_system_link]
*   Microsoft Windows with Visual Studio 2019 (64-bit) [![Status][windows_vcpkg_svg]][windows_vcpkg_link]
*   Centos 7 [![Status][centos_system_svg]][centos_system_link]

Antares-Xpansion is built using CMake.

## Download Antares-Xpansion

To download the latest version of Antares-Xpansion, visit [Antares-Xpansion repository][antares_xpansion_release_url] and download the binary that matches your platform. If you prefer to build the software from source file, please refer to the [developer guide](../../developer-guide/install_from_sources/0-INSTALL.md).

## Installation instruction

Once you have downloaded the archive, extract the files in the folder of your choice. You are now ready to use Antares-Xpansion. See [launching an optimization](4-launching-optimization.md) for instructions on how to launch the software.

[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/linux-system.yml/badge.svg
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/linux-system.yml
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Windows%20CI%20(VCPKG)/badge.svg
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Windows%20CI%20(VCPKG)"
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Centos7%20CI%20(system%20libs)/badge.svg
[centos_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Centos7%20CI%20(system%20libs)"
[antares_xpansion_release_url]: https://github.com/AntaresSimulatorTeam/antares-xpansion/releases