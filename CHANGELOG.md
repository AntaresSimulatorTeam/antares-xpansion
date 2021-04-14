Antares-Xpansion Changelog
=================
v0.2.0 (04/2021)
-------------------- 
### Features

 - Add `update` step to update antares study links with calculated investments
 
### Bug fixes

 - Invalid best iteration display in logs
 - Rounding problem in writing MPS with or-tools
 - Antares-Xpansion can't be launched twice at the same time
 - Windows: Add Microsoft Visual Studio needed redistribuable to installer
 - Linux: wrong antares-solver executable permission and install directory


v0.1.0 (03/2021)
-------------------- 
We’re happy to announce the first open-source version v0.1.0 of Antares-Xpansion.
**Antares-Xpansion** optimizes the installed capacities of an ANTARES study.

The investment decisions are optimized by running ANTARES' simulations iteratively. At each iteration, the installed capacity of the investments are updated, and the simulations are repeated until the total costs have converged to a minimum. The total cost evaluated in this problem are the sum of the **expected operation cost during one year** and **the investment annuity**. 
The user defines investment candidates. Candidate capacities for investment are necessarily links from an ANTARES study.  Investment candidates can be:
 - transmission capacity between two areas
 - thermal generation capacity (located in a virtual node)
 - renewable generation capacity (located in a virtual node)
 - or even flexibilities (located in virtual nodes)

## Notes
This version is only compatible with Antares v8.0 [Antares-Simulator](https://antares-simulator.org). The Antares binaries needed by Antares-Xpansion are available in the installation package of AntaresXpansion

An annual cost, a potential, a link-profile or an already-installed-capacity can be defined for each candidate. Linear constraints between the invested capacities of investment candidates can also be imposed

The optimization is launched from the command prompt, where the details of the iterations are displayed

The optimization can be launched on large studies. Computing performance can be improved by using MPI.
