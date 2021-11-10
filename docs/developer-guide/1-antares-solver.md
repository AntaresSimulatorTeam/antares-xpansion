# Optimization problems generation
`antares-solver` is run using *xpansion* options. The weekly optimization problems (1st and 2nd optimization) are written as `.mps` files.

## Specific Antares-Xpansion configuration
In order to run the `antares-solver` study correctly, some 
specific simulation options are required.

`settings/generaldata.ini` file must be changed :

```ini
[optimization]
include-exportmps = true
include-exportstructure = true
include-tc-minstablepower = false
include-tc-min-ud-time = false
include-dayahead = false
[general]
mode = Economy
[other preferences]
unit-commitment-mode = fast
```

|Parameters|Remark|
|-----|-----|
|`[optimization]include-exportmps`| Ask for export of .mps | 
|`[optimization]include-exportstructure`| Ask for export of structure file : "variables.txt, area.txt, ..."|
|`[optimization]include-tc-minstablepower`| TODO| 
|`[optimization]include-tc-min-ud-timer`| TODO| 
|`[optimization]include-dayahead`| TODO| 
|`[general]mode`| TODO| 
|`[other preferences]unit-commitment-mode`| TODO| 

>Note :
>The modifications of these parameters is not done by user but by the python orchestrator.

## Output files

### .mps files for the weekly optimization problems

File name formatting: `problem-<mc_year>-<week>-<timestamp>.mps`.

For example `problem-1-1-20210928-155703.mps` represents the weekly problem for MC year 1 and week 1 generated in 2021/09/28 at 15h5703s.

### variables.txt file for each .mps file

File name formatting :`variables-<mc_year>-<week>-<timestamp>.txt`

For example `variables-1-1-20210928-155703.txt` represents the variables name of problem for MC year 1 and week 1 generated in 2021/09/28 at 15h57 03s.

The file contains one line for each column in the weekly problem.

Line content depends on the type of the column (if column related or not to a country).

Example for `462 ValeurDeNTCOrigineVersExtremite 0 0 14`

|Value|Parameter|
|-----|-----|
|`462`| column ID in weekly problem | 
|`ValeurDeNTCOrigineVersExtremite`| column name|
|`0`| country| 
|`0`| link ID| 
|`14`| time step| 

### area.txt

Only one file describing the areas is produced by `antares-solver`.

File name formatting: `area-<mc_year>-<week>-<timestamp>.txt`.

Because area doesn't change in each MC-year or week, the file is created only for the first MC-year and week simulated.

The file contains one line for each area used in simulation.

Example :
```
area1
area2
flex
peak
pv
semibase
```

### interco.txt

Only one file describing the link is produced by `antares-solver`. 

File name formatting: `interco-<mc_year>-<week>-<timestamp>.txt`.

Because links doesn't change in each MC-year or week, the file is created only for the first MC-year and week simulated.

The file contains one line for each link used in simulation.

Example :
``` 
3 1 2
link_id : 3 
index_pays_origin : 1
index_pays_extremite : 2
```

The index is a match to the index in the `area.txt` file. So for this example this means that link_id `3` match to link `area2-flex`.
