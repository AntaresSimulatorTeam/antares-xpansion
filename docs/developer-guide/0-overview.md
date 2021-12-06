# Antares-Xpansion simulation procedure

An Antares-Xpansion simulation is currently run in four separate steps, managed by a Python orchestrator.

The basic idea behind an Antares-Xpansion simulation is to let Antares-Simulator
read the Antares study and build the initial supply-demand optimisation problem without knowledge of the 
investment candidates. Then the investment optimization problem is built and solved within Antares-Xpansion.

There are four steps in a full Antares-Xpansion simulation. Each section title is followed by the corresponding value of the variable `--step` when launching `antares-xpansion-launcher` from the command line.

## 1- Antares-Simulator optimization problems retrieval: `antares`

The solver of Antares-Simulator, `antares-solver`, runs the Antares study with a specific configuration set by Antares-Xpansion, see [Antares optimization problem retrieval](1-antares-solver-problem.md) for more details. 

The weekly supply-demand optimization problems are written as `.mps` files. In this mode Antares-Simulator writes some additional files that add a business context of the linear problems (`variable.txt`, `area.txt` and `interco.txt`). These files allow to map the variable numbers in the `.mps` files to their physical meaning. For more details, see [Antares optimization problem retrieval](1-antares-solver-problem.md).

## 2- Investment problem generation: `problem_generation`

This stage is responsible for building the investment optimization problem, using the [Benders reformulation](../../user-guide/optimization-principles/1-problem-formalization.md), which results in the creation of a **master problem** and of **satellite problems**. The **master problem** takes investment decisions whereas **satelitte problems** solve the Antares supply-demand problems once the investment decisions are taken.

Using the data on investment candidates specified in the `user/expansion/candidates.ini` file,
Antares-Xpansion modifies the `.mps` problems provided by
Antares-Simulator in the previous step to add information on the candidates. These modified problems, still written
 as `.mps` files, are the so-called **satellite problems**. They correspond to the supply-demand problem to be solved once the investment has been fixed to a given value (that changes during the Antares-Xpansion optimization).

This stage is also responsible for building the intial **master problem**
used for Benders decomposition. If there are additional constraints linking the investment candidates (defined in the `additional-constraints` field of the `user/expansion/settings.ini` file), then they are added to the **master problem**.

For more details on how the master and the satellite problems are generated, see [Investment problem generation](2-problem-modification.md).

## 3- Resolution stage with Benders decomposition: `benders`

The core step of Antares-Xpansion problem consists in solving the investment problem generated in the previous step.
Antares-Xpansion uses a [Benders decomposition](https://en.wikipedia.org/wiki/Benders_decomposition) algorithm as it is well-suited to the structure of the investment problem: the **satellite problems** are independent, i.e. the weekly problems in an Antares study are independent (using water values is not allowed in Antares-Xpansion). Details of the methodology are given in the [optimization principles](../user-guide/optimization-principles/0-optimization-principles.md) page.


## 4- Update of antares study: `study_update`

The resolution stage provides, for each candidate, the investment that minimizes the
overall cost of the investment problem. However, it does not allow
access the details provided by an Antares-Simulator simulation.
In order to access these details, a "standard" Antares-Simulator simulation
is prepared by updating the original Antares study with the values obtained from the resolution stage.
Each investment is associated to a link and its investment value is added to the link direct and indirect
transfer capacity.

## Antares-Xpansion package executables

Antares-Xpansion consists in four executables and a Python orchestrator that is responsible for 
calling these executables with the correct options for each step.


|Executable|Simulation step|
|-----|-----|
|`antares-x.x-solver`|Antares-Simulator optimization problems retrieval, where `x.x` stands for the version number of Antares-Simulator, for example `antares-8.1-solver`. |
|`lp_namer`|Investment problem generation. |  
|`benderssequential` or `bendersmpi`|Benders decomposition, either sequential or with MPI. | 
|`xpansion-study-updater `|Update of antares study. | 