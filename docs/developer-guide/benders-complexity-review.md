# Benders module: complexity review & refactoring plan

Status: **P1–P3 implemented and validated** (build + unit + e2e green) — branch `refactor/benders-complexity` (based on `origin/develop` @ `ccbab405b`)

## 1. Context

Two earlier branches (`feature/refacto_outer`, `feature/refacto_outer_5`) started improving the
Benders design with an adapter pattern, but were forked from an old develop
(`38862e770`) and never merged. Since then, develop received significant Benders development:

- Micro-iterations 2 & 3 (`#1239`, `#1252`): plugin interface, `SubproblemWorkerFactory`,
  `SubproblemBasisCache`, warm-start / basis handling, ~+600 lines in `Benders_MICRO_ITERS`
- Skeleton subproblem mode (`#1267`): `SkeletonCoefficientSet`, `SkeletonConstraintSetLoader`,
  `SkeletonSolverLoader`, `SolverRowExtractor`
- `ICommunicationStrategy` applied to `OuterLoop` (`#1223`)
- `OuterLoopBenders` moved to `outer_loop/`, `BendersByBatch` growth, new unit tests

Net effect: **+2624/−398 lines** in `src/cpp/benders` since the fork point, mostly in
`BendersBase.cpp` (now **1604 lines**) and the micro-iterations plugin. The god-class problem
the old branches tried to address has grown, not shrunk. This document restarts the effort from
the current develop.

## 2. Current architecture

### 2.1 CMake target graph

```
factories ──► benders_sequential_core ──► benders_core ◄──► outer_loop_lib   (CYCLE)
     │                 benders_mpi_core ──┘        ▲  ▲
     │                 benders_by_batch_core       │  │
     └──► outer_loop_lib ──────────────────────────┘  │
        plugins ◄─────────────────────────────────────┘   (CYCLE, via BendersBase.h)
        logger, output, merge_mps, merge_master_mps
```

Two target-level cycles exist:

| Cycle | Cause |
|---|---|
| `benders_core` ↔ `outer_loop_lib` | `benders_core/MasterUpdate.h` includes `outer_loop/IMasterUpdate.h`; both link each other PUBLIC |
| `benders_core` ↔ `plugins` | `BendersBase.h` includes `plugins/BendersPlugin.h`; both link each other PUBLIC |

### 2.2 Class map

```
BendersApp (factories)          BendersFactory, BendersPluginFactory, logger/writer factories
   │
   ├─ plain:      benders->launch()
   └─ outer loop: MasterUpdateBase(benders, τ, threshold)   ← lives in benders_core!
                   CutsManagerRunTime                       ← dead, never used after injection
                   OuterLoopBenders(criteria, updater, cutsManager, benders, strategy)

OuterLoop (template method, abstract)
   └── OuterLoopBenders  ── holds pBendersBase + IMasterUpdate + ICutsManager + OuterLoopBiLevel
         delegates ~20 calls to benders_ (incl. benders_->_logger, public data member access)

BendersBase (1604 lines, ~108 methods, 9+ responsibilities)
   ├── lifecycle:        launch / Run / free / resume mode / timers
   ├── master LP proxy:  ~15 Master* methods (AddRows, GetRhs, Variables, Objective…) → _master
   ├── subproblems:      worker factory, map, 3 cut-gathering paths, basis save/restore
   ├── cuts:             compute_cut / aggregate / build_all_aggregated_cuts / tolerances
   ├── criteria:         CriterionComputation, per-iteration criteria vectors
   ├── output:           CSV trace file, JSON writer, math logger, LogData assembly
   ├── outer-loop state: benders_num_run, lambda, bilevel_best_ub, outer solution,
   │                     DO_OUTER_LOOP checks inside output methods
   ├── stopping/relax:   gap checks, integer/continuous master switch
   └── data bag:         CurrentIterationData _data (30 mutable public fields)

Worker ("mother class": LP proxy + solver lifecycle + file IO, public data)
   ├── SubproblemWorker   (subgradient, fix_to, skeleton delete_rows)
   └── WorkerMaster       (cut bookkeeping ×5 methods, alphas fixing, integrity constraints)

BendersMpi : BendersBase        (MPI steps step_1/2/4, overrides SolveSubproblem for criteria)
   └── BendersByBatch       (610 lines, batch loop, duplicates cut-gathering with batch args)
BendersSequential : BendersBase (used ONLY by unit tests; production is always MPI)

BendersPlugin (11 callbacks) ← NoOperationPlugin, Benders_MICRO_ITERS (593 lines:
   Julia FFI via dlopen + 10 C function pointers, constraint tracking/replay, skeleton loading)
```

## 3. Complexity audit

| # | Finding | Evidence |
|---|---|---|
| F1 | **`BendersBase` is a god class** — 9+ responsibilities in one type; 1604-line cpp, 382-line header, ~108 methods | `BendersBase.h/.cpp` |
| F2 | **Target-level dependency cycles** core↔outer_loop and core↔plugins | both `CMakeLists.txt`; `MasterUpdate.h` include |
| F3 | **MPI leaks into core & plugins**: `SubproblemWorkerFactory.h` includes `boost/mpi.hpp` (communicator ctor param); `Benders_MICRO_ITERS` includes `benders_mpi/common_mpi.h`; `BendersPluginFactory` takes `boost::mpi::communicator*` | `SubproblemWorkerFactory.h:6`, `Benders_MICRO_ITERS.h:27`, `BendersPluginFactory.cpp:17` |
| F4 | **Dead code**: `ICutsManager`/`CutsManagerRunTime` constructed & injected but never used; `workerMasterDataVect_` + `AllCuts()` referenced only by comments; `BendersSequential` never built in production; commented-out `BendersCuts` blocks | `BendersApp.cpp:185`, `BendersBase.h:160`, `BendersFactory.cpp:120-150` |
| F5 | **Outer-loop state embedded in core**: `CriteriaCurrentIterationData` (benders_num_run, λ, λ_min/max, bilevel_best_ub) nested in `CurrentIterationData` (marked `// ugly`); 10+ outer-loop-only methods on `BendersBase`; `DO_OUTER_LOOP` checked inside core output methods | `BendersStructsDatas.h:60`, `BendersBase.cpp:925,1390` |
| F6 | **`Worker`/`WorkerMaster` overloaded**: "mother class" mixes LP proxy, solver lifecycle, file IO and exposes public data (`_solver`, `_name_to_id`, `_id_to_name`, `_is_master`); `WorkerMaster` has 5 overlapping `add*cut*` methods + test-only setters | `Worker.h`, `WorkerMaster.h` |
| F7 | **Three overlapping subproblem-cut paths** (`GetSubproblemCut` / `...Fast` / `...Cache` / `GetCompactInMemCuts`) selected by `CACHE_PROBLEMS` 0/1/2, **duplicated with batch variants** in `BendersByBatch` | `BendersBase.h:216-221`, `BendersByBatch.h:44-52` |
| F8 | **Plugin contract is a 11-method callback list** with wide parameter types; `Benders_MICRO_ITERS` bundles Julia FFI + constraint tracking + skeleton loading; special-case solver hand-off in `BendersMpi::launch` (`CACHE_PROBLEMS==2` builds skeleton solver, passes it into `OnBendersStart`) | `BendersPlugin.h`, `Benders_MICRO_ITERS.cpp`, `BendersMPI.cpp:580` |
| F9 | **`BendersByBatch : BendersMpi`** — the batch algorithm is entangled with the MPI backend; cannot be run/tested sequentially | `BendersByBatch.h:7` |
| F10 | **Orchestration state scattered**: `BendersApp` holds `benders_`, `criterion_input_holder_` (a `std::variant` visited everywhere), `method_`, `context_`; `RunExternalLoop` hand-wires 4 collaborators | `BendersApp.cpp` |
| F11 | **Inconsistent naming/encapsulation**: `_data` vs `benders_plugin_` vs `subproblem_map` (no underscore); PascalCase + snake_case methods; public data members `_logger`, `_writer` accessed as `benders_->_logger` from `OuterLoopBenders` | `BendersBase.h:137-139`, `OuterLoopBenders.cpp:23-25` |

### 3.1 What the legacy branches did (and what to keep)

`refacto_outer_5` (final state) introduced an **`OuterLoopBendersAdapter`** wrapping
`pBendersBase`:

- outer-loop state (λ, λ_min/max, benders_num_run, bilevel_best_ub, outer solution,
  iteration snapshot) moved **out of `BendersBase` into the adapter**;
- `BendersBase` lost its outer-loop-only methods, gained generic accessors
  (`GetLastWorkerMasterData`, `GetCriteriaPerIteration`, `GetCurrentBendersSolution`);
- `suppress_output_file_writes_` flag replaced `DO_OUTER_LOOP` checks inside core output methods;
- adapter `Launch()` = `benders_->launch()` + state refresh (removes the
  "remember to refresh" obligation);
- removed the unused `ICutsManager` from `OuterLoopBenders`.

Assessment: **the direction is sound** and it complements the already-merged
`ICommunicationStrategy` work. However the branch is stale (pre micro-iterations 2/3,
pre skeleton, `OuterLoopBenders` was still in `benders_mpi/`), oscillated between
"adapter" and "data accessor" designs mid-flight, and ended with a `fix build` commit —
rebase friction on top of a heavily-changed `BendersBase.cpp`.

**Decision: do not rebase. Re-land the final adapter state as a fresh, small PR on top of
current develop.**

## 4. Target architecture

```
factories (BendersApp / BendersFactory / PluginFactory)
   │  depends on: engine API, outer_loop, plugins (concrete)
   ▼
engine (benders_core)
   BendersBase  →  orchestrator only:
   ├── MasterProblemController   WorkerMaster, variable map, master path, Master* API
   ├── SubproblemService         factory, worker map, cut gathering (1 path, config-driven),
   │                             basis cache, CACHE_PROBLEMS modes
   ├── CutBuilder                aggregation, per-cut tolerances, build_all_aggregated_cuts
   ├── CriteriaService           CriterionComputation, per-iteration vectors, max-criterion area
   ├── BendersOutputService      JSON writer, CSV trace, math logger, LogData/SolutionData
   └── BendersState              CurrentIterationData split into small typed structs
   backends: BendersMpi / BendersSequential / BendersByBatch
             (communication via ICommunicationStrategy; loop-variant hooks, no criteria
              side-effect overrides)
   plugins: IBendersPlugin INTERFACE lives in engine; narrow resolution context
outer_loop
   OuterLoopBenders + OuterLoopBendersAdapter (owns outer-loop state)
   + MasterUpdateBase + OuterLoopBiLevel
   depends on engine through narrow interfaces only (no pBendersBase in headers)
plugins (concrete)
   NoOperationPlugin, MicroIterationsPlugin (Julia FFI isolated in a dedicated backend class)
```

Dependency rules after the refactor:

- `outer_loop` → engine (one-way); engine never includes outer_loop headers;
- engine defines the plugin interface; concrete plugins → engine (one-way);
- `boost/mpi` appears only in `benders_mpi` (and its test dependencies);
- no target-level cycles (verifiable by eye / CMake graph).

## 5. Phased plan

Every phase lands as an independent, green PR (format + unit + e2e gates, §6).
Phases are strictly incremental; each ends with a working system.

> **Status (branch `refactor/benders-complexity`)**: P1, P2 and P3 are implemented
> as separate commits on top of the review commit and validated: full build
> green, unit tests green, and the e2e gates green (`benders_memory_and_micro_iterations`,
> `outer_loop_tests`, `Benders`, `Benders_criterion_output_tests`, `benders_aggreg_cuts`).
> The only failing e2e targets (`examples_long_sequential`, `examples_long_mpi`,
> `examples_long_benders_by_batch_mpi`) are pre-existing on `origin/develop`: #1105
> ("Disable automatic study update") removed the automatic study update from the
> full-run paths but left `verify_study_update` in the long tests, and CI excludes
> the `long_*` labels. Out of scope for this refactor.
> P4–P6 remain to be scheduled.

### Phase 0 — Baseline & safety net (prerequisite)
1. Restore local build environment (vcpkg checkout + toolchain; no `CMakeCache` survives on
   this machine and network was unavailable at review time) **or** agree that CI is the gate.
2. Record baseline results of the e2e equivalence suite
   (`benders_memory_and_micro_iterations.feature`: identical cost/solution across cache
   levels 0/1/2, batch on/off, 1/3 procs, micro-it ± warm-start) and
   `outer_loop_tests.feature`. These exact values are the regression contract.
3. Extend unit coverage where components will be moved (CutBuilder, MasterProblemController,
   SubproblemService) *before* moving them — characterization tests on current behavior.

### Phase 1 — Dead code & hygiene (low risk, 1 PR) ✅ done

- Delete `ICutsManager`/`CutsManagerRunTime` (`CutsManagement.{h,cpp}`), the injection in
  `BendersApp::RunExternalLoop`, and the member in `OuterLoopBenders` (already done on the
  legacy branch: commit `9d80437f7`).
- Delete `workerMasterDataVect_`, `AllCuts()`, commented-out `BendersCuts`/`Clean` blocks,
  and the now-unused `WorkerMasterDataVect` typedef.
- Move the free `selectPolicy` template out of `BendersBase.h` into `ExecutionPolicy.h`.
- Keep `BendersSequential` (valuable test double) and document it as test-only.

### Phase 2 — Break the target cycles (medium risk, 1–2 PRs) ✅ done
1. Move the `IBendersPlugin` interface (and only it) from `plugins` into `benders_core`
   (renamed from `BendersPlugin`). `plugins` keeps the concrete plugins. → core↔plugins
   cycle broken.
2. Move `MasterUpdateBase` + `IMasterUpdate` from `benders_core` to `outer_loop`.
   → core↔outer_loop cycle broken; `benders_core` no longer links `outer_loop_lib`.
3. De-MPI the skeleton loading path in `benders_core`: `SubproblemWorkerFactory`,
   `SkeletonConstraintSetLoader` and `SkeletonCoefficientSet` take an optional
   fatal-error callback instead of a `boost::mpi::communicator*`; `BendersMpi` and the
   micro-iterations plugin supply the abort callback. `boost/mpi` no longer appears in
   `benders_core`.

Known remaining cycle (out of scope, noted for later): `benders_mpi_core` ↔ `factories`.

### Phase 3 — Outer-loop adapter (re-land refacto_outer_5, medium risk, 1–2 PRs) ✅ done
1. `OuterLoopBendersAdapter` owns λ/λ_min/λ_max, benders_num_run, bilevel_best_ub, outer
   solution data; `Launch()` = seed engine context + `benders_->launch()` + state refresh.
2. `BendersBase` lost its outer-loop state & methods; gained generic accessors
   (`StartOuterLoopIteration(ctx)`, `LastIterationSnapshot`, `GetCriteriaPerIteration`,
   `GetCurrentBendersSolution`).
3. Per-iteration/final output-file writes in the engine are gated by
   `suppress_output_file_writes_` (set by the adapter) instead of
   `EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP`.
4. `OuterLoopBenders` talks to the engine only through the adapter; `OuterLoopLambdaMin/Max`
   left the `OuterLoop` interface (kept concrete on `OuterLoopBenders`).

Behavior-preservation notes: the per-iteration math-logging columns (benders_num_run,
max_criterion\*, bilevel best ub, λ values) are seeded into the engine before each run via
`StartOuterLoopIteration` — identical values to the removed 3-arg `init_data`; the timer
restart semantics are preserved.

### Phase 4 — Decompose `BendersBase` (core of the effort, 4–6 PRs)
Slice in this order (each slice: extract → delegate → slim base class → green):
1. **`CutBuilder`** — `compute_cut`, `compute_cut_aggregate`, `build_all_aggregated_cuts`,
   `SetAggregation`, `GetSubCutTolerance`. Needs only: cut data in, "add cut to master"
   callback out. Purest extraction; no state beyond options.
2. **`MasterProblemController`** — all `Master*` pass-throughs, `master_variable_map_`,
   master path/name, `MasterIsEmpty`, reset-from-last-iteration. `MasterUpdateBase` and the
   adapter then depend on this narrow type instead of `BendersBase`.
3. **`SubproblemService`** — worker factory + `subproblem_map`, the three cut-gathering
   paths unified into one `GatherCuts(mode)` (kill the Fast/Cache/Compact duplication incl.
   the by-batch variants where possible), basis save/restore, `SolveSubproblem` with plugin
   hooks. MPI criteria side-effect (`BendersMpi::SolveSubproblem` override) becomes an
   injectable per-subproblem hook (`CriteriaService` registers it) — removes the override.
4. **`BendersOutputService` + `BendersState`** — CSV trace, `Save*`/`Print*` methods,
   `bendersDataToLogData`/`iteration`/`solution` assembly; split `CurrentIterationData`
   into `BoundsState` (lb/ub/best), `CostState`, `IterationMeta` (it, stop, timings),
   `SolutionState` (x_in/x_out/x_cut, invest/operational). Math logger reads the split
   structs.
5. **Backend slimming** — after 1–4, `BendersBase` is an orchestrator (~300 lines target);
   `BendersMpi` keeps only MPI mechanics; audit remaining `_data` field accesses.

### Phase 5 — Plugin contract (medium risk, 1–2 PRs)
1. Narrow `IBendersPlugin`: replace the wide `OnBendersStart(subproblem_map, logger,
   options, solver_log_manager, sub_problem_solver)` with a context object
   (`ISubproblemResolutionContext`: worker, name, solver, x_cut, iteration counters) —
   or keep the 11 callbacks but with narrowed parameters; measure first.
2. Isolate the Julia FFI (dlopen + C function-pointer table) into a
   `JuliaMicroIterationBackend` class inside the plugin target; the plugin implements the
   contract and delegates. FFI signatures stay byte-identical (external contract).
3. Remove the skeleton-solver hand-off from `BendersMpi::launch`
   (`CACHE_PROBLEMS==2` special case): the plugin obtains the shared solver from
   `SubproblemWorkerFactory` (which already owns it) — engine stops knowing about it.

### Phase 6 — `BendersByBatch` decoupling (optional, last)
Re-express the batch algorithm as a **loop policy over the engine** (separation loop over
batches) instead of inheriting from `BendersMpi`. Enables sequential by-batch unit tests and
removes the duplicated cut-gathering. Highest-risk phase; only after Phase 4 stabilizes.

## 6. Regression gates (every PR)

| Gate | What it catches |
|---|---|
| `ctest -L unit` (benders + outer_loop suites) | component behavior |
| e2e `Benders.feature`, `benders_aggreg_cuts.feature`, `Benders_criterion_output_tests.feature` | convergence, cut aggregation, criterion files |
| e2e `benders_memory_and_micro_iterations.feature` | **equivalence**: identical cost/solution across cache 0/1/2 × batch × 1/3 procs × micro-it ± warm-start |
| e2e `outer_loop_tests.feature` | outer loop end-to-end |
| Determinism spot-check | same study → same final JSON (do not reorder TBB/map iterations) |
| Resume mode | `last_iteration.json` shape & keys unchanged (used by restart) |

## 7. Risks & mitigations

| Risk | Mitigation |
|---|---|
| MPI path only verifiable in CI (`mpirun`) | keep 3-proc e2e scenarios in every benders PR; CI runs them |
| Numerical drift from reordering reductions | never change iteration order of maps / `par_unseq` reductions; equivalence e2e is the tripwire |
| Julia FFI fragility (MICRO_ITERS) | FFI signatures frozen; `RecordingSolver`-based unit test already exists (`BendersMicroIterationsTest`) |
| Output/JSON contract (resume, downstream tools) | no key renames; `last_iteration.json` e2e covered |
| In-memory / compact mode used by other workflows | `GetCompactInMemCuts`/`init_for_compact_in_mem` API preserved through Phase 4 |
| Scope creep in Phase 4 | strict slicing (§5.4), one component per PR, base class shrinks monotonically |

## 8. Decisions

1. **Scope & cadence** — P0–P3 first, then reassess P4–P6. ✅ approved; P1–P3 implemented
   on `refactor/benders-complexity`, pending build/test validation.
2. **Build environment** — local build generation/build/test handled by the maintainer
   (no usable vcpkg toolchain on the review machine).
3. **Conflicts** — none reported; P4's `BendersBase` surgery still needs a go/no-go against
   in-memory / micro-iteration work in flight.
4. **`BendersSequential`** — kept as a test-only double (documented in the header).
5. **PR shape for P3** — landed as a single commit (adapter + `BendersBase` trim) on top of
   the P1/P2 commits; split into PRs per commit.