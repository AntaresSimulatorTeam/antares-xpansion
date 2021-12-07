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

The power transmission which transits in the link at a given time step $t$ is denoted by $p_t$. If the maximum power going in the forward direction (resp. backward) is given by $P_{t,max}^{+}$ (resp. $P_{t,max}^{-}$), the original problem bounds for $p_t$ (`ValeurDeNTCOrigineVersExtremite`) are
$$
- P_{t,max}^{-} \leq p_t \leq P_{t,max}^{+}
$$
If the hurdle costs for the link are activated, there are two extra variables $p_{t}^{+}$ (`CoutOrigineVersExtremiteDeLInterconnexion`) and $p_{t}^{-}$ (`CoutExtremiteVersOrigineDeLInterconnexion`) with the following bounds:
$$
0 \leq p_{t}^{+} \leq P_{t,max}^{+}
$$

$$
0 \leq p_{t}^{-} \leq P_{t,max}^{-}
$$
and the additional constraints:
$$
p_{t} = p_{t}^{+} - p_{t}^{-}
$$

The following modifications are applied to each `.mps` file in order to have the following system:
$$
-\infty \leq p_t \leq +\infty
$$
If the hurdle costs for the link are activated, there are two extra variables $p_{t}^{+}$ (`CoutOrigineVersExtremiteDeLInterconnexion`) and $p_{t}^{-}$ (`CoutExtremiteVersOrigineDeLInterconnexion`) with the following bounds:
$$
0 \leq p_{t}^{+} \leq +\infty
$$

$$
0 \leq p_{t}^{-} \leq +\infty
$$
and the additional constraints:
$$
p_{t} = p_{t}^{+} - p_{t}^{-}
$$
We need to add the new investment variables $x$ having a direct (resp. indirect) temporal profile $X_t^{+}$ (resp. $X_t^{-}$) on a link with the already installed capacity $\bar{P}_{t,max}^{+}$
$$
p_t + X_t^{-} \cdot x \geq \bar{P}_{t,max}^{-}\\
p_t - X_t^{+} \cdot x \leq \bar{P}_{t,max}^{+}
$$

$$
p_{t}^{+} - X_t^{+} \cdot x \leq \bar{P}_{t,max}^{+}
$$

$$
p_{t}^{-} - X_t^{-} \cdot x \leq \bar{P}_{t,max}^{-}
$$

This translates into the following steps:

- Remove bounds for `ValeurDeNTCOrigineVersExtremite` variables (only for link with candidate),
- Change upper bounds to `Inf` for `CoutOrigineVersExtremiteDeLInterconnexion`  and `CoutExtremiteVersOrigineDeLInterconnexion` variables (only for link with candidate),
- Add new columns for each candidate,
- Add constraint on `ValeurDeNTCOrigineVersExtremite`,
- Add constraint on `CoutOrigineVersExtremiteDeLInterconnexion`,
- Add constraint on `CoutExtremiteVersOrigineDeLInterconnexion`.

### 3- Read additional candidate constraints

The `additional-constraints` parameter, that specifies the path to file, may be defined in the `settings.ini`. This is used to define linear constraints between the invested capacities of investment candidates, and is read at this stage of Antares-Xpansion process. For more information on the file format, see the [corresponding part](../../user-guide/get-started/3-settings-definition.md#add-constr) of the user guide.

### 4- Creation of master problem

The _master problem_ is created from the list of candidates in the following way:

- Add a new column (= variable) for each candidate with lower bound `0` and upper bound `max-investment` or `max-units` \\(\small\times\\) `unit-size`.
- If the investment on the candidate has integer constraints (use of the `unit-size` parameter): 
    - Add a new (integer) column for the number of built units, with lower bound `0` and upper bound `max-units`.
    - Add a constraint that links the number of units to the invested capacity.
    
- If additional constraints are defined:
    - Create binary constraints, that represent for example exclusion constraints, see [**Figure 13**](../../user-guide/get-started/3-settings-definition.md#add-constr).
    - Create the other linear constraints between invested capactities of the candidates.
    
### 5- Creation of a variable / column ID mapping file

A file (`structure.txt`) is created, containing for each `.mps` file, the associated column ID for the candidates.

!!! Example
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

    Here:
    
    - In `master.mps`, the candidate `battery` has column ID `0`,
    - In `problem-1-1-20210713-163528`, the candidate `peak` has column ID `5545`.

!!! Note
    The column ID is no longer used by `benders`. This file must still be created so that `benders` knows the list of candidate for each problem.
