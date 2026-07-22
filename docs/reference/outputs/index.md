# Outputs

When the Antares Xpansion algorithm terminates, i.e. when an optimal investment 
combination has been found, the package produces an archive `simulation-name.zip`
located in the output folder of the Antares study.

- [Results of the optimization](./optimization-output.md)
    - `out.json`: summary of the algorithm iterations
    - `last_iteration.json`: informations about the best and last iterations (subset of the `out.json`)
- [Other output files](./others.md) for logs, mathematical aspects and algorithm metrics
    - `lp/LOLD.txt`: lost of load duration at each iteration
    - `lp/UnsuppliedEnergy.txt`: unsupplied energy duration at each iteration
    - `reportbenders.txt`
    - `benders_solver.log`
    - `benders_output_trace.csv`: iteration progress of Benders algorithm
    - `master_last_iteration.mps` (or `.svf`): mathematical formulation of the master problem
    - `master_last_basis.bss`: last basis of Benders decomposition
