# Micro Iterations

## Motivation

In Antares-Xpansion, the investment problem is solved by Benders decomposition:
at each iteration, the master problem proposes a candidate investment plan, and
its operational cost is evaluated by solving the subproblems, each of which
corresponds to the operation of the Antares study over a given time frame —
typically one week — at hourly resolution.

Representing the system at a 1-hour time step means that every operational
constraint is instantiated for each of the 168 hours of the week:
supply–demand balance in every area, flow bounds on every interconnection,
technical limits of thermal clusters, hydraulic storage management, as well as
user-defined binding constraints. The size of the resulting linear programs
grows accordingly, and for large interconnected studies each subproblem becomes
a heavy optimization problem in its own right, with a resolution time that is
far from negligible.

Since a classical Benders iteration requires solving *all* subproblems before
the master problem can be enriched with new cuts, the subproblem phase
concentrates the vast majority of the computational effort, and the cost of a
single full iteration becomes the limiting factor of the whole resolution. The
*micro-iterations* paradigm was introduced to attack precisely this bottleneck.


## Core concept

The micro-iterations paradigm applies a constraint-generation scheme at the
subproblem level. Instead of instantiating the full set of operational
constraints from the outset, each subproblem is initially built with only a
subset of them, yielding a relaxed problem that is considerably smaller and
faster to solve.

This relaxation is solved a first time, and the resulting solution is handed to
a verification mechanism whose role is to check, outside of the optimization
itself, whether the constraints that were left out of the model are satisfied
by the current solution. The constraints found to be violated are injected into
the subproblem, which is then re-solved with this enriched formulation.

This solve–check–inject sequence constitutes one *micro-iteration*, and it is
repeated until the verification step detects no further violation. At that
point, the solution of the relaxed problem is feasible for the complete
formulation — and therefore optimal for it, since removing constraints can only
relax the problem.

The subproblem thus returns exactly the same optimal value and dual information
as if it had been solved with all constraints from the start, but the solver
only ever handles the constraints that actually matter: in practice, a large
share of the operational constraints are inactive at the optimum, and the
successive relaxations remain far smaller than the full problem.

![Micro Iterations](../../assets/media/micro_iterations.png)


## Launching a study with micro-iterations

The micro-iterations mode is enabled through the simulation settings by setting
the variable `MICRO_ITERATIONS: true`. When this mode is active, the study must
be enriched with additional inputs that describe the constraints excluded from
the initial subproblems and provide the machinery needed to detect their
violation. Compared to a standard study, the following elements must be added:

- **`MICRO_ITERATIONS: true`** — the simulation variable that activates the
  micro-iterations mode. Without it, the study is solved with the classical
  scheme in which each subproblem carries its full set of constraints from the
  start.

- **MPS/SVF files for the removed constraints** — in addition to the MPS/SVF
  files representing the initial (relaxed) subproblem optimization problems,
  the study must provide MPS/SVF files describing the constraints that were
  removed from the initial formulation. These files are the pool from which
  violated constraints are injected into the subproblems at each
  micro-iteration.

- **A `plugin_inputs` folder** — this folder contains the artefacts needed to
  determine, from a subproblem solution, which of the removed constraints are
  violated. In particular, it can contain an external library that is
  dynamically linked at runtime to evaluate the violated constraints from the
  current subproblem solution.

- **A `variables_to_follow.txt` file** — this file lists the ids of the
  variables whose values must be extracted from the subproblem solution in
  order to evaluate the removed constraints and detect violations.