# Installation

Antares-Xpansion is developed mainly in **C++** and uses a **Python** runner
to drive the execution of multiple executables.

This software suite has been tested under:

*   Ubuntu 20.04 [![Status][ubuntu_system_svg]][ubuntu_system_link]
*   Microsoft Windows with Visual Studio 2019 (64-bit) [![Status][windows_vcpkg_svg]][windows_vcpkg_link]
*   Centos 7 [![Status][centos_system_svg]][centos_system_link]
*   Oracle Linux 8 [![Status][oracle_svg]][oracle_link]

Antares-Xpansion is built using CMake.

## Download Antares-Xpansion

To download the latest version of Antares-Xpansion, visit [Antares-Xpansion repository][antares_xpansion_release_url] and download the binary that matches your platform. If you prefer to build the software from the source files, please refer to the [developer guide](../../developer-guide/install_from_sources/0-INSTALL.md).

## Installation instruction

Once you have downloaded the archive, extract the files in the folder of your choice. You are now ready to use Antares-Xpansion. 

To check the installation, you can open a command prompt in the Antares-Xpansion install directory and run the following command to execute one of the examples included in the package:

```
antares-xpansion-launcher.exe -i examples\SmallTestFiveCandidates
```

See [Launch the optimization](launching-optimization.md) for more details on how to launch the software.

[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-release.yml/badge.svg?query=branch%3Adevelop
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-release.yml?query=branch%3Adevelop
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg.yml/badge.svg?query=branch%3Adevelop
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg.yml?query=branch%3Adevelop
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/centos-release.yml/badge.svg?query=branch%3Adevelop
[centos_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/centos-release.yml?query=branch%3Adevelop
[oracle_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ol8-release.yml/badge.svg?query=branch%3Adevelop
[oracle_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ol8-release.yml?query=branch%3Adevelop
[antares_xpansion_release_url]: https://github.com/AntaresSimulatorTeam/antares-xpansion/releases