<div style="display: flex; align-items: center; margin-bottom: 30px; justify-content: center;">
  <img
    src="assets/antares.svg"
    alt="Antares Logo"
    style="height: 150px; width: 150px; margin-right: 30px;"
  />
  <div>
    <h1 style="margin: 0;">Antares Xpansion</h1>
    <p style="margin: 5px 0 0 0; font-size: 1.2em; color: #666;">
      Antares investment package
    </p>
  </div>
</div>

[![Status][ubuntu_system_svg]][ubuntu_system_link]  [![Status][windows_vcpkg_svg]][windows_vcpkg_link] [![OL8 CI Status][oracle_svg]][oracle_link] [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=AntaresSimulatorTeam_antares-xpansion&metric=alert_status)][sonarcloud_link]

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)

!!! note "Role of this documentation"

    This documentation corresponds to the technical documentation of Antares
    Xpansion: [input](./reference/inputs/index.md) and 
    [output](./reference/inputs/index.md) file formats, 
    developer guide, complete
    [changelog](./changelog/CHANGELOG.md)... 
    Check out the [Antares user documentation][readthedocs-antares] for more insights on Antares. 

## Introduction

The [Antares Xpansion][xpansion-github] package, works along with RTE's adequacy software 
[Antares][antareswebsite] that is also [hosted on github][antares-github].
Antares Xpansion aims at performing investment simulations for Antares studies.

Antares Xpansion optimizes the investments on new capacities and transmission lines for an Antares study. Typical uses of Antares Xpansion are for example:

- **long-term scenario building**: build an economically consistent long-term generation mix,
- **transmission expansion planning** : compute the network development which maximizes social welfare.

!!! info "Goal of Antares Xpansion" 

    Antares Xpansion optimizes the _investments_ in order to minimize the global cost, which is the sum of the **expected operation cost during one year** and the **investment annuity**.

## Antares study

In an Antares study, the user builds a power system with a network of zones
characterised by power plants (with their constraints e.g. max power and costs),
power consumption and power transfer between zones (with the import-export transfer capacity and costs).

Antares performs probabilistic simulations of the system
throughout many year-long scenarios made of 8760 hourly
time frames each.
The goal of the Antares simulation is to minimize the
**expected operation cost during one year**.

## Antares Xpansion simulation

Given an Antares simulation the user can define some
_investment candidates_ in the power network in order to:

- increase or create the transfer capacity between areas,
- increase or create the maximum power of a generation facility.

Each _investment candidate_ can potentially decrease the variable operational cost
of the power system, but induces an additional **investment annuity** that includes:

- Annualized investment costs to physically build the facility,
- Fixed operational costs and maintenance costs to sustain the operation.

## Contributing

Antares Xpansion is currently under development. Feel free to submit any issue
or [contribute to the documentation](./developer-guide/contributing/doc-guidelines.md).


[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-release.yml/badge.svg?query=branch%3Adevelop
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ubuntu-release.yml?query=branch%3Adevelop
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg.yml/badge.svg?query=branch%3Adevelop
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/windows-vcpkg.yml?query=branch%3Adevelop
[oracle_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ol8-release.yml/badge.svg?query=branch%3Adevelop
[oracle_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions/workflows/ol8-release.yml?query=branch%3Adevelop
[sonarcloud_link]: https://sonarcloud.io/dashboard?id=AntaresSimulatorTeam_antares-xpansion

[xpansion-github]: https://github.com/AntaresSimulatorTeam/antares-xpansion
[antares-github]: https://github.com/AntaresSimulatorTeam/Antares_Simulator
[readthedocs-antares]: https://antares-doc.readthedocs.io/
[antareswebsite]: https://antares-simulator.org
