# Sensitivity analysis inputs

In order to run the sensitivity analysis, the user must create a JSON file named `sensitivity_in.json` that is stored in the `user/expansion/senstivity` directory of the Antares study.

```
antares-study
└── input
└── layers
└── logs
└── output
└── settings
└── user
    └── expansion
        └── candidates.ini
        └── settings.ini
        └── ...
        └── sensitivity
            └── sensitivity_in.json
```

The file `sensitivity_in.json` contains 3 fields:

- `epsilon` (float) : Defines the maximum gap with the optimal solution that is allowed.
- `capex` (bool) : If `true`, the CAPEX sensitivity problems are solved (minimization and maximization), otherwise they are not solved.
- `projection` (list of strings) : List of candidate names for which the projection of the set of $\varepsilon$-optimal solutions is computed. For each candidate, two problems are solved: minimization and maximization of the invested capacity.

An sample `sensitivity_in.json` is given below:
```json
{
    "epsilon" : 1e4,
    "capex": false,
    "projection": ["peak", "semibase"]
}
```
   