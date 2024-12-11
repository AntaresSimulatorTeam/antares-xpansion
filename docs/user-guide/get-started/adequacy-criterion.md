# Adequacy criterion

By default, Antares-Xpansion tries to solve an investment problem by minimising the sum of expected operational costs and investment cost. Nothing guarantees that the expected number of hours of loss of load is under a certain reliability threshold. 

However, Antares-Xpansion is able to enforce solutions to satisfy a maximum number of expected hours of loss of load per area. To use this feature, the user must:

1. Define the reliability constraints in an input file `user/expansion/adequacy_criterion/adequacy_criterion.yml`.
2. Launch the optimization with the `-m adequacy_criterion` flag.

The resolution of the reliability-constrained investment problem is based on a heuristic that is described in [Reliability-constrained investment problem](../optimization-principles/problem-formalization.md#reliability-constrained-investment-problem).

## Input data for adequacy criterion in `adequacy_criterion.yml`

The input file for the adequacy criterion should be located in `antares-study-folder/user/expansion/adequacy_criterion/adequacy_criterion.yml`. It is in YAML format and follows the following template:

```yaml
stopping_threshold: 1e4
criterion_count_threshold: 1
patterns:
  - area: "fr"
    criterion: 3
  - area: "de"
    criterion: 5
```

- `stopping_threshold` in euros (positive float): Stopping criterion of the algorithm, that is the difference between `lambda_min` and `lambda_max`, see [Reliability-constrained investment problem](../optimization-principles/problem-formalization.md#reliability-constrained-investment-problem).
    - Default value: `1e-4`
- `criterion_count_threshold` in MWh (positive float): Number of MWh of unsupplied energy in a given hour to consider that this is an hour with loss of load (ex: An hour is considered to be with loss of load if there is more than 1 MWh of unsupplied energy).
    - Default value: `1`
- `patterns`: A list giving the area names and the associated criterion.
    - required at least a pair of (criterion; area)
    - `area` (string): Name of the area
    - `criterion` in hours (positive float): Maximum expected number of hours with loss of load over all scenarios for the corresponding area

## Launching the optimization with the adequacy criterion

To launch the optimization with the adequacy criterion, simply use the option `-m adequacy_criterion` in the command line:
```
antares-xpansion-launcher.exe -i examples\SmallTestFiveCandidates -m adequacy_criterion
```

## Outputs

The final solution can be read in the file `output/<simulation-name>/expansion/out.json`, in the field `solution`.

Several log files are written:

- `reportbenders.txt` gives information on the progress of the algorithm with an operational perspective,
- `benders_solver.log` contains more detailed information on all data of interest to follow the progress of the algorithm (`lambda_min`, `lambda_max`, detailed solving times, ...).
- The file `LOLD.txt` under `lp/` dir stores adequacy criteria for all valid patterns (area+criterion)
- The file `PositiveUnsuppliedEnergy.txt` under `lp/` dir stores the amount of unsupplied energy for all valid patterns (area+criterion)
