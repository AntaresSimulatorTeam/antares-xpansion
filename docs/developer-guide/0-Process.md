# antares-xpansion simulation procedure

There are 4 steps in a full antares-xpansion simulation.

## 1- Optimization problems generation
`antares-solver` is run using xpansion options. The weekly optimization problems (1st and 2nd optimization) are written as .mps files.
Some files are also produce so problems context (column id and variable map for exemple) is available for next step.

## 2- Modification of problems to introduce investment variables
Investment candidates are defined in `user/expansion/candidates.ini`, they will be used to change problem .mps file to introduce investment variables.

The master problem used for benders decomposition is also created. The constraint between investment are added to the master problem.

## 3- Benders decomposition
From the master problem and the satellite problem as .mps file, a benders decomposition is run to define the optimization of the investment.

## 4- Update of antares study
After benders decomposition, investment for each candidates is calculated.
For the link associated with the candidate, antares study link direct and indirect capacity is updated with calculated investment.

## antares-xpansion package executables
antares-xpansion consists in 4 executables and a python orchestrator which responsability is to call these executable with the correct options for each simulation step.


|Executable|Simulation step|
|-----|-----|
|`antares-solver`|Optimization problems generation |
|`lp_namer`|Modification of problems to introduce investment variables |  
|`benders`|Benders decomposition | 
|`xpansion-study-updater `|Update of antares study | 