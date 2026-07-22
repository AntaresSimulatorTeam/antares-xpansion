# Adequacy criterion

The input file for the adequacy criterion should be located in 
`antares-study-folder/user/expansion/adequacy_criterion/adequacy_criterion.yml`.

It is characterized by the following parameters:

- `stopping_threshold` in euros (positive float): Stopping criterion of the algorithm 
    (see [Reliability-constrained investment problem](https://antares-doc.readthedocs.io/en/latest/reference/xpansion-theory/#reliability-constrained-investment-problem)).
    - Default value: `1e-4`
- `criterion_count_threshold` in MWh (positive float): Number of MWh of unsupplied energy
    in a given hour to consider that this is an hour with loss of load 
    (ex: An hour is considered to be with loss of load if there is more than 1 MWh of unsupplied energy).
    - Default value: `1`
- `patterns`: A list giving the area names and the associated criterion.
    - required at least a pair of (criterion; area)
    - `area` (string): Name of the area
    - `criterion` in hours (positive float): Maximum expected number of hours with loss of load over all scenarios for the corresponding area

It is in YAML format and follows the following template:

```yaml
stopping_threshold: 1e4
criterion_count_threshold: 1
patterns:
  - area: "fr"
    criterion: 3
  - area: "de"
    criterion: 5
```

