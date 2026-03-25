/*
 * JuliaMicroIterOracle.cpp
 *
 * Implementation of JuliaMicroIterOracle: loads an external shared library
 * via dlopen and calls the C ABI functions declared in MicroIterationOracleCABI.h.
 */

#include "antares-xpansion/benders/plugins/JuliaMicroIterOracle.h"

#include <cstring>
#include <stdexcept>


/* --------------------------------------------------------------------------
 * Construction / destruction
 * -------------------------------------------------------------------------- */

JuliaMicroIterOracle::JuliaMicroIterOracle(
    const std::filesystem::path& shared_library_path)
{
    handle_ = dlopen(shared_library_path.c_str(), RTLD_NOW);
    if (!handle_)
    {
        throw std::runtime_error(
            std::string("JuliaMicroIterOracle: dlopen failed for '")
            + shared_library_path.string()
            + "': "
            + dlerror());
    }

    fn_initialize_   = LoadSymbol<jl_oracle_initialize_fn>            ("jl_oracle_initialize");
    fn_finalize_     = LoadSymbol<jl_oracle_finalize_fn>              ("jl_oracle_finalize");
    fn_update_state_ = LoadSymbol<jl_oracle_update_state_fn>          ("jl_oracle_update_state");
    fn_free_state_   = LoadSymbol<jl_oracle_free_state_fn>            ("jl_oracle_free_state");
    fn_deserialize_  = LoadSymbol<jl_oracle_deserialize_state_fn>     ("jl_oracle_deserialize_state");
    fn_evaluate_     = LoadSymbol<jl_oracle_evaluate_fn>              ("jl_oracle_evaluate");
    fn_free_keys_    = LoadSymbol<jl_oracle_free_constraint_keys_fn>  ("jl_oracle_free_constraint_keys");
    fn_iter_end_     = LoadSymbol<jl_oracle_on_master_iteration_end_fn>("jl_oracle_on_master_iteration_end");
}

JuliaMicroIterOracle::~JuliaMicroIterOracle()
{
    if (!finalized_)
    {
        Finalize();
    }
    if (handle_)
    {
        dlclose(handle_);
        handle_ = nullptr;
    }
}


/* --------------------------------------------------------------------------
 * Lifecycle
 * -------------------------------------------------------------------------- */

void JuliaMicroIterOracle::Initialize(
    const std::vector<std::string>& subproblem_ids,
    const std::vector<std::string>& master_variable_names,
    const std::filesystem::path&    oracle_config_dir)
{
    // Build flat C arrays of pointers for subproblem IDs
    std::vector<const char*> sub_id_ptrs;
    sub_id_ptrs.reserve(subproblem_ids.size());
    for (const auto& id : subproblem_ids)
        sub_id_ptrs.push_back(id.c_str());

    // Build flat C arrays of pointers for master variable names
    std::vector<const char*> var_name_ptrs;
    var_name_ptrs.reserve(master_variable_names.size());
    for (const auto& name : master_variable_names)
        var_name_ptrs.push_back(name.c_str());

    fn_initialize_(
        oracle_config_dir.c_str(),
        sub_id_ptrs.empty()  ? nullptr : sub_id_ptrs.data(),
        static_cast<int>(sub_id_ptrs.size()),
        var_name_ptrs.empty() ? nullptr : var_name_ptrs.data(),
        static_cast<int>(var_name_ptrs.size()));
}

void JuliaMicroIterOracle::Finalize()
{
    if (!finalized_)
    {
        fn_finalize_();
        finalized_ = true;
    }
}


/* --------------------------------------------------------------------------
 * Oracle state (master iteration)
 * -------------------------------------------------------------------------- */

OracleStateBytes JuliaMicroIterOracle::UpdateStateFromMaster(
    const DecisionPoint& master_decision_point,
    int                  iteration_number)
{
    // Flatten DecisionPoint into parallel C arrays
    std::vector<const char*> var_names;
    std::vector<double>      var_values;
    var_names.reserve(master_decision_point.size());
    var_values.reserve(master_decision_point.size());

    for (const auto& [name, value] : master_decision_point)
    {
        var_names.push_back(name.c_str());
        var_values.push_back(value);
    }

    // Call into the external library (no mutex needed: called sequentially)
    OracleStateBuffer buf = fn_update_state_(
        var_names.data(),
        var_values.data(),
        static_cast<int>(var_names.size()),
        iteration_number);

    // Copy bytes into a C++-owned vector before freeing the external buffer
    OracleStateBytes result;
    if (buf.bytes_length > 0 && buf.bytes_ptr != nullptr)
    {
        result.resize(static_cast<std::size_t>(buf.bytes_length));
        std::memcpy(result.data(), buf.bytes_ptr, static_cast<std::size_t>(buf.bytes_length));
    }

    fn_free_state_(buf);

    return result;
}

void JuliaMicroIterOracle::DeserializeState(const OracleStateBytes& serialized_state)
{
    fn_deserialize_(
        serialized_state.empty() ? nullptr : serialized_state.data(),
        static_cast<int>(serialized_state.size()));
}


/* --------------------------------------------------------------------------
 * Separation oracle
 * -------------------------------------------------------------------------- */

ConstraintKeySet JuliaMicroIterOracle::EvaluateViolatedConstraints(
    const std::string&   sub_id,
    const DecisionPoint& sub_solution)
{
    // Flatten DecisionPoint into parallel C arrays
    std::vector<const char*> var_names;
    std::vector<double>      var_values;
    var_names.reserve(sub_solution.size());
    var_values.reserve(sub_solution.size());

    for (const auto& [name, value] : sub_solution)
    {
        var_names.push_back(name.c_str());
        var_values.push_back(value);
    }

    // Protect Julia GC from concurrent calls
    std::lock_guard<std::mutex> lock(oracle_mutex_);

    ViolatedConstraintKeys keys = fn_evaluate_(
        sub_id.c_str(),
        var_names.data(),
        var_values.data(),
        static_cast<int>(var_names.size()));

    // Copy key strings into a C++-owned vector before freeing the external buffer
    ConstraintKeySet result;
    result.reserve(static_cast<std::size_t>(keys.n_keys));
    for (int i = 0; i < keys.n_keys; ++i)
        result.emplace_back(keys.keys_ptr[i]);

    fn_free_keys_(keys);

    return result;
}


/* --------------------------------------------------------------------------
 * End-of-iteration notification
 * -------------------------------------------------------------------------- */

void JuliaMicroIterOracle::OnMasterIterationEnd(int iteration_number)
{
    fn_iter_end_(iteration_number);
}


/* --------------------------------------------------------------------------
 * Private helper
 * -------------------------------------------------------------------------- */

template <typename Fn>
Fn JuliaMicroIterOracle::LoadSymbol(const char* symbol_name)
{
    // Clear any previous error
    dlerror();

    void* sym = dlsym(handle_, symbol_name);
    const char* error = dlerror();
    if (error != nullptr)
    {
        throw std::runtime_error(
            std::string("JuliaMicroIterOracle: failed to resolve symbol '")
            + symbol_name
            + "': "
            + error);
    }

    Fn fn;
    // Use memcpy to convert void* -> function pointer (avoids UB of direct cast)
    static_assert(sizeof(void*) == sizeof(Fn),
                  "Function pointer size does not match void* size");
    std::memcpy(&fn, &sym, sizeof(fn));
    return fn;
}

// Explicit instantiations for all function pointer types used in this TU
template jl_oracle_initialize_fn              JuliaMicroIterOracle::LoadSymbol<jl_oracle_initialize_fn>             (const char*);
template jl_oracle_finalize_fn                JuliaMicroIterOracle::LoadSymbol<jl_oracle_finalize_fn>               (const char*);
template jl_oracle_update_state_fn            JuliaMicroIterOracle::LoadSymbol<jl_oracle_update_state_fn>           (const char*);
template jl_oracle_free_state_fn              JuliaMicroIterOracle::LoadSymbol<jl_oracle_free_state_fn>             (const char*);
template jl_oracle_deserialize_state_fn       JuliaMicroIterOracle::LoadSymbol<jl_oracle_deserialize_state_fn>      (const char*);
template jl_oracle_evaluate_fn                JuliaMicroIterOracle::LoadSymbol<jl_oracle_evaluate_fn>               (const char*);
template jl_oracle_free_constraint_keys_fn    JuliaMicroIterOracle::LoadSymbol<jl_oracle_free_constraint_keys_fn>   (const char*);
template jl_oracle_on_master_iteration_end_fn JuliaMicroIterOracle::LoadSymbol<jl_oracle_on_master_iteration_end_fn>(const char*);
