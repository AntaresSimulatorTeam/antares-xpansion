# antares-xpansion simulation procedure

There are 4 steps in a full antares-xpansion simulation :

1- antares-solver :

antares-solver is run using xpansion options. The weekly optimization problems (1st and 2nd optimization) are written as .mps files.
Some files are also produce so problems context (column id and variable map for exemple) is available for next step.

2- update problem to introduce investment variable :

From a list of .mps file created by antares-solver and the associated variables, lp_namer change the problem to introduction investment variables.

The master.mps used by benders is also created.

3- Benders optimization

4- antares study update

antares-xpansion consists in 4 executables and a python orchestrator which responsability is to call these executable with the correct options.
