Antares-Xpansion Changelog
=================

v0.6.0
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
