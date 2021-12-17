# Antares-Xpansion 
> Investment simulations for [Antares][antareswebsite] studies

[![Status][ubuntu_system_svg]][ubuntu_system_link]  [![Status][windows_vcpkg_svg]][windows_vcpkg_link] [![Status][centos_system_svg]][centos_system_link] [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=AntaresSimulatorTeam_antares-xpansion&metric=alert_status)][sonarcloud_link]

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)

# Introduction

The [Antares-Xpansion][xpansion-github] package, works along with RTE's adequacy software [Antares][antareswebsite] that is also [hosted on github][antares-github]. Antares-Xpansion aims at performing investment simulations for Antares studies. 

Please see the [Antares-Xpansion Documentation][readthedocs] for an introductory tutorial,
and a full user guide.
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

## Links:

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
    Antares study with the `-i` parameter. For example, the following command inside the `/antares-xpansion-install-dir/` runs one of the examples:
    ```shell
    antares-xpansion-launcher.exe -i examples\SmallTestFiveCandidates
    ```

#### Command line parameters

##### `-h, --help`

Show a help message and exit.

##### `--step {full, antares, problem_generation, benders, study_update}`

Default value: `full`. 

The execution of Antares-Xpansion consists of several steps that can be run separately. The
`--step` parameter allows to select the steps to execute:

| Step                 | Description                    |
| :--------            | ------------------------------------------------------------------------ |
| `antares`            | Launch Antares-Simulator once to get the Antares problem.
| `problem_generation` | Generate the full Antares-Xpansion problem using the user input and the output of the Antares-Simulator run. |
| `benders`            | Solve the investment optimization problem of Antares-Xpansion, using the [Benders decomposition][benders].|
| `study_update`       | Update the Antares study with the solution returned by the [Benders decomposition][benders] algorithm. |
| `full`               | Launch all steps in order: `antares` \> `problem_generation` \> `benders` \> `study_update`           |

##### `-i, --dataDir`

Specifies the Antares study folder path. Use quotes `“antares study path”` in case of a space in the path.

##### `--simulationName {last, your-antares-output-directory}`

Default value: `last`. 

Defines the name of the Antares-Simulator output directory that Antares-Xpansion uses to generate the expansion problem. If the value is `last`, the most recent run will be used. This option only has an effect when `--step` is among `{problem_generation, benders, study_update}`.

##### `-m, --method {sequential, mpibenders, mergeMPS}`

Default value: `sequential`. 

Sets the optimization method used by Antares-Xpansion.

| Option | Description                           |
| ---------- | ----------------------------------------------------------------------- |
| `sequential` | Launch sequential Benders decomposition.
| `mpibenders` | Launch the MPI version of Benders decomposition if the user has MPI. Use the option `-n` or `--np` to set the number of processes to use.  |
| `mergeMPS`   | Launch a frontal resolution of the investment problem (i.e. without decomposition). This is much more time-consuming than using Benders decomposition.|

##### `-n, --np`

Default value: 2. 

Sets the number of MPI processes to use for the MPI version of Benders decomposition. This option only has an effect when `-m` is set to `mpibenders`.

##### `--antares-n-cpu`

Default value: 1.

Sets the number of threads to use for Antares-Simulator in the initial `antares` step.

##### `--keepMps`

Default value: `False`.

If set to `True`, keeps `mps` files that encodes the problems solved by the optimizer. This option must be set to `True` if the user intends to launch the optimization several times on the same study (`--step benders`) without doing the other steps of Antares-Xpansion.
##### `-v, --version`

Show the Antares-Xpansion version.

##### `--antares-version`

Show the Antares-Simulator version (used in the `antares` step).

### Graphical user interface

Antares-Xpansion comes with a GUI. The GUI can be launched by running `antares-xpansion-ui.exe`. For now, this GUI is in the experimental phase.

## Technologies
Antares-Xpansion is developed mainly in **C++** and uses a **Python** runner
to drive the execution of multiple executables.

This software suite has been tested under:

*   Ubuntu 20.04 [![Status][ubuntu_system_svg]][ubuntu_system_link] 
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
* [documentation/](documentation)	 - Documentation generation with doxygen.
* [src/cpp/](src/cpp)      - Source code for cpp application (lpnamer, benders with MPI, benders without MPI, mergeMPS, study updater)
* [src/python/](src/python)       - Python script for Antares-Xpansion launch.


<!-- ## Definition of investment candidates

### Concept

The user of the package defines investment candidates. Each investment candidate
is characterized with the following properties:

 - **name**: name of the investment candidate (warnings : must not contains spaces)
 - **link**: link whose capacity will be invested
 - **annual cost per MW**: investment cost, per year and per MW
 - **unit size**: size, in MW, of an investment unit (e.g. one group of 300 MW)
 - **maximum units**: maximum number of units which can be built
 - **has link profile**: True if candidate has a capacity profile

Concretely, the investment decision will affect only the capacity of the Antares'
links. Investing in interconnections can be made directly with the package, while
investing in generation capacity or storage capacity can be made using the
so-called concept of "virtual nodes" with Antares.

The definition of all the investment candidates is given in a new input file,
located in the user folder of the study: ./user/expansion/candidates.ini. The
syntax used within this file is illustrated in the example below.

### Example

An example with two investments candidates, one in semi-base generation and one
in network capacity, is given below.

![simple_example](./media/simple-investments-candidates.png)

The invested semi-base generation in area 1 is shifted in the "virtual node"
invest_semibase. Within the optimization process, the capacity of the link
between area 1 and invest_semibase will be updated with the new invested capacity.
The `candidates.ini` file for this example will be the following one. This file has to be saved in the folder `user/expansion/`

```
[1]
name = semibase
link = area1 - invest_semibase
annual-cost-per-mw = 126000
unit-size = 200
max-units = 5
already_installed_capacity = 200

[2]
name = grid
link = area1 - area2
annual-cost-per-mw = 3000
unit-size = 500
max-units = 4
```

### Distributed generation

For the case of distributed generation and storage, the investment variables can be continuous, without steps of several MW. In that case, the properties unit-size and max-units can be replaced by the property max-investment, and the invested capacity will be able to take any real value between 0 and max-investment (in MW).

### Capacity profile

By default, the investment candidates offer a "perfect capacity". That is to say, when 1 MW is invested, this 1 MW will be fully available for all the hours of the year. However, by adding the property `link-profile`, one can define a time series (with an hourly time step) of the ratio between the invested capacity and the capacity which is actually available at a given hour. This feature can be used to model investments in (e.g.) intermittent generation or thermal generation with seasonalized maintenance shutdowns.

The property `link-profile` can be used as below:

```
[1]
name = solar_power
link = area1 - pv1
annual-cost-per-mw = 100000
max-investment = 10000
link-profile = pv1.txt
```

Where `pv1.txt` is a text file, located in the `user/expansion/capa/` folder of the study, and which contains the load factor time series of the candidate (one column of 8760 values between 0 and 1, or two columns if there is a direct and indirect profile on the link). When x MW of the candidate solar_power will be invested, the actual time series of available power will be equal to the product of `x` and the time series `pv1.txt`.

## Method and settings

### Benders decomposition

The method used to perform the optimal expansion is a benders decomposition in which:

- The slave problem is an Antares simulation, where the installed capacities of the investment candidates are fixed,
- The master problem is a MILP, in which the decision variables are the installed capacities of each investment candidate.

The objective function which is minimized, is the sum of the expected annual operation cost (column OV. COST + column HURDLE COST of the outputs of Antares) and the investment annuity (as defined in the `candidates.ini` file).

Concretely, the package will run Antares iteratively. Depending on the number of candidates, the convergence to the optimal solution can be quite long (several hundred iterations, and so several hundred Antares' simulations). Some algorithmic settings are however available and propose different tradeoffs between the accuracy of the solution and the computation time.

### Settings

The different settings can be modified by the user of the package. All the settings are saved in a file `settings.ini` located in the folder `user/expansion/`.

 - `optimality_gap`: The optimality gap can take any numeric value. The optimality gap is the maximum possible distance (in euros) between the solution returned by the method and the optimal solution.
 - `master`: Can take two values, `relaxed` or `integer`. Defines whether or not should integer variables be used for the master problem.
 - `uc_type`: The UC (Unit Commitment) type can take two different values, `expansion_fast` or `expansion_accurate`. It defines which mode will be used in Antares. `expansion_fast` correponds to the fast mode of Antares in which the minimum up/down and minimum stable power constraints are relaxed whereas `expansion_accurate` takes into account the technical constraints of the thermal power plants as well as their start-up costs.

An example of a `settings.ini` file with the appropriate syntax is given below.
```
uc_type = expansion_fast
master = integer
optimality_gap = 0
max_iteration = 100
```
Note that the optimality gap can also be given relatively to be best found solution by entering a % after the numeric value of the setting. In that case, the optimality is between 0%and 100% and the decimal separator is a point (.).

#### Which settings should I use for my expansion problem?

##### 1. **If I have to run several expansion optimizations of different variants of a study and compare them**

In that case, if the optimal solutions are not returned by the package, the comparison of several variants can be tricky as the imprecision of the method might be in the same order of magnitude as the changes brought by the input variations.
It is therefore advised to be as closed as possile from the optimum of the expansion problem. To do so, the two following conditions should necessarily be fulfilled:
- set the `optimality_gap` to zero.
[Remark : even with the conditions mentionned above, the reslult might be slighty different from the optimum due to numeric approximations, this can be partly solved by putting to optimality gap to -Inf]

##### 2. **If I'm building one consistent generation/transmission scenario**

As the optimal solution is not more realistic than an approximate solution of the modelled expansion problem. The settings can be less constraining with:
 - an `optimality_gap` of a few million euros -->



[ubuntu_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Ubuntu%20CI%20(system%20libs)/badge.svg
[ubuntu_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Ubuntu%20CI%20(system%20libs)"
[windows_vcpkg_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Windows%20CI%20(VCPKG)/badge.svg
[windows_vcpkg_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Windows%20CI%20(VCPKG)"
[centos_system_svg]: https://github.com/AntaresSimulatorTeam/antares-xpansion/workflows/Centos7%20CI%20(system%20libs)/badge.svg
[centos_system_link]: https://github.com/AntaresSimulatorTeam/antares-xpansion/actions?query=workflow%3A"Centos7%20CI%20(system%20libs)"
[sonarcloud_link]: https://sonarcloud.io/dashboard?id=AntaresSimulatorTeam_antares-xpansion
[antares_xpansion_release_url]: https://github.com/AntaresSimulatorTeam/antares-xpansion/releases

[xpansion-github]: https://github.com/AntaresSimulatorTeam/antares-xpansion
[antares-github]: https://github.com/AntaresSimulatorTeam/Antares_Simulator
[readthedocs]: https://antares-xpansion.readthedocs.io/
[readthedocs-antares]: https://antares-doc.readthedocs.io/
[antareswebsite]: https://antares-simulator.org
[developer-guide]: https://antares-xpansion.readthedocs.io/en/stable/developer-guide/install_from_sources/0-INSTALL/
[user-guide]: https://antares-xpansion.readthedocs.io/en/stable/user-guide/optimization-principles/investment-problem/
[benders]: https://antares-xpansion.readthedocs.io/en/latest/user-guide/optimization-principles/investment-problem/