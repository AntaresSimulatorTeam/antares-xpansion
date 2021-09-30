# Modification of problems to introduce investment variables

## Python orchestrator `get_names` 
Before changing .mps file, a post processing of `antares-solver` created file must be done.


`lp_namer` executable is responsibl
Investment candidates are defined in `user/expansion/candidates.ini`, they will be used to change problem .mps file to introduce investment variables.

The master problem used for benders decomposition is also created. The constraint between investment are added to the master problem.

TODO : describe goal of getnames in python orchestrator

TODO : describe input/output