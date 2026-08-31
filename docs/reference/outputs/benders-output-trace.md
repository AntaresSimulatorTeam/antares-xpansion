# Benders output trace

The Benders decomposition algorithm generates a CSV trace file to track
iteration progress and results. By default, this file is named
`benders_output_trace.csv` and is located in the output directory specified by
the [`OUTPUTROOT` option](../benders/options.md) (default: current directory).

The filename can be customized via the `CSV_NAME` option in the [`option.json` file](../benders/options.md).

## File structure

The CSV file uses semicolons (`;`) as delimiters and contains one header row
followed by data rows. Each iteration of the Benders algorithm produces:

- One **Master** row with information about the master problem
- Multiple **Subproblem** rows (one per subproblem) with information about each subproblem

## Columns

| Column                            | Master Row                                                  | Subproblem Row                                        | Description                                                             |
| --------------------------------- | ----------------------------------------------------------- | ----------------------------------------------------- | ----------------------------------------------------------------------- |
| `Ite`                             | Iteration number                                            | Iteration number                                      | Current iteration counter (1-indexed)                                   |
| `Worker`                          | `Master`                                                    | `Subproblem`                                          | Type of worker/row                                                      |
| `Problem`                         | Master problem name (from `MASTER_NAME`, default: `master`) | Subproblem name                                       | Identifier for the problem                                              |
| `Id`                              | Total number of subproblems                                 | Subproblem index                                      | For master: count of subproblems. For subproblem: its unique identifier |
| `UB`                              | Upper bound (`trace._ub`)                                   | Subproblem cost (`subproblem_data.subproblem_cost`)   | Upper bound of the master problem / Cost of the subproblem              |
| `LB`                              | Lower bound (`trace._lb`)                                   | -                                                     | Lower bound of the master problem                                       |
| `bestUB`                          | Best upper bound (`trace._best_ub`)                         | -                                                     | Best upper bound found so far                                           |
| `simplexiter`                     | -                                                           | Simplex iterations (`subproblem_data.simplex_iter`)   | Number of simplex iterations for the subproblem                         |
| `jump`                            | Cut distance (`norm_point(x_cut, trace.get_x_cut())`)       | -                                                     | Euclidean distance between current and previous cut points              |
| `single_subpb_costs_under_approx` | -                                                           | Approximation cost (`alpha_i`)                        | Subproblem cost under current approximation                             |
| `time`                            | Master duration (`trace._master_duration`)                  | Subproblem timer (`subproblem_data.subproblem_timer`) | Wall-clock time spent solving in seconds                                |
| `basis`                           | -                                                           | -                                                     | Reserved for basis information (currently unused)                       |


## Example

| Ite | Worker     | Problem   | Id   | UB     | LB    | bestUB  | simplexiter | jump | single_subpb_costs_under_approx | time | basis |
|-----|------------|-----------|------|--------|-------|---------|-------------|------|---------------------------------|------|-------|
| 1   | Master     | master    | 5    | 1000.5 | 950.2 | 1000.5  | 0.123       |      |                                 | 0.25 |       |
| 1   | Subproblem | sub-1     | 0    |        |       |         |             | 150  | 450.5                           | 0.1  |       |
| 1   | Subproblem | sub-2     | 1    |        |       |         |             | 200  | 350.2                           | 0.08 |       |
| 2   | Master     | master    | 5    | 980.1  | 975.3 | 980.1   | 0.087       |      |                                 | 0.30 |       |
| 2   | Subproblem | sub-1     | 0    |        |       |         |             | 120  | 445.2                           | 0.09 |       |
| 2   | Subproblem | sub-2     | 1    |        |       |         |             | 180  | 348.5                           | 0.07 |       |


## Notes

- The file is created at the start of the Benders run and appended to if [`RESUME` mode](../benders/options.md) is enabled
- Empty cells in the CSV represent values that are not applicable for that row type
- The `jump` column for master rows indicates how much the solution has changed between iterations
- The `single_subpb_costs_under_approx` column for subproblem rows shows the contribution of each subproblem to the overall approximation

