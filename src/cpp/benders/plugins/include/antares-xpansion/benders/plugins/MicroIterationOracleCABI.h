/*
 * MicroIterationOracleCABI.h
 *
 * Stable C ABI for loading a micro-iteration oracle from an external shared
 * library (e.g. a Julia .so) via dlopen.
 *
 * This header declares the C function signatures that an external library must
 * export to be usable as an IMicroIterationOracle implementation via
 * JuliaMicroIterOracle.
 *
 * ABI design principles
 * ---------------------
 * - No C++ types cross the boundary: only C primitives, flat arrays, and
 *   simple POD structs.
 * - No business-domain concepts appear: variable names and values are passed
 *   as flat parallel arrays; the external library interprets them.
 * - Memory ownership is explicit: functions that return heap-allocated data
 *   always have a corresponding free function that must be called after the
 *   caller has copied the data. This avoids GC-lifetime hazards when the
 *   external runtime (e.g. Julia) manages its own memory.
 * - A single opaque byte blob carries the serialized oracle state, replacing
 *   any domain-specific named buffers in prior designs.
 *
 * Correspondence to IMicroIterationOracle
 * ----------------------------------------
 *   jl_oracle_initialize          -> IMicroIterationOracle::Initialize
 *   jl_oracle_finalize            -> IMicroIterationOracle::Finalize
 *   jl_oracle_update_state        -> IMicroIterationOracle::UpdateStateFromMaster
 *   jl_oracle_free_state          -> (memory management for update_state return)
 *   jl_oracle_deserialize_state   -> IMicroIterationOracle::DeserializeState
 *   jl_oracle_evaluate            -> IMicroIterationOracle::EvaluateViolatedConstraints
 *   jl_oracle_free_constraint_keys-> (memory management for evaluate return)
 *   jl_oracle_on_master_iteration_end -> IMicroIterationOracle::OnMasterIterationEnd
 */

#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Lifecycle
 * ------------------------------------------------------------------------- */

/*
 * Initialize the oracle runtime and load oracle-specific data files.
 *
 * Must be called once before any other jl_oracle_* function.
 *
 * @param oracle_config_dir
 *   Null-terminated path to the directory containing oracle input files.
 *   The oracle reads its own data from this directory; C++ does not inspect
 *   the contents.
 *
 * @param subproblem_ids   Array of n_subproblems null-terminated subproblem ID strings.
 * @param n_subproblems    Length of subproblem_ids array.
 * @param master_var_names Array of n_master_vars null-terminated variable name strings.
 * @param n_master_vars    Length of master_var_names array. May be 0 if the oracle
 *                         discovers master variable names from oracle_config_dir.
 */
void jl_oracle_initialize(
    const char*  oracle_config_dir,
    const char** subproblem_ids,
    int          n_subproblems,
    const char** master_var_names,
    int          n_master_vars);

/*
 * Finalize the oracle and shut down the embedded runtime (e.g. Julia GC).
 * Must be called once after the Benders loop ends.
 */
void jl_oracle_finalize(void);


/* -------------------------------------------------------------------------
 * Oracle state (master iteration, rank 0 path)
 * ------------------------------------------------------------------------- */

/*
 * Opaque buffer returned by jl_oracle_update_state.
 *
 * Ownership: bytes_ptr points into external-runtime-managed memory.
 * The C++ caller MUST copy bytes_ptr[0..bytes_length-1] into its own buffer
 * before calling any other jl_oracle_* function (which may trigger GC or
 * invalidate the pointer). Call jl_oracle_free_state when done copying.
 */
struct OracleStateBuffer
{
    const uint8_t* bytes_ptr;
    int            bytes_length;
};

/*
 * Compute oracle state from the current master solution (rank 0 only).
 *
 * Corresponds to IMicroIterationOracle::UpdateStateFromMaster.
 *
 * @param var_names    Array of n_vars null-terminated master variable name strings.
 * @param var_values   Parallel array of n_vars double values.
 * @param n_vars       Number of master variables.
 * @param iteration    Benders iteration number (1-based).
 *
 * @return  Buffer pointing into external-runtime memory. Copy before next call.
 */
OracleStateBuffer jl_oracle_update_state(
    const char**  var_names,
    const double* var_values,
    int           n_vars,
    int           iteration);

/*
 * Release the external-runtime buffer allocated by jl_oracle_update_state.
 * Must be called after the caller has finished copying the bytes.
 */
void jl_oracle_free_state(OracleStateBuffer buf);

/*
 * Restore oracle state from serialized bytes (called on all MPI ranks).
 *
 * Corresponds to IMicroIterationOracle::DeserializeState.
 *
 * @param bytes         Pointer to the broadcast state bytes.
 * @param bytes_length  Number of bytes.
 */
void jl_oracle_deserialize_state(
    const uint8_t* bytes,
    int            bytes_length);


/* -------------------------------------------------------------------------
 * Micro-iteration separation oracle
 * ------------------------------------------------------------------------- */

/*
 * Array of violated constraint keys returned by jl_oracle_evaluate.
 *
 * Ownership: keys_ptr and each string it points to are owned by the external
 * runtime. The C++ caller MUST copy all strings before calling any other
 * jl_oracle_* function. Call jl_oracle_free_constraint_keys when done.
 *
 * n_keys == 0 means no constraints are violated: the micro-iteration loop
 * for this subproblem should terminate (convergence reached).
 */
struct ViolatedConstraintKeys
{
    const char** keys_ptr;
    int          n_keys;
};

/*
 * Separation oracle: find violated constraint keys for one subproblem solve.
 *
 * Corresponds to IMicroIterationOracle::EvaluateViolatedConstraints.
 *
 * Thread safety: safe for concurrent calls with distinct sub_id values.
 *
 * @param sub_id       Null-terminated subproblem identifier.
 * @param var_names    Array of n_vars null-terminated subproblem variable names.
 * @param var_values   Parallel array of n_vars double values (primal solution).
 * @param n_vars       Number of subproblem variables.
 *
 * @return  Violated constraint keys (external-runtime memory; copy before reuse).
 */
ViolatedConstraintKeys jl_oracle_evaluate(
    const char*   sub_id,
    const char**  var_names,
    const double* var_values,
    int           n_vars);

/*
 * Release the external-runtime buffer allocated by jl_oracle_evaluate.
 * Must be called after the caller has finished copying the keys.
 */
void jl_oracle_free_constraint_keys(ViolatedConstraintKeys keys);


/* -------------------------------------------------------------------------
 * End-of-iteration notification
 * ------------------------------------------------------------------------- */

/*
 * Notify the oracle that all micro-iterations for this master iteration ended.
 *
 * Corresponds to IMicroIterationOracle::OnMasterIterationEnd.
 *
 * @param iteration  The master iteration number that just completed (1-based).
 */
void jl_oracle_on_master_iteration_end(int iteration);

#ifdef __cplusplus
} // extern "C"
#endif

/* -------------------------------------------------------------------------
 * Function pointer type aliases (for use with dlsym)
 * ------------------------------------------------------------------------- */

#ifdef __cplusplus

using jl_oracle_initialize_fn             = decltype(&jl_oracle_initialize);
using jl_oracle_finalize_fn               = decltype(&jl_oracle_finalize);
using jl_oracle_update_state_fn           = decltype(&jl_oracle_update_state);
using jl_oracle_free_state_fn             = decltype(&jl_oracle_free_state);
using jl_oracle_deserialize_state_fn      = decltype(&jl_oracle_deserialize_state);
using jl_oracle_evaluate_fn               = decltype(&jl_oracle_evaluate);
using jl_oracle_free_constraint_keys_fn   = decltype(&jl_oracle_free_constraint_keys);
using jl_oracle_on_master_iteration_end_fn = decltype(&jl_oracle_on_master_iteration_end);

#endif // __cplusplus
