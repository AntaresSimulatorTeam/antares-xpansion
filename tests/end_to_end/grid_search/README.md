# BendersEndToEnd tests description

## Context

This is a test for the inner optimization part of AntaresXpansion. Those tests focus on the  C++ executable grid_search.

**Input** : 

* studies.json : file containing the path to the scenarios to be tested.
* out.json : file containing the points that benders used with their investment cost, operational cost and the overall cost.
* grid.csv : file generated using the out.json containing the points used by benders.
* stucture.txt : file containing the indices of the variables in the subproblems.

**Output** : output.json, the file containing the costs of the points used by benders calculated by the grid_search executable.

## Description

The file *studies.json* contains every information to run the test. 
Each section is a test containing at least :
1. "path" : A path to the instance data, where the program will be launched

## Launching the test

Python module *pytest* should be installed. The test can be run with the following command, from the gridSearchEndToEnd folder :
    python3 -m pytest --intallDir=*path_to_executables_folder*

## Test markers

All the tests present in this test have the common marker **optim**.

To run some specific tests, markers can be used as follows :
    python3 -m pytest --intallDir=*path_to_executables_folder* -m *wanted_marker*