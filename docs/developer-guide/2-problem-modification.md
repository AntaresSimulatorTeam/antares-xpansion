# Investment problem generation

The goal of this step is to modify the `.mps` files of the weekly Antares problems to create the so-called _satellite problems_ of the investment problem. The _master problem_ is also generated in this step. 

## Python orchestrator `get_names` 

Before changing the `.mps` file, a post-processing of the files created by `antares-solver` must be done.

The goal is to produce a file `mps.txt` containing each `.mps` with his associated `variables.txt` file (defining the mapping between the column IDs and variable names). Each line of the `mps.txt` file looks as follows:

```
problem-1-1-20210713-163528.mps variables-1-1-20210713-163528.txt constraints-1-1-20210713-163528.txt
```

!!! Note
    `constraints.txt` files are still defined in the file but not used in later steps.

Then:
    
- The `area-<mc_year>-<week>-<timestamp>.txt` file produced by `antares-solver` is copied as `area.txt`. 
- The `interco-<mc_year>-<week>-<timestamp>.txt` file produced by `antares-solver` is also copied as `interco.txt`.

## Modification of weekly problems with `lp_namer`

Here are the main steps of problem modification with `lp_namer`, that lead to the creation of the _satellite problems_.

### 1- Read candidates and get link column ID

Each investment link corresponds to a variable in the weekly optimization problem. The first step is to retrieve the column ID of each investment candidate link from its name specified in `candidates.ini`:

- The `area.txt` and `interco.txt` files are used to map area names to link IDs.
- Candidates defined in `candidates.ini` and the associated link profile (if defined) are read.

### 2- Modification of .mps file

The following modifications are applied to each `.mps` file:

- Remove bounds for `ValeurDeNTCOrigineVersExtremite` variables (only for link with candidate),
- Change upper bounds to `Inf` for `CoutOrigineVersExtremiteDeLInterconnexion`  and `CoutExtremiteVersOrigineDeLInterconnexion` variables (only for link with candidate),
- Add new columns for each candidate,
- Add constraint on `ValeurDeNTCOrigineVersExtremite`, (laquelle ?)
- Add constraint on `CoutOrigineVersExtremiteDeLInterconnexion`, (laquelle ?)
- Add constraint on `CoutExtremiteVersOrigineDeLInterconnexion`, (laquelle ?)

### 3- Read additional candidate constrains
Antares-Xpansion setting file can contains `additional-constraints` parameter.

In this case the file is used to define additional constraints for each candidate.
For more information on file format see corresponding user guide (TODO : add reference ?)

### 4- Creation of master problem
The master problem is created with the list of candidate.

- add new columns for each candidate with lower bound 0 and upper bound max. investment
- if the candidate represent integer constraint (use of `unit-size` parameter) : 
  * add new column for unit size
  * add constraint with maximum unit size
    
- if additional constraints are defined ;
    * create binary constraints
    * create other constraints
    
### 5- Creation of a variable map file
A file (`structure.txt`) is created, containing for each .mps file the associated column id for the candidates:
```
                      master  battery                   0
                      master  peak                      1
                      master  pv                        2
                      master  semibase                  3
                      master  transmission_line         4
 problem-1-1-20210713-163528  battery                5547
 problem-1-1-20210713-163528  peak                   5545
 problem-1-1-20210713-163528  pv                     5548
 problem-1-1-20210713-163528  semibase               5546
 problem-1-1-20210713-163528  transmission_line      5544
```
For this example:

`master.mps` candidate `battery` has column ID `0`
`problem-1-1-20210713-163528`candidate `peak` has column ID `5545`.

!!! Note
    It seems that the column ID is no longer used by benders.
    This file must still be created so benders can know the list of candidate for each problem.
