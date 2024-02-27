Antares-Xpansion changelog
=================

v1.2.2 (02/2024)
--------------------------------------------------------

- Update Antares simulator to 8.8.3

v1.2 (10/2023)
--------------------------------------------------------

### Features

- Support "named mps" from Antares simulator
- Use Antares Simulator 8.8.0-rc2

### Bug fixes

- Fix issue on windows with concurrent access to log files

v1.1.1 (09/2023)
--------------------

### Fix

- Update performance graph in the documentation

v1.1.0 (09/2023)
--------------------

### Features

- Implementation of the Benders by batch algorithm : performance improvements over the classical Benders algorithm
- New parameter `batch_size` in `settings.ini` to tune the Benders by batch algorithm, more details [here](../user-guide/get-started/settings-definition.md#batch_size)
- Logs improvements:
    - Print user and hostname in the standard output and in `reportbenders.txt`
    - Print both walltime and cumulative CPU time for subproblems resolution


### For developers

- MPI dependency is now mandatory as `benderssequential` and `bendersmpi` have been merged in a single `benders` executable 

v1.0.3 (02/2023)
--------------------

### Features

- Use Antares-Simulator 8.5.0

### Bug

- Fix sensitivity not working with AntaresWeb

v1.0.2 (01/2023)
--------------------

### Bug fix

- Fix missing performance graph from release

v1.0.1 (01/2023)
--------------------

### Features

- Use Antares-Simulator 8.4.2

v1.0.0 (12/2022)
--------------------

### Features

- Use Antares-Simulator 8.4.1
- Performance improvement in execution time
    - In-out stabilisation of the Benders algorithm : new parameters `separation_parameter` and `relaxed_optimality_gap`
    to tune the stabilization, more
    information [here](../user-guide/get-started/settings-definition.md#separation_parameter)
    - Parallelization of the `problem_generation` step
- Performance improvement in disk space consumption : in full run mode i.e with option `--step full` (default behavior), Antares-Xpansion outputs are zipped within Antares study archive
- Change the default value of `relative_gap` from `1e-12` to `1e-6`

### Documentation

- Add new page for [performance history](../user-guide/performance-history/perf-graphs.ipynb)

v0.8.0 (10/2022)
--------------------

### Features

- Use Antares-Simulator 8.2.2
- Allow the use of different chronicles of link profile for each Monte-Carlo year. More
  information [here](../user-guide/get-started/candidate-definition.md#using-different-profiles-depending-on-the-monte-carlo-year)
- New directories to store additional constraints files and weights files:
    - Additional constraints files must be stored in `user/expansion/constraints`
    - Weights files must be stored in `user/expansion/weights`

### Bug fixes
- Allow the `capa` folder to be missing if unused
- The algorithm now properly stops after `max_iteration` instead of `max_iteration + 1` iterations

v0.7.0 (06/2022)
--------------------
### Features

- Add the possibility to perform sensitivity studies on optimal investment solutions
- Antares-Xpansion outputs are written in the `expansion` directory instead of the `lp` directory
- Add `timelimit` option as a stopping criterion for the Antares-Xpansion algorithm
- Add `log_level` option to choose the verbosity level of the solver
- Add `resume` step, to provide the possibility to resume an unfinished study
- Bump antares version used by Xpansion to 8.1.1
### Bug fixes

### For developers

- Remove documentation generation with doxygen
- Use C++17

v0.6.0 (12/2021)
--------------------
### Features

- Antares-Xpansion is now compatible with Antares v8.1 studies
- Add `relative_gap` option as a stopping criterion for the Antares-Xpansion algorithm 
- Add `--antares-n-cpu` option to use Antares-Simulator parallelism
- Add `-v, --version` option to show Antares-Xpansion version
- Add `--antares-version` option to show Antares-Simulator version
- Add `mergeMPS` option to use a frontal resolution of the investment problem instead of Benders decomposition
- Add `--installDir` option to be able to launch Antares-Xpansion from another directory than its own
- Set default value of `--simulationName` to `last` in order to use the last Antares simulation

### Bug fixes
- Path to binaries directory was not found in some situations with python launcher and in python exclusive package
- Correct bug when there are too many zeros in a link-profile 

### For developers
- cpp `lpnamer` library is split into smaller and consistent libraries
- Each Antares-Xpansion step has its own driver (python class) instead of the all-in driver
- Upgrade to a new version of `antares-deps` (bug fix)

v0.5.0 (10/2021)
--------------------
### Features

 - In benders optimization, master problem .mps is written after each resolution
 - Several investment candidates on the same link  
 - Review of `candidates.ini` file content (remove `has-link-profile` and add `enable` feature)
 
### Bug fixes

 - Option `keepMps` was still removing some files needed for lp step
 - Correction of lp step call if it was the only step asked by user
 - Correction of optimization if hurdles cost were used in Antares-Simulator
 - Correction of optimization with binary additionnal constraint between investment candidates

### For developers
 - Fix mergemps optimization in debug mode
 - Fix vcpkg on Github Actions
 - Add build cache for Github Actions (Ubuntu only) to speed up the build
 - Add sonarcloud analysis
 - Use of docker image for build on centos7
 - Add read-the-docs documentation
 - Generation of pdf user guide from read-the-docs documentation
 - Update Cmake to support Xpress on centos7
 
v0.4.0 (07/2021)
-------------------- 
### Features

 - A new multi solver interface is introduced, we no longer use OR-tools
 - The user can choose different solvers
 - The optimality gap is no longer hard-coded and the user can define it in the inputs
 - In case of unfeasible problems, the program now prints a console and file outputs
 - Solver traces can be activated by specific option in benders/bendersmpi console applications
 - Benders console information are now also printed on a specific file
 
### Bug fixes

 - Remove erroneus value from the `"problem_status"` entry  of the output json file
 - Fix display of overall cost of the best solutions

v0.3.0 (05/2021)
-------------------- 
### Features

 - Add `yearly-weights` file support to define a weight for each antares study MC years
 - Clean of intermediate result file after each simulation step
 - Creation of a Antares-Xpansion version with only one executable
 
### Bug fixes

 - Use study stored in mount network drive 
 - Problem accepting capital letters in candidate areas
 - Small alignment fix in output

v0.2.0 (04/2021)
-------------------- 
### Features

 - Add `update` step to update antares study links with calculated investments
 
### Bug fixes

 - Invalid best iteration display in logs
 - Rounding problem in writing MPS with OR-tools
 - Antares-Xpansion can't be launched twice at the same time
 - Windows: Add Microsoft Visual Studio needed redistribuable to installer
 - Linux: wrong `antares-solver` executable permission and install directory


v0.1.0 (03/2021)
-------------------- 
We’re happy to announce the first open-source version v0.1.0 of Antares-Xpansion.
Antares-Xpansion optimizes the installed capacities of an Antares study.

The investment decisions are optimized by running Antares' simulations iteratively. At each iteration, the installed capacity of the investments are updated, and the simulations are repeated until the total costs have converged to a minimum. The total cost evaluated in this problem are the sum of the **expected operation cost during one year** and **the investment annuity**. 
The user defines investment candidates. Candidate capacities for investment are necessarily links from an Antares study.  Investment candidates can be:
 - transmission capacity between two areas
 - thermal generation capacity (located in a virtual node)
 - renewable generation capacity (located in a virtual node)
 - or even flexibilities (located in virtual nodes)

## Notes
This version is only compatible with Antares v8.0 [Antares-Simulator](https://antares-simulator.org). The Antares binaries needed by Antares-Xpansion are available in the installation package of Antares-Xpansion.

An annual cost, a potential, a link-profile or an already-installed-capacity can be defined for each candidate. Linear constraints between the invested capacities of investment candidates can also be imposed.

The optimization is launched from the command prompt, where the details of the iterations are displayed.

The optimization can be launched on large studies. Computing performance can be improved by using MPI.
