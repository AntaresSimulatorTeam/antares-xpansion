/*
 * BendersMicroIterPlugin.h
 *
 * Implements BendersPlugin by wrapping an IMicroIterationOracle.
 *
 * This class owns all C++ infrastructure concerns for the micro-iteration
 * lazy-constraint workflow:
 *
 *   - MPI state broadcast: calls oracle->UpdateStateFromMaster on rank 0,
 *     broadcasts the serialized bytes to all ranks via boost::mpi, then
 *     calls oracle->DeserializeState on every rank (including rank 0).
 *
 *   - Constraints dictionary: reads constraints_dictionary.csv at construction,
 *     mapping each constraint key to one or more MPS row names.
 *
 *   - ConstraintsReader map: builds one ConstraintsReader per local subproblem
 *     in OnBendersStart (once the subproblem workers are available).
 *
 *   - Deduplication: tracks which constraint keys have been added for each
 *     subproblem in the current master iteration; prevents re-adding active
 *     constraints.
 *
 *   - Warm-start: in warm-start mode, added rows persist across master
 *     iterations and the deduplication set is also preserved. In cold-start
 *     mode, rows and the deduplication set are cleared at each master
 *     iteration end.
 *
 * This class does NOT know about:
 *   - Business-domain concepts (HVDC, PTDF, investment decisions, flow variables)
 *   - Julia, Python, or any specific oracle implementation technology
 *   - How the oracle serializes its internal state
 */

#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/logger/MicroIterationsLog.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/benders/plugins/IMicroIterationOracle.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"


class BendersMicroIterPlugin : public BendersPlugin
{
public:
    /*
     * Constructor.
     *
     * @param oracle
     *   The lazy-constraint oracle (ownership transferred). Any implementation
     *   of IMicroIterationOracle is accepted, enabling mock injection for tests.
     *
     * @param options
     *   Full simulation options. Required by CouplingMapGenerator and
     *   MicroIterationsLog. Stored by reference; must outlive this object.
     *
     * @param coupling_map
     *   Coupling map (subproblem name -> variable name -> column id).
     *   Used to build the SubProblemConstraintMap via CouplingMapGenerator.
     *
     * @param world
     *   Boost MPI communicator (non-owning pointer; must outlive this object).
     *
     * @param oracle_config_dir
     *   Directory passed verbatim to oracle->Initialize. C++ treats it as
     *   opaque; only the oracle reads its contents.
     *
     * @param warm_start
     *   If true, rows added during micro-iterations persist across master
     *   iterations (warm-start mode). If false, rows are deleted at each
     *   OnBendersMasterResolutionEnd call.
     *
     * @param constraints_dict_path
     *   Path to constraints_dictionary.csv.
     *   Format per line: key,mps_row_name_1,mps_row_name_2,...
     *   Read at construction time.
     */
    BendersMicroIterPlugin(
        std::unique_ptr<IMicroIterationOracle> oracle,
        const SimulationOptions&               options,
        const CouplingMap&                     coupling_map,
        mpi::communicator*                     world,
        std::filesystem::path                  oracle_config_dir,
        bool                                   warm_start,
        std::filesystem::path                  constraints_dict_path);

    virtual ~BendersMicroIterPlugin() = default;

    /*
     * Build ConstraintsReader objects for each local subproblem, then call
     * oracle->Initialize with the list of subproblem IDs and master variable
     * names (derived from coupling_map).
     */
    void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                        const Logger&             logger,
                        const BendersBaseOptions& options,
                        const SolverLogManager&   solver_log_manager) override;

    /*
     * Call oracle->Finalize.
     */
    void OnBendersEnd() override;

    void OnBendersIterationStart() override;
    void OnBendersIterationEnd() override;
    void OnBendersSubResolutionStart() override;

    /*
     * Log the total number of micro-iterations performed for sub_name.
     */
    void OnBendersSubResolutionEnd(std::string sub_name, int num_micro_iter) override;

    /*
     * Orchestrate MPI state broadcast for this master iteration:
     *   1. Reset deduplication state (if !warm_start_).
     *   2. Rank 0: call oracle->UpdateStateFromMaster(x^t) -> bytes.
     *   3. mpi::broadcast(*world_, bytes, 0).
     *   4. All ranks: call oracle->DeserializeState(bytes).
     *   5. Log state-update elapsed time (rank 0, LOG_LEVEL >= 2).
     */
    void OnBendersMasterResolutionStart(
        std::map<std::string, double>& master_out,
        int& num_iter) override;

    /*
     * Clean up after all subproblems for this master iteration:
     *   1. If !warm_start_: call ConstraintsReader::delete_added_rows() for
     *      each local subproblem and clear added_constraints_per_sub_.
     *   2. Call oracle->OnMasterIterationEnd(current_iteration_).
     */
    void OnBendersMasterResolutionEnd() override;

    /*
     * No-op placeholder; available for future pre-loop setup.
     */
    void OnBendersMicroIterationStart() override;

    /*
     * Core micro-iteration callback. For sub_name:
     *   1. Read current subproblem primal solution via ConstraintsReader.
     *   2. Build DecisionPoint from full solution vector and variable name map.
     *   3. Call oracle->EvaluateViolatedConstraints(sub_name, sub_solution).
     *   4. For each returned constraint key:
     *      a. Skip if already in added_constraints_per_sub_[sub_name].
     *      b. Look up MPS row name(s) in constraints_dict_.
     *      c. Call ConstraintsReader::add_rows(row_name) for each.
     *      d. Record key in added_constraints_per_sub_[sub_name].
     *   5. Set added_rows = (number of new rows added > 0).
     *   6. Log micro-iteration data (LOG_LEVEL >= 2).
     *
     * @param sub_name    Subproblem identifier (matches coupling_map key).
     * @param added_rows  [out] True if new constraints were added; controls
     *                    whether the micro-iteration loop continues.
     * @param solve_time  Elapsed solve time string (for logging).
     */
    void OnBendersMicroIterationEnd(
        std::string  sub_name,
        bool&        added_rows,
        std::string  solve_time) override;

private:
    /*
     * Build a ConstraintsReader for each subproblem in subproblem_map and
     * populate constraints_map_ and added_constraints_per_sub_.
     */
    void BuildConstraintsReaderMap(
        const SubproblemsMapPtr& subproblem_map,
        const BendersBaseOptions& options,
        const SolverLogManager&   solver_log_manager);

    /*
     * Parse constraints_dict_path_ into constraints_dict_.
     * Format per CSV line: key,row_name_1,row_name_2,...
     */
    void ReadConstraintsDictionary();

    /*
     * Build a DecisionPoint from a flat solution vector and the variable
     * name -> column index map exposed by SubproblemWorker.
     * All variables in the subproblem are included.
     */
    static DecisionPoint BuildDecisionPoint(
        const std::vector<double>& solution,
        const VariableMap&         name_to_id);

    std::unique_ptr<IMicroIterationOracle> oracle_;
    const SimulationOptions&               options_;
    CouplingMap                            coupling_map_;
    mpi::communicator*                     world_;
    std::filesystem::path                  oracle_config_dir_;
    bool                                   warm_start_;
    std::filesystem::path                  constraints_dict_path_;

    // Subproblem name -> ConstraintsReader
    // Key is the constraint file base name (from SubProblemConstraintMap).
    ConstraintsReaderPtrMap                constraints_map_;

    // Subproblem name -> constraint file base name
    // Built by CouplingMapGenerator::BuildSubProblemConstaintMap.
    SubProblemConstraintMap                subproblem_constraint_map_;

    // constraints_dictionary.csv: constraint key -> list of MPS row names
    std::map<std::string, std::vector<std::string>> constraints_dict_;

    // Per-subproblem set of constraint keys currently active in the LP.
    // In warm-start mode, persists across master iterations.
    // In cold-start mode, cleared at each OnBendersMasterResolutionEnd.
    std::map<std::string, std::vector<std::string>> added_constraints_per_sub_;

    // Current master iteration number (set in OnBendersMasterResolutionStart).
    int current_iteration_ = 0;

    Logger                              logger_;
    std::shared_ptr<MicroIterationsLog> micro_iterations_logger_;
};
