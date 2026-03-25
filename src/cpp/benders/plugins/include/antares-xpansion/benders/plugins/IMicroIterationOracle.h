/*
 * IMicroIterationOracle.h
 *
 * Pure abstract interface for a lazy-constraint oracle used in Benders
 * micro-iterations.
 *
 * Mathematical context
 * --------------------
 * In each Benders master iteration t, the master problem yields a primal
 * solution x^t. For each subproblem k, the micro-iteration loop repeatedly
 * solves a restricted LP and adds violated constraints until none are found.
 *
 * This oracle encapsulates the separation function S:
 *
 *   S(k, x^t, y^(k,m)) -> { c in C | g_c(x^t, y^(k,m)) > 0 }
 *
 * where:
 *   - k   is the subproblem identifier
 *   - x^t is the master decision point (fixed for all micro-iterations)
 *   - y^(k,m) is the subproblem primal solution after the m-th micro-solve
 *   - C   is the pool of candidate lazy constraints
 *   - g_c is the violation function for constraint key c
 *
 * The oracle is stateful: UpdateStateFromMaster(x^t) precomputes parameters
 * Theta(x^t) shared across all subsequent EvaluateViolatedConstraints calls
 * within the same master iteration.
 *
 * Responsibilities of this interface (oracle side):
 *   - Maintain Theta(x^t) and serialize/deserialize it for MPI broadcast
 *   - Identify violated constraint keys given a subproblem solution
 *
 * Responsibilities of the C++ adapter (NOT this interface):
 *   - MPI transport: broadcasting serialized state to all ranks
 *   - Constraint file I/O: reading LP rows from MPS files
 *   - Row mutation: adding/removing rows on SubproblemWorker
 *   - Deduplication: not re-adding already-active constraints
 *   - Warm-start lifecycle: deleting rows between master iterations
 *
 * Thread safety
 * -------------
 * EvaluateViolatedConstraints may be called concurrently from different
 * threads for distinct sub_id values. Implementations must be thread-safe
 * for such concurrent calls. UpdateStateFromMaster and DeserializeState are
 * always called sequentially, never concurrently with EvaluateViolatedConstraints.
 */

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

/*
 * A point in decision space: maps solver variable names to their values.
 * Used for both the master solution x^t and subproblem solutions y^(k,m).
 */
using DecisionPoint = std::map<std::string, double>;

/*
 * A set of constraint keys identifying violated constraints.
 * Each key is an opaque string that the C++ adapter maps to concrete LP
 * row definitions via constraints_dictionary.csv.
 * Mathematical meaning: indices into the constraint pool C.
 */
using ConstraintKeySet = std::vector<std::string>;

/*
 * Serialized oracle state for MPI broadcast.
 * Encodes Theta(x^t): the internal parameters precomputed by the oracle
 * after receiving the master solution. The encoding is fully owned by the
 * oracle implementation; C++ treats it as an opaque byte sequence.
 */
using OracleStateBytes = std::vector<uint8_t>;


class IMicroIterationOracle
{
public:
    virtual ~IMicroIterationOracle() = default;

    /*
     * Initialize the oracle for a Benders run. Called once before any
     * master iteration begins.
     *
     * @param subproblem_ids
     *   IDs of the subproblems managed by this MPI rank (index set K_rank).
     *   The oracle may use these to pre-allocate per-subproblem data structures.
     *
     * @param master_variable_names
     *   Names of all variables in the master decision point. Passed so the
     *   oracle can pre-build index maps. An empty vector is valid if the
     *   oracle discovers variable names from oracle_config_dir.
     *
     * @param oracle_config_dir
     *   Directory containing all oracle-specific input files (e.g. serialized
     *   Julia structures, network data, per-subproblem metadata). C++ never
     *   reads from this directory; it is entirely owned by the oracle.
     */
    virtual void Initialize(
        const std::vector<std::string>& subproblem_ids,
        const std::vector<std::string>& master_variable_names,
        const std::filesystem::path&    oracle_config_dir) = 0;

    /*
     * Release all oracle resources. Called once after the Benders loop ends.
     * Implementations should finalize any embedded runtimes (e.g. Julia GC).
     */
    virtual void Finalize() = 0;

    /*
     * Compute oracle state from the current master solution (rank 0 only).
     *
     * Called after each master solve, before subproblem solves begin.
     * This is the expensive step: it precomputes Theta(x^t) = f(x^t, data),
     * the internal parameters that allow EvaluateViolatedConstraints to run
     * efficiently for any subproblem and primal solution.
     *
     * @param master_decision_point
     *   Full master solution x^t: all master variable names and their values.
     *   The oracle is responsible for extracting the variables it cares about;
     *   no C++-side filtering by domain concept is performed here.
     *
     * @param iteration_number  Benders iteration counter (1-based).
     *
     * @return
     *   Serialized encoding of Theta(x^t). These bytes will be broadcast to
     *   all MPI ranks and then passed verbatim to DeserializeState on each rank.
     *   The returned vector is owned by the caller after return.
     */
    [[nodiscard]]
    virtual OracleStateBytes UpdateStateFromMaster(
        const DecisionPoint& master_decision_point,
        int                  iteration_number) = 0;

    /*
     * Restore oracle state from bytes broadcast from rank 0.
     *
     * Called on all MPI ranks (including rank 0) after the broadcast of the
     * serialized oracle state. After this call the oracle on every rank holds
     * the same Theta(x^t) computed on rank 0, ensuring EvaluateViolatedConstraints
     * behaves identically regardless of rank.
     *
     * @param serialized_state  Bytes returned by UpdateStateFromMaster on rank 0.
     */
    virtual void DeserializeState(const OracleStateBytes& serialized_state) = 0;

    /*
     * Separation oracle: identify violated constraints for one subproblem solve.
     *
     * Called inside the micro-iteration loop for subproblem sub_id.
     * May be called multiple times per (subproblem, master-iteration) pair
     * until it returns an empty set, which signals convergence and terminates
     * the loop for that subproblem.
     *
     * Mathematical meaning: given the current y^(k,m), evaluate
     *   { c in C | g_c(x^t, y^(k,m)) > 0 }
     * using the precomputed Theta(x^t) stored internally.
     *
     * @param sub_id        Subproblem identifier k.
     *
     * @param sub_solution
     *   Full current primal solution of subproblem k: all variable names and
     *   values. The oracle filters internally to extract the variables it needs
     *   (e.g. flow variables); no C++-side variable selection is performed.
     *
     * @return
     *   Keys of violated constraints. An empty vector means no violation was
     *   found: the micro-iteration loop for sub_id terminates. Each returned
     *   key maps to one or more concrete LP rows via the constraints dictionary
     *   managed by the C++ adapter.
     *
     * Thread safety: safe for concurrent calls with distinct sub_id values.
     */
    [[nodiscard]]
    virtual ConstraintKeySet EvaluateViolatedConstraints(
        const std::string&   sub_id,
        const DecisionPoint& sub_solution) = 0;

    /*
     * Notification that all subproblem micro-iterations for master iteration
     * iteration_number have completed.
     *
     * Gives the oracle an opportunity to reset per-iteration bookkeeping,
     * flush diagnostics, or update internal caches. No-op implementation is valid.
     *
     * @param iteration_number  The master iteration that just completed.
     */
    virtual void OnMasterIterationEnd(int iteration_number) = 0;
};
