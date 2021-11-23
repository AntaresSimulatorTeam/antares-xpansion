# Antares-Xpansion simulation procedure

An Antares-Xpansion simulation is currently run in 4 separate steps.

The basic idea behind an Antares-Xpansion simulation is to let Antares-Simulator do the work of
reading the Antares study and building the mathematical optimisation problems without knowledge of the 
investment candidates then build and solve the Antares-Xpansion problem.

There are 4 steps in a full Antares-Xpansion simulation.

## 1- Optimization problems generation
`antares-solver` is run using Antares-Xpansion options. The weekly optimization problems (1st and 2nd optimization) are written as `.mps` files.
In this mode Antares-Simulator write some additional files that add a business context of the linear problems
(column id and variable map for example).

## 2- Modification of problems to introduce investment variables
The user specifies the Antares-Xpansion-specific data inside the `user/expansion/` directory.
The investment candidates are specified in the `user/expansion/candidates.ini` file.
The Antares-Xpansion simulation is built at this stage by modifying the `.mps` problems provided by
Antares by adding the information of the investment candidates. These "problems" are then written
again as `.mps` files and becomes the so-called **satellite problems**.

This stage is also responsible for building the **master problem**
used for benders decomposition.
The user can also define some additional constraints that link the investment candidates.
These constraints are added to the **master problem** at this stage.

## 3- Resolution stage: Benders decomposition
This stage actually solves the Antares-Xpansion problem composed by
the **master problem** and the **satellite problems**.
The [Benders decomposition](https://en.wikipedia.org/wiki/Benders_decomposition) is a well-suited algorithm for the resolution of two-stage problems
like the Antares-Xpansion problem. We will not discuss the details of the algorithm, 
some elements are given in the [user guide](../user-guide/0-introduction.md).


## 4- Update of antares study
The resolution stage provides the investment values for each candidate that minimizes the
global cost of both the **master problem** and the **satellite problems**. However, it does not allow
access to the details provided by an Antares-Simulator simulation.
In order to access these details, a "standard" Antares-Simulator simulation
is prepared by updating the original study with the values obtained for the resolution stage.
Each investment is associated to a link and its investment value is added to the link direct and indirect
transfer capacity.

## Antares-Xpansion package executables
Antares-Xpansion consists in 4 executables and a python orchestrator that is responsible for 
calling these executable with the correct options for each simulation step.


|Executable|Simulation step|
|-----|-----|
|`antares-solver`|Optimization problems generation |
|`lp_namer`|Modification of problems to introduce investment variables |  
|`benders`|Benders decomposition | 
|`xpansion-study-updater `|Update of antares study | 