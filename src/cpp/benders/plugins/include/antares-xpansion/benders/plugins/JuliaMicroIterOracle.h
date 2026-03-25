/*
 * JuliaMicroIterOracle.h
 *
 * Concrete implementation of IMicroIterationOracle that loads an external
 * shared library (e.g. a compiled Julia module) via dlopen and calls the
 * C ABI functions declared in MicroIterationOracleCABI.h.
 *
 * This is the only class in the codebase that:
 *   - Knows about dlopen / dlsym / dlclose
 *   - Knows about the jl_oracle_* symbol names
 *   - Handles OracleStateBuffer / ViolatedConstraintKeys C structs
 *   - Converts between DecisionPoint (C++ map) and flat C arrays
 *
 * It does NOT know about:
 *   - MPI, ConstraintsReader, SubproblemWorker
 *   - Business-domain concepts (HVDC, PTDF, investment decisions)
 *   - The constraints dictionary or warm-start logic
 *
 * Thread safety
 * -------------
 * EvaluateViolatedConstraints acquires a mutex before calling across the
 * Julia boundary because Julia's GC is not fully thread-safe. This serializes
 * oracle evaluations within a rank but avoids races. When Julia multi-threading
 * is verified safe for jl_oracle_evaluate, the mutex can be removed.
 */

#pragma once

#include <dlfcn.h>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "antares-xpansion/benders/plugins/IMicroIterationOracle.h"
#include "antares-xpansion/benders/plugins/MicroIterationOracleCABI.h"


class JuliaMicroIterOracle final : public IMicroIterationOracle
{
public:
    /*
     * Load the shared library and resolve all required C ABI symbols.
     *
     * @param shared_library_path  Path to the compiled .so / .dylib / .dll file.
     *
     * @throws std::runtime_error  If dlopen fails or any required symbol is missing.
     */
    explicit JuliaMicroIterOracle(const std::filesystem::path& shared_library_path);

    /*
     * Calls Finalize() if not already called, then dlclose.
     */
    ~JuliaMicroIterOracle() override;

    // Non-copyable, non-movable: owns the dlopen handle.
    JuliaMicroIterOracle(const JuliaMicroIterOracle&)            = delete;
    JuliaMicroIterOracle& operator=(const JuliaMicroIterOracle&) = delete;
    JuliaMicroIterOracle(JuliaMicroIterOracle&&)                  = delete;
    JuliaMicroIterOracle& operator=(JuliaMicroIterOracle&&)       = delete;

    /*
     * Calls jl_oracle_initialize with flattened subproblem_ids and
     * master_variable_names arrays. The oracle_config_dir path is passed
     * as a null-terminated C string.
     */
    void Initialize(
        const std::vector<std::string>& subproblem_ids,
        const std::vector<std::string>& master_variable_names,
        const std::filesystem::path&    oracle_config_dir) override;

    /*
     * Calls jl_oracle_finalize to shut down the Julia runtime.
     */
    void Finalize() override;

    /*
     * Converts master_decision_point to flat C arrays, calls
     * jl_oracle_update_state, copies the returned bytes into an OracleStateBytes
     * vector, calls jl_oracle_free_state, and returns the vector.
     *
     * Called on rank 0 only; the returned bytes are broadcast by the C++ adapter.
     */
    [[nodiscard]]
    OracleStateBytes UpdateStateFromMaster(
        const DecisionPoint& master_decision_point,
        int                  iteration_number) override;

    /*
     * Calls jl_oracle_deserialize_state with the raw bytes from the broadcast.
     * Called on all MPI ranks (including rank 0) by the C++ adapter.
     */
    void DeserializeState(const OracleStateBytes& serialized_state) override;

    /*
     * Converts sub_solution to flat C arrays, calls jl_oracle_evaluate,
     * copies the returned constraint key strings into a ConstraintKeySet,
     * calls jl_oracle_free_constraint_keys, and returns the vector.
     *
     * Thread-safe: protected by oracle_mutex_ for Julia GC safety.
     */
    [[nodiscard]]
    ConstraintKeySet EvaluateViolatedConstraints(
        const std::string&   sub_id,
        const DecisionPoint& sub_solution) override;

    /*
     * Calls jl_oracle_on_master_iteration_end.
     */
    void OnMasterIterationEnd(int iteration_number) override;

private:
    /*
     * Load a symbol from the shared library, casting to Fn.
     * Throws std::runtime_error if the symbol is not found.
     */
    template <typename Fn>
    Fn LoadSymbol(const char* symbol_name);

    void* handle_ = nullptr;

    // Resolved function pointers
    jl_oracle_initialize_fn              fn_initialize_    = nullptr;
    jl_oracle_finalize_fn                fn_finalize_      = nullptr;
    jl_oracle_update_state_fn            fn_update_state_  = nullptr;
    jl_oracle_free_state_fn              fn_free_state_    = nullptr;
    jl_oracle_deserialize_state_fn       fn_deserialize_   = nullptr;
    jl_oracle_evaluate_fn                fn_evaluate_      = nullptr;
    jl_oracle_free_constraint_keys_fn    fn_free_keys_     = nullptr;
    jl_oracle_on_master_iteration_end_fn fn_iter_end_      = nullptr;

    bool finalized_ = false;

    /*
     * Mutex protecting calls into Julia.
     * EvaluateViolatedConstraints acquires this lock before calling
     * jl_oracle_evaluate to prevent concurrent GC races.
     * UpdateStateFromMaster and DeserializeState are called sequentially
     * by the C++ adapter so they do not need the lock.
     */
    mutable std::mutex oracle_mutex_;
};
