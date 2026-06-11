# Optimization output

The final solution can be read in the file `output/<simulation-name>/expansion/out.json`,
in the field `solution`.

For each iteration:

1. The investment combination that has been evaluated,
2. The operational, investment and overall costs,
3. The current lower and upper bounds (no upper bound for the Benders by batch),
4. The absolute and relative gap,
5. The resolution time for the subproblems and the master problem.

There is also information on the iteration number which has led to the best solution.
The file `out.json` also gives the parameters that are used by the optimization 
algorithm (some of them are defined by the user in `settings.ini`).


