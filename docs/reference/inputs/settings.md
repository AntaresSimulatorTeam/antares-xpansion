# Settings 

In order to solve the investment problem defined by an Antares study 
and its associated `candidates.ini` file, Antares Xpansion uses the 
Benders decomposition algorithm. The simulation options and algorithmic parameters 
are defined in the `settings.ini` file, located in the folder `user/expansion/`
of the Antares study.

!!! info 

    For more info about the signification of the parameters, visit the 
    user documentation page for 
    [Xpansion settings](https://antares-doc.readthedocs.io/en/latest/reference/xpansion-settings/).

The following section lists the configurable parameters.
If the user does not specify the value of a parameter, its default value is used.

| Name | Default value | Description |
| -----| -------------| -------------|
|`optimality_gap` | `1` | Tolerance on absolute gap |
|`relative_gap` | `1e-6` | Tolerance on relative gap |
|`max_iteration` | `+Inf` | Maximum number of Benders iterations |
|`timelimit` | `1e12` | Timelimit (in seconds) of the Benders step |
|`uc_type` | `expansion_fast` | Unit-commitment type used by Antares |
|`master` | `integer` | Resolution mode of the master problem |
|`yearly-weights` | `None` | Path of the Monte-Carlo weights file |
|`solver` | `Cbc` | Name of the solver |
|`log_level` | `0` | Logs severity |
|`additional-constraints` | `None` | Path of the additional constraints file |
|`separation_parameter` | `0.5` | Step size for the in-out separation |
|`relaxed_optimality_gap` | `1e-5` | Threshold to switch from relaxed to integer master |
|`batch_size` | `0` | Number of subproblems per batch |
|`master_solution_tolerance` | `1e-4` | Tolerance for rounding master solution variables |
|`cut_coefficient_tolerance` | `5e-3` | Tolerance for rounding cut coefficients and rhs |

The format is a standard `.ini` and should follow this template:

```ini title="Example `settings.ini`"
uc_type = expansion_fast
master = integer
optimality_gap = 0
max_iteration = 100
timelimit = 300
additional-constraints = constraint.txt
log_level = 0
```
