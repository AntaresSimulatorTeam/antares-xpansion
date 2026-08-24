# Other generated files

# Logs

- The file `LOLD.txt` under `lp/` dir stores the number of hours of loss of load for each area at each iteration of the algorithm.
- The file `UnsuppliedEnergy.txt` under `lp/` dir stores the amount of unsupplied energy for each area at each iteration of the algorithm.

# Benders related files

You can find more information about Benders [in the dedicated section](../benders/index.md).

- `reportbenders.txt` gives information on the progress of the algorithm with an operational perspective,
- `benders_solver.log` is the mathematical log of the Benders solver. 
    It contains more detailed information on all data of interest to follow the progress
    of the algorithm (`lambda_min`, `lambda_max`, detailed solving times, ...).
- `benders_output_trace.csv` for iteration progress information
    (more on [this](./benders-output-trace.md) page)

# Master problem formulation

- `master_last_iteration.mps` (or `.svf` depending on the solver) to store the cuts at the end of a Benders run
- `master_last_basis.bss`: Basis of the last master

Both files are useful (used as inputs) for the [sensitivty analysis](../inputs/sensitivity.md).
