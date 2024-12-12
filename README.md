# Antares-Xpansion 
> Investment simulations for [Antares][antareswebsite] studies

[![Ubuntu CI Status][ubuntu_system_svg]][ubuntu_release]  [![Windows CI Status][windows_vcpkg_svg]][windows_vcpkg_link] [![Centos CI Status][centos_system_svg]][centos_system_link][![OL8 CI Status][oracle_svg]][oracle_link] [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=AntaresSimulatorTeam_antares-xpansion&metric=alert_status)][sonarcloud_link]

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)

# Introduction

The [Antares-Xpansion][xpansion-github] package, works along with RTE's adequacy software [Antares][antareswebsite] that is also [hosted on github][antares-github]. Antares-Xpansion aims at performing investment simulations for Antares studies. 

Please see the [Antares-Xpansion Documentation][readthedocs] for an introductory tutorial,
and a full user guide.

For developers: [here][xpansion-doxygen] you can find doxygen code documentation.

Check out the [Antares-Simulator documentation][readthedocs-antares] for more insights on Antares. 

Antares-Xpansion optimizes the investments on new capacities and transmission lines for an Antares study. Typical uses of Antares-Xpansion are for example:

- **long-term scenario building**: build an economically consistent long-term generation mix,
- **transmission expansion planning** : compute the network development which maximizes social welfare.

## Antares study

In an Antares study, the user builds a power system with a network of zones
characterised by power plants (with their constraints e.g. max power and costs),
power consumption and power transfer between zones (with the import-export transfer capacity and costs).

Antares performs probabilistic simulations of the system
throughout many year-long scenarios made of 8760 hourly
time frames each.
The goal of the Antares simulation is to minimize the
**expected operation cost during one year**.

## Antares-Xpansion simulation

Given an Antares simulation the user can define some
_investment candidates_ in the power network in order to:

- (increase or create) the transfer capacity between areas,
- (increase or create) the maximum power of a generation facility.

Each _investment candidate_ can potentially decrease the variable operational cost
of the power system, but induces an additional **investment annuity** that includes:

- Annualized investment costs to physically build the facility,
- Fixed operational costs and maintenance costs to sustain the operation.

Antares-Xpansion optimizes the _investments_ in order to minimize the global cost, which is the sum of the **expected operation cost during one year** and the **investment annuity**.

Antares-Xpansion is currently under development. Feel free to submit any issue.

## Links

- [Official Documentation][readthedocs]
- [Antares website][antareswebsite]
- [Antares github][antares-github]
- [Antares documentation][readthedocs-antares]

## Installation

Antares-Xpansion is currently released as standalone-portable solution.
It can be run either using the single file executable or
an archive including multiple binaries called by a driver.

To download the latest version of Antares-Xpansion, visit the [release page][antares_xpansion_release_url] of Antares-Xpansion repository and download the binary that matches your platform. If you prefer to build the software from the source files, please refer to the [developer guide][developer-guide].

Once you have downloaded the archive, extract the files in the folder of your choice. You are now ready to use Antares-Xpansion. 

To check the installation, you can open a command prompt in the Antares-Xpansion install directory and run the following command to execute one of the examples included in the package:

```shell
antares-xpansion-launcher.exe -i examples\SmallTestFiveCandidates
```

## Getting started

The user should first create an Antares study 
that allows for the definition of investment candidates
(see the [user guide][user-guide] for detailed informations)
and create the `candidates.ini` and `settings.ini` files
in the directory `study_path/user/expansion`.

Since v0.6.0, Antares-Xpansion includes an experimental graphical interface but it is optimally used as a command line prompt. 

### Command-line usage

1.  Open a command prompt in your Antares-Xpansion install directory. By default (on Windows) it is named `antaresXpansion-x.y.z-win64`
where `x.y.z` is the version number.

2.  Run `antares-xpansion-launcher.exe` and choose the path to the
    Antares study with the `-i` parameter. For example, the following command inside the `antares-xpansion-install-dir\` runs one of the examples:
    ```shell
    antares-xpansion-launcher.exe -i examples\SmallTestFiveCandidates
    ```


## Technologies
Antares-Xpansion is developed mainly in **C++** and uses a **Python** runner
to drive the execution of multiple executables.

This software suite has been tested under:

*   Ubuntu 20.04 [![Status][ubuntu_system_svg]][ubuntu_release] 
*   Microsoft Windows with Visual Studio 2019 (64-bit) [![Status][windows_vcpkg_svg]][windows_vcpkg_link]
*   Centos 7 [![Status][centos_system_svg]][centos_system_link] 

Antares-Xpansion is built using CMake.
For build instructions, please visit the [developer guide][developer-guide].

## Source Code Content

* [README](README.md)             - This file.
* [cmake/](cmake)        - Files for initializing a solution ready for compilation. 
* [conception/](conception)        - json output description. 
* [docs/](docs) - Markdown files for documentation generation.  
* [data_test/](data_test)	 - Free sample data sets.
* [src/cpp/](src/cpp)      - Source code for cpp application (lpnamer, benders with MPI, benders without MPI, mergeMPS, study updater)
* [src/python/](src/python)       - Python script for Antares-Xpansion launch.



[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-release.yml/badge.svg?query=branch%3Adevelop
[ubuntu_release]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-release.yml?query=branch%3Adevelop
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg.yml/badge.svg?query=branch%3Adevelop
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg.yml?query=branch%3Adevelop
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/centos-release.yml/badge.svg?query=branch%3Adevelop
[centos_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/centos-release.yml?query=branch%3Adevelop
[oracle_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ol8-release.yml/badge.svg?query=branch%3Adevelop
[oracle_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ol8-release.yml?query=branch%3Adevelop
[sonarcloud_link]: https://sonarcloud.io/dashboard?id=AntaresSimulatorTeam_antares-xpansion
[antares_xpansion_release_url]: https://github.com/AntaresSimulatorTeam/antares-xpansion/releases

[xpansion-github]: https://github.com/AntaresSimulatorTeam/antares-xpansion
[xpansion-doxygen]: https://antaressimulatorteam.github.io/antares-xpansion
[antares-github]: https://github.com/AntaresSimulatorTeam/Antares_Simulator
[readthedocs]: https://antares-xpansion.readthedocs.io/
[readthedocs-antares]: https://antares-doc.readthedocs.io/
[antareswebsite]: https://antares-simulator.org
[developer-guide]: https://antares-xpansion.readthedocs.io/en/stable/developer-guide/install_from_sources/0-INSTALL/
[user-guide]: https://antares-xpansion.readthedocs.io/en/stable/user-guide/optimization-principles/investment-problem/
[benders]: https://antares-xpansion.readthedocs.io/en/latest/user-guide/optimization-principles/investment-problem/
