## Outputs

The final solution can be read in the file `output/<simulation-name>/expansion/out.json`, in the field `solution`.

Several log files are written:

- `reportbenders.txt` gives information on the progress of the algorithm with an operational perspective,
- `benders_solver.log` contains more detailed information on all data of interest to follow the progress of the algorithm (`lambda_min`, `lambda_max`, detailed solving times, ...).
- The file `LOLD.txt` under `lp/` dir stores adequacy criteria for all valid patterns (area+criterion)
- The file `UnsuppliedEnergy.txt` under `lp/` dir stores the amount of unsupplied energy for all valid patterns (area+criterion
