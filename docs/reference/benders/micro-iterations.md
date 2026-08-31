# Micro Iterations


## Core concept

The micro-iterations paradigm applies a constraint-generation scheme (lazy-constraint) at the subproblem level. Instead of instantiating the full set of operational
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

## Warm start

Rather than rebuilding each subproblem from its minimal relaxed formulation and re-discovering the relevant constraints from scratch, the warm-start mechanism can optionally initialize the subproblem with the constraints injected during the previous iteration's solve–check–inject sequence. These constraints were active (binding or violated) for the previous master solution, and when successive master solutions remain close to one another, a meaningful share of them are still needed, so starting from this enriched formulation can require fewer micro-iterations to reach feasibility.

This behavior is available as an option, but whether it actually helps is use-case dependent and left to the user's judgment. The active-constraint set can vary significantly between successive Benders iterations, in which case the carried-over constraints add little value and mostly inflate the initial problem for no benefit. There is also a compounding risk: as constraints accumulate across iterations without being pruned, the subproblem can drift back toward containing its full constraint set, which produces the same large, heavy-to-solve formulations the micro-iteration scheme was designed to avoid — negating the paradigm's benefits. Whether to enable warm-starting, and whether some form of constraint pruning is needed alongside it, is therefore a trade-off the user should assess for their specific problem structure.

## Launching a study with micro-iterations

The micro-iterations mode is enabled through the simulation settings by setting
`MICRO_ITERATIONS: true`. When this mode is active, the study must
be enriched with additional inputs that describe the constraints excluded from
the initial subproblems and provide the machinery needed to detect their
violation. Compared to a standard study, the following elements must be added:

- **`MICRO_ITERATIONS: true`**  in the `options.json` file, see [Settings of benders executable](./options.md) — the simulation variable that activates the
  micro-iterations mode. Without it, the study is solved with the classical
  scheme in which each subproblem carries its full set of constraints from the
  start.

- **MPS/SVF files for the removed constraints** — in addition to the MPS/SVF
  files representing the initial (relaxed) subproblem optimization problems,
  the study must provide MPS/SVF files describing the constraints that were
  removed from the initial formulation. These files are the pool from which
  violated constraints are injected into the subproblems at each
  micro-iteration.

    To make the role of these files concrete, consider a subproblem with
    objective $c$ and three constraints $a_1$, $a_2$ and $a_3$. With the
    classical scheme, the subproblem is written as a single optimization
    problem:

    $$
    \begin{aligned}
        \min_{x} \quad & c^\top x \\
        \text{s.t.} \quad & a_1^\top x=b_1\\
        & a_2^\top x = b_2\\
        & a_3^\top x = b_3
    \end{aligned}
    $$

    In micro-iterations mode, this problem is split into the initial relaxed
    subproblem — which only carries $a_1$ — and the constraints removed from
    it, $a_2$ and $a_3$, described in separate MPS/SVF files:

    $$
    \underbrace{
    \begin{aligned}
        \min_{x} \quad & c^\top x \\
        \text{s.t.} \quad & a_1^\top x = b_1
    \end{aligned}
    }_{\text{initial minimal subproblem}}
    \qquad\qquad\Big|\qquad\qquad
    \underbrace{
    \begin{aligned}
        & a_2^\top x =b_2 \\ 
        & a_3^\top x = b_3
    \end{aligned}
    }_{\text{constraints file with null objective function}}
    $$


    The files on the left represent the initial subproblem, while the files on
    the right are the "removed constraints" files required here: they are not
    part of the initial relaxation, but they form the pool from which the
    verification mechanism selects the violated constraints to re-inject into
    the relaxed subproblem at each micro-iteration. Solving the left-hand
    problem enriched with every constraint from the right-hand side is
    equivalent to solving the original complete problem.

- **A `plugin_inputs` folder** — this folder contains the artefacts needed to
  determine, from a subproblem solution, which of the removed constraints are
  violated. In particular, it can contain an external library that is
  dynamically linked at runtime to evaluate the violated constraints from the
  current subproblem solution.

- **A `variables_to_follow.txt` file** — this file lists the ids of the
  variables whose values must be extracted from the subproblem solution in
  order to evaluate the removed constraints and detect violations.