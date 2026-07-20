# Logs

Several log files are written:

- `reportbenders.txt` gives information on the progress of the algorithm with an operational perspective,
- `benders_solver.log` is the mathematical log of the Benders solver. It contains more detailed information on all data of interest to follow the progress of the algorithm (`lambda_min`, `lambda_max`, detailed solving times, ...).
- The file `LOLD.txt` under `lp/` dir stores the number of hours of loss of load for each area at each iteration of the algorithm.
- The file `UnsuppliedEnergy.txt` under `lp/` dir stores the amount of unsupplied energy for each area at each iteration of the algorithm.