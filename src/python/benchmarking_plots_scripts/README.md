# Benchmarking Plot Scripts

## Scripts

### `plot_benders_duration.py`
**Args:** `<study_path>`
Prints cumulative master/subproblem solve durations (in ms) per Benders iteration from `results_benders/out.json`. No plot — text output only.

### `plot_constraints_per_master.py`
**Args:** `<study_path>`
Stacked bar chart of added constraints per subproblem, **one PNG per master iteration**. Bars split by value=0 vs value=1 (is_N). Saved to `<study_path>/plots_per_master/`.

### `plot_micro_iterations.py`
**Args:** none (reads `micro_iterations_proc_0.log` from script directory)
Stacked bar chart of micro-iteration solve time per Benders iteration, **one PNG per subproblem**. Colors represent individual micro-iterations.

### `load_added_constraints.py`
**Args:** `<study_path>`
Bar chart of added constraints per subproblem for the **last master iteration** only. Bars split by value=0 vs value=1. Saves `added_constraints_histogram.png` in the study directory.

### `read_added_constraints.py`
**Args:** none (reads from `added_constraints/` directory next to the script)
Text output only — prints constraint count and a sample per subproblem from the last master/micro iteration. No plot.
