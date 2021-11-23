[![Status][ubuntu_system_svg]][ubuntu_system_link]  [![Status][windows_vcpkg_svg]][windows_vcpkg_link] [![Status][centos_system_svg]][centos_system_link] [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=AntaresSimulatorTeam_antares-xpansion&metric=alert_status)](https://sonarcloud.io/dashboard?id=AntaresSimulatorTeam_antares-xpansion)

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)

The Antares-Xpansion package works along with RTE's adequacy software [Antares][antareswebsite] that is also [hosted on github][antares-github]. Antares-Xpansion aims at performing investment simulations for Antares studies. Visit the [Antares-Simulator documentation][readthedocs-antares] for more insights on Antares. 

## Introduction

Antares-Xpansion optimizes the installed capacities of an Antares study.

Typical uses of Antares-Xpansion are for example:

- **long-term scenario building**: build an economically consistent long-term generation mix,
- **transmission expansion planning** : compute the network development which maximizes social welfare.


### Antares study

In an Antares study, the user builds a power system with a network of zones
characterised by power plants (with their constraints e.g., max power etc. and costs),
power consumption and power transfer between zones (with the import-export transfer capacity and costs).

Antares performs probabilistic simulations of the system
throughout many year-long scenarios made of 8760 hourly
time-frames each.
The goal of the simulation is to minimize the
**expected operation cost during one year**.

### Antares-Xpansion simulation

Given an Antares simulation the user can define some
_investment candidates_ in the power network such as:

- (increase or create) transfer capacity between to areas,
- (increase or create) maximum power of a generation facility.

Each _investment candidate_ can potentially decrease the operational cost
of the power system, but is also characterised by one or more costs such as:

- annualized investment costs to physically build it the facility,
- operational costs and maintenance costs to sustain the operation.

>Antares-Xpansion optimizes the _investments_ in order
>to minimize the global cost, that is the sum of the
>**expected operation cost during one year**
>and the **investment annuity**.

Antares-Xpansion is currently under development. Feel free to submit any issue.


[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Ubuntu%20CI%20(system%20libs)/badge.svg
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Ubuntu%20CI%20(system%20libs)"
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Windows%20CI%20(VCPKG)/badge.svg
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Windows%20CI%20(VCPKG)"
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Centos7%20CI%20(system%20libs)/badge.svg
[centos_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Centos7%20CI%20(system%20libs)"

[antares-github]: https://github.com/AntaresSimulatorTeam/Antares_Simulator
[readthedocs]: https://antares-xpansion.readthedocs.io/
[readthedocs-antares]: https://antares-doc.readthedocs.io/
[antareswebsite]: https://antares-simulator.org
