[![Status][ubuntu_system_svg]][ubuntu_system_link]  [![Status][windows_vcpkg_svg]][windows_vcpkg_link] [![Status][centos_system_svg]][centos_system_link] [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=AntaresSimulatorTeam_antares-xpansion&metric=alert_status)][sonarcloud_link]

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)

# Introduction

The [Antares-Xpansion][xpansion-github] package, works along with RTE's adequacy software [Antares][antareswebsite] that is also [hosted on github][antares-github]. Antares-Xpansion aims at performing investment simulations for Antares studies. Check out the [Antares-Simulator documentation][readthedocs-antares] for more insights on Antares. 

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

!!! info "Goal of Antares-Xpansion" 
    Antares-Xpansion optimizes the _investments_ in order to minimize the global cost, which is the sum of the **expected operation cost during one year** and the **investment annuity**.

Antares-Xpansion is currently under development. Feel free to submit any issue.


[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-system-deps-build.yml/badge.svg
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-system-deps-build.yml
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg-deps-build.yml/badge.svg
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg-deps-build.yml
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/centos7-system-deps-build.yml/badge.svg
[centos_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/centos7-system-deps-build.yml
[sonarcloud_link]: https://sonarcloud.io/dashboard?id=AntaresSimulatorTeam_antares-xpansion

[xpansion-github]: https://github.com/AntaresSimulatorTeam/antares-xpansion
[antares-github]: https://github.com/AntaresSimulatorTeam/Antares_Simulator
[readthedocs-antares]: https://antares-doc.readthedocs.io/
[antareswebsite]: https://antares-simulator.org
