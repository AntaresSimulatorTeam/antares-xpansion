# Outputs

When the Antares Xpansion algorithm terminates, i.e. when an optimal investment 
combination has been found, the package produces an archive `simulation-name.zip`
located in the output folder of the Antares study.

Once you unzip the archive, the files `lp/reportbenders.txt` and the 
`expansion/out.json` log information for each iteration:

1. The investment combination that has been evaluated,
2. The operational, investment and overall costs,
3. The current lower and upper bounds (no upper bound for the Benders by batch),
4. The absolute and relative gap,
5. The resolution time for the subproblems and the master problem.

There is also information on the iteration number which has led to the best solution.
The file `out.json` also gives the parameters that are used by the optimization 
algorithm (some of them are defined by the user in `settings.ini`).