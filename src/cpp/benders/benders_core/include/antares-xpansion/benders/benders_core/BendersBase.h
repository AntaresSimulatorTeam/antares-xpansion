#pragma once

#include <filesystem>
#include <functional>
#include <mutex>
#include <tbb/tbb.h>

#include "BendersMathLogger.h"
#include "BendersStructsDatas.h"
#include "CriterionComputation.h"
#include "ExecutionPolicy.h"
#include "IBendersPlugin.h"
#include "ICommunicationStrategy.h"
#include "SubproblemBasisCache.h"
#include "SubproblemCut.h"
#include "SubproblemWorker.h"
#include "SubproblemWorkerFactory.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common.h"

class BendersBase
{
public:
    virtual ~BendersBase() = default;
    BendersBase(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                std::shared_ptr<ICommunicationStrategy> communication_strategy = nullptr);
    virtual void launch() = 0;
    void set_solver_log_file(const std::filesystem::path& log_file);
    void SetPlugin(std::shared_ptr<IBendersPlugin> benders_plugin);
    double execution_time() const;
    virtual std::string BendersName() const = 0;
    LogData GetBestIterationData() const;
    void set_input_map(const CouplingMap& coupling_map);
    int MasterRowIndex(const std::string& row_name) const;
    void MasterChangeRhs(int id_row, double val) const;
    void MasterGetRhs(double& rhs, int id_row) const;
    void GetCompactInMemCuts(SubProblemDataMap& subproblem_data_map);

    const VariableMap& MasterVariables() const
    {
        return master_variable_map_;
    }

    std::vector<double> MasterObjectiveFunctionCoeffs() const;
    void MasterRowsCoeffs(std::vector<int>& mstart,
                          std::vector<int>& mclind,
                          std::vector<double>& dmatval,
                          int size,
                          std::vector<int>& nels,
                          int first,
                          int last) const;
    int MasterGetNElems() const;
    void MasterAddRows(const std::vector<char>& qrtype_p,
                       const std::vector<double>& rhs_p,
                       const std::vector<double>& range_p,
                       const std::vector<int>& mstart_p,
                       const std::vector<int>& mclind_p,
                       const std::vector<double>& dmatval_p,
                       const std::vector<std::string>& row_names = {}) const;
    void MasterGetRowType(std::vector<char>& qrtype, int first, int last) const;
    void ResetMasterFromLastIteration();
    std::filesystem::path LastMasterPath() const;
    bool MasterIsEmpty() const;

    void DoFreeProblems(bool free_problems)
    {
        free_problems_ = free_problems;
    }

    int MasterGetnrows() const;
    int MasterGetncols() const;
    WorkerMasterData BestIterationWorkerMaster() const;
    void SetMasterObjectiveFunctionCoeffsToZeros() const;
    void SetMasterObjectiveFunction(const double* coeffs, int first, int last) const;
    virtual void InitializeProblems() = 0;

    void SetMaxIteration(int max_iteration)
    {
        _options.MAX_ITERATIONS = max_iteration;
    }

    BendersBaseOptions Options() const
    {
        return _options;
    }

    virtual void free() = 0;

    int GetBendersRunNumber() const
    {
        return _data.criteria_current_iteration_data.benders_num_run;
    }

    void IncrementBendersRunNumber()
    {
        ++_data.criteria_current_iteration_data.benders_num_run;
    }

    CurrentIterationData GetCurrentIterationData() const;

    CriteriaCurrentIterationData GetOuterLoopData() const;

    std::vector<double> GetOuterLoopCriterionAtBestBenders() const;
    virtual void init_data();
    void init_data(double external_loop_lambda,
                   double external_loop_lambda_min,
                   double external_loop_lambda_max);
    Output::SolutionData GetOuterLoopSolution() const;
    void SaveOuterLoopSolutionInOutputFile() const;
    void SaveCurrentOuterLoopIterationInOutputFile() const;
    void SetBilevelBestub(double bilevel_best_ub);
    void UpdateOuterLoopSolution();

    bool isExceptionRaised() const;
    void UpdateOverallCosts();
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver_;
    std::once_flag variable_indice_once_flag;
    void setCriterionComputationInputs(
      const Benders::Criterion::CriterionInputData& criterion_input_data);

    [[nodiscard]] std::shared_ptr<ICommunicationStrategy> GetCommunicationStrategy() const
    {
        return communication_strategy_;
    }

protected:
    bool exception_raised_ = false;
    CurrentIterationData _data;
    WorkerMasterPtr _master;
    std::shared_ptr<IBendersPlugin> benders_plugin_;
    VariableMap master_variable_map_;
    CouplingMap coupling_map_;
    VariableMap _problem_to_id;
    std::shared_ptr<SubproblemWorkerFactory> subproblem_worker_factory_;
    BendersRelevantIterationsData relevantIterationData_ = {WorkerMasterData(), WorkerMasterData()};
    bool init_data_ = true;
    bool init_problems_ = true;
    bool free_problems_ = true;
    BendersBaseOptions _options;

    std::vector<std::vector<double>> criteria_vector_for_each_iteration_;
    bool is_bilevel_check_all_ = false;

    virtual void Run() = 0;
    void update_best_ub();
    bool ShouldBendersStop();
    bool is_initial_relaxation_requested() const;
    bool SwitchToIntegerMaster(bool is_relaxed) const;
    virtual void HandleInitialMasterRelaxation();
    virtual void UpdateTrace();
    virtual void ComputeXCut();
    void roundXCut();
    void ComputeInvestCost();
    virtual void compute_ub();
    virtual void get_master_value();
    void GetSubproblemCut(SubProblemDataMap& subproblem_data_map);
    void GetSubproblemCutFast(SubProblemDataMap& subproblem_data_map);
    std::shared_ptr<SubproblemWorker> makeSubproblemWorker(
      const std::pair<std::string, VariableMap>& kvp) const;
    void StoreSubproblemBasis(const std::string& name,
                              const std::shared_ptr<SubproblemWorker>& worker);
    void TryRestoreSubproblemBasis(const std::string& name,
                                   const std::shared_ptr<SubproblemWorker>& worker);
    void GetSubproblemCutCache(SubProblemDataMap& subproblem_data_map);
    virtual void post_run_actions() const;
    void BuildCutFull(const SubProblemDataMap& subproblem_data_map);
    virtual void DeactivateIntegrityConstraints() const;
    virtual void ActivateIntegrityConstraints() const;
    virtual void SetDataPreRelaxation();
    virtual void ResetDataPostRelaxation();
    void set_rank(int rank);
    [[nodiscard]] std::filesystem::path GetSubproblemPath(const std::string& subproblem_name) const;
    [[nodiscard]] double SubproblemWeight(int subproblem_count, const std::string& name) const;
    [[nodiscard]] std::filesystem::path get_master_path() const;
    [[nodiscard]] std::filesystem::path get_structure_path() const;
    [[nodiscard]] LogData bendersDataToLogData(const CurrentIterationData& data) const;

    template<typename T, typename... Args>
    void reset_master(Args&&... args)
    {
        _master = std::make_shared<T>(std::forward<Args>(args)...);
        master_is_empty_ = false;
    }

    void free_master();
    void free_subproblems();
    void AddSubproblem(const std::pair<std::string, VariableMap>& kvp);
    [[nodiscard]] virtual WorkerMasterPtr get_master() const;
    void MatchProblemToId();
    void AddSubproblemName(const std::string& name);
    [[nodiscard]] std::string get_master_name() const;
    [[nodiscard]] std::string get_solver_name() const;
    [[nodiscard]] int get_log_level() const;
    [[nodiscard]] bool is_trace() const;
    [[nodiscard]] Point get_x_cut() const;
    void set_x_cut(const Point& x0);
    [[nodiscard]] Point get_x_out() const;
    void set_x_out(const Point& x0);
    [[nodiscard]] double GetSubproblemCost() const;
    void SetSubproblemCost(const double& subproblem_cost);
    bool IsResumeMode() const;

    std::filesystem::path LastIterationFile() const
    {
        return std::filesystem::path(_options.LAST_ITERATION_JSON_FILE);
    }

    void UpdateMaxNumberIterationResumeMode(int nb_iteration_done);
    void SaveCurrentIterationInOutputFile() const;
    void SaveSolutionInOutputFile() const;
    void PrintCurrentIterationCsv();
    void OpenCsvFile();
    void CloseCsvFile();
    void ChecksResumeMode();
    virtual void SaveCurrentBendersData();
    void ClearCurrentIterationCutTrace();
    virtual void EndWritingInOutputFile() const;

    [[nodiscard]] int GetNumIterationsBeforeRestart() const
    {
        return iterations_before_resume;
    }

    double GetBendersTime() const;
    virtual void write_basis() const;

    SubproblemsMapPtr GetSubProblemMap() const
    {
        return subproblem_map;
    }

    StrVector GetSubProblemNames() const
    {
        return subproblems;
    }

    double AbsoluteGap() const
    {
        return _options.ABSOLUTE_GAP;
    }

    double RelativeGap() const
    {
        return _options.RELATIVE_GAP;
    }

    double RelaxedGap() const
    {
        return _options.RELAXED_GAP;
    }

    DblVector GetAlpha_i() const
    {
        return _data.single_subpb_costs_under_approx;
    }

    void SetAlpha_i(const DblVector& single_subpb_costs_under_approx)
    {
        _data.single_subpb_costs_under_approx = single_subpb_costs_under_approx;
    }

    int ProblemToId(const std::string& problem_name) const
    {
        return _problem_to_id.at(problem_name);
    }

    virtual void UpdateStoppingCriterion();
    virtual bool ShouldRelaxationStop() const;

    int GetNumOfSubProblemsSolvedBeforeResume()
    {
        return cumulative_number_of_subproblem_resolved_before_resume;
    }

    void BoundSimplexIterations(int subproblem_iteration);
    void ResetSimplexIterationsBounds();

    SubproblemsMapPtr subproblem_map;
    SolverLogManager solver_log_manager_;

    virtual void SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                                 const std::string& name,
                                 const std::shared_ptr<SubproblemWorker>& worker,
                                 const std::function<void()>& post_reset_hook);
    void SetSubproblemVariablesIndices(const SubproblemWorker& subproblem);

    Benders::Criterion::CriterionComputation criterion_computation_;
    /**
     * for the nth variable name, Subproblems shares the same prefix , only the
     suffix is different
     * ex variable at index = 0 is named in:

    * subproblems-1-1  --> DirectFlow::link<area1$$area2>::hour<0>
                                      * subproblems-3-5  -->
    DirectFlow::link<area1$$area2>::hour<672>
     */
    // Search for variables in sub problems that satisfy patterns
    // var_indices is a vector(for each patterns p) of vector (var indices related
    // to p)
    void SetSubproblemsVariablesIndices();

    void build_all_aggregated_cuts(const std::vector<SubProblemNamesInCut>& subproblem_names,
                                   const std::vector<SubProblemDataMap>& gathered_subproblem_map);

    int SetAggregation(int max_aggregation) const;

    std::map<int, double> GetSubCutTolerance() const;

private:
    void print_master_and_cut(std::ostream& file,
                              int ite,
                              WorkerMasterData& trace,
                              const Point& xopt);
    void print_master_csv(std::ostream& stream,
                          const WorkerMasterData& trace,
                          const Point& xopt) const;
    void check_status(const SubProblemDataMap& subproblem_data_map) const;
    [[nodiscard]] LogData build_log_data_from_data() const;
    [[nodiscard]] Output::SolutionData solution() const;
    [[nodiscard]] Output::SolutionData BendersSolution() const;
    [[nodiscard]] std::string status_from_criterion() const;
    void compute_cut_aggregate(const SubProblemDataMap& subproblem_data_map);
    void compute_cut(const SubProblemDataMap& subproblem_data_map);
    [[nodiscard]] std::map<std::string, int> get_master_variable_map(
      const std::map<std::string, std::map<std::string, int>>& input_map) const;
    [[nodiscard]] virtual bool shouldParallelize() const;

    Output::Iteration iteration(const WorkerMasterData& masterDataPtr_l) const;
    LogData FinalLogData() const;
    void FillWorkerMasterData(WorkerMasterData& data) const;
    bool master_is_empty_ = true;
    int _totalNbProblems = 0;
    StrVector subproblems;
    std::ofstream _csv_file;
    std::filesystem::path _csv_file_path;
    LogData best_iteration_data;
    int iterations_before_resume = 0;
    int cumulative_number_of_subproblem_resolved_before_resume = 0;
    Timer benders_timer;
    Output::SolutionData outer_loop_solution_data_;
    SubproblemBasisCache subproblem_basis_cache_;
    std::shared_ptr<ICommunicationStrategy> communication_strategy_;

    int rank_;
};

using pBendersBase = std::shared_ptr<BendersBase>;
