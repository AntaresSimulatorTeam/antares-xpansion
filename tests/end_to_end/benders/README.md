# BendersEndToEnd tests description

## Context

This is a test for the inner optimization part of AntaresXpansion. Those tests focus on three C++ executables :

1. benderssequential
2. bendersmpi
3. merge_mps

**Input** : Files created by *lpnamer* executable of the project, or any group of files following the desired structure :

* A MPS file for the master problem (default name : master.mps)
* A group of MPS files for the subproblems
* a file *structure.txt* containing the column index of each coupling variable in each problem

**Output** : At least a file *out.json*, containing a section "solution", where the data to be tested is written.

## Description

The file *resultTest.json* contains every information to run the test. 
Each section is a test containing at least :
1. "path" : A path to the instance data, where the program will be launched
2. "option_file" : A name of the option file to use 

### mini_network_default

Tests the resolution of a mini network design instance with default options. 

### mini_network_slave_weight

Tests the resolution of a mini network design instance with a file giving weights for subproblems. 

### mini_instance_LP_default

Tests the resolution of a mini LP problem with default options.

### mini_instance_MIP_default

Tests the resolution of a mini MIP problem with default options. Optimal solution differs from its LP relaxation.

### mini_instance_UNBOUNDED

Tests the resolution of an unbounded instance.

### mini_instance_INFEASIBLE

Tests the resolution of an infeasible instance.

## Launching the test

Python module *pytest* should be installed. The test can be run with the following command, from the bendersEndToEnd folder :
    python3 -m pytest --intallDir=*path_to_executables_folder*

## Test markers

All the tests present in this test have the common marker **optim**.
There are also markers to launch only tests on specific optimization executables :

* benderssequential : launching only tests with benderssequential executable
* bendersmpi : launching only tests with bendersmpi executable
* mergemps : launching only tests with merge_mps executable

To run some specific tests, markers can be used as follows :
    python3 -m pytest --intallDir=*path_to_executables_folder* -m *wanted_marker*