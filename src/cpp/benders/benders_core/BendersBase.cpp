#include "antares-xpansion/benders/benders_core/BendersBase.h"

#include <memory>
#include <mutex>
#include <numeric>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/CustomVector.h"
#include "antares-xpansion/benders/benders_core/LastIterationPrinter.h"
#include "antares-xpansion/benders/benders_core/LastIterationReader.h"
#include "antares-xpansion/benders/benders_core/LastIterationWriter.h"
#include "antares-xpansion/helpers/solver_utils.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

BendersBase::BendersBase(BendersBaseOptions options,
                         Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                         std::shared_ptr<ICommunicationStrategy> communication_strategy):
    _logger(std::move(logger)),
    _writer(std::move(writer)),
    mathLoggerDriver_(std::move(mathLoggerDriver)),
    _options(std::move(options)),
    _csv_file_path(std::filesystem::path(_options.OUTPUTROOT) / (_options.CSV_NAME + ".csv")),
    communication_strategy_(std::move(communication_strategy))
{
}

bool BendersBase::shouldParallelize() const
{
    if (communication_strategy_)
    {
        return communication_strategy_->ShouldParallelize();
    }
    // Default when no strategy is provided (e.g., in test doubles):
    // use local TBB parallelism
    return true;
}

/*!
 *  \brief Initialize set of data used in the loop
 */
void BendersBase::init_data()
{
    _data.lb = relevantIterationData_.last._lb = -1e20;
    _data.ub = relevantIterationData_.last._ub = +1e20;
    _data.best_ub = relevantIterationData_.last._best_ub = +1e20;
    _data.stop = false;
    _data.it = 0;
    _data.overall_subpb_cost_under_approx = 0;
    _data.invest_cost = relevantIterationData_.last._invest_cost = 0;
    _data.best_it = 0;
    _data.stopping_criterion = StoppingCriterion::empty;
    _data.is_in_initial_relaxation = false;
    _data.cumulative_number_of_subproblem_solved = 0;
    relevantIterationData_.best = relevantIterationData_.last;
    _data.benders_time = 0;
    _data.iteration_time = 0;
    _data.timer_master = 0;
    _data.subproblems_walltime = 0;
    criteria_vector_for_each_iteration_.clear();
}

void BendersBase::OpenCsvFile()
{
    if (!_csv_file.is_open())
    {
        const auto opening_mode = _options.RESUME ? std::ios::app : std::ios::trunc;
        _csv_file.open(_csv_file_path, std::ios::out | opening_mode);
        if (_csv_file && !_options.RESUME)
        {
            _csv_file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;single_"
                         "subpb_costs_under_approx;"
                         "time;basis;"
                      << std::endl;
        }
        else
        {
            using namespace std::string_literals;
            _logger->display_message("Impossible to open the .csv file: "s
                                     + _csv_file_path.string());
        }
    }
}

void BendersBase::CloseCsvFile()
{
    if (_csv_file.is_open())
    {
        _csv_file.close();
    }
}

void BendersBase::PrintCurrentIterationCsv()
{
    if (relevantIterationData_.last._valid)
    {
        auto ite = _data.it - 1;
        Point x_cut;
        // Write first problem : use result of best iteration
        if (ite == 0)
        {
            int best_it_index = _data.best_it - 1;
            if (best_it_index >= 0)
            {
                x_cut = relevantIterationData_.best.get_x_cut();
            }
        }
        else
        {
            x_cut = relevantIterationData_.last.get_x_cut();
        }
        print_master_and_cut(_csv_file,
                             ite + 1 + iterations_before_resume,
                             relevantIterationData_.last,
                             x_cut);
    }
}

/*!
 *  \brief Print in a file subproblem's information
 *
 *  \param stream : output stream
 *
 *  \param handler : handler to manage subproblem data
 *
 *  \param name : problem name
 *
 *  \param subproblem_index : problem id
 */
void print_cut_csv(std::ostream& stream,
                   const PlainData::SubProblemData& subproblem_data,
                   const std::string& subproblem_name,
                   int subproblem_index,
                   double alpha_i)
{
    stream << "Subproblem" << ";";
    stream << subproblem_name << ";";
    stream << subproblem_index << ";";
    stream << subproblem_data.subproblem_cost << ";";
    stream << ";";
    stream << ";";
    stream << subproblem_data.simplex_iter << ";";
    stream << ";";
    stream << alpha_i << ";";
    stream << subproblem_data.subproblem_timer << ";";
    stream << ";";
    stream << std::endl;
}

void BendersBase::print_master_and_cut(std::ostream& file,
                                       int ite,
                                       WorkerMasterData& trace,
                                       const Point& x_cut)
{
    file << ite << ";";

    print_master_csv(file, trace, x_cut);

    for (auto& [subproblem_name, subproblem_data]: trace._cut_trace)
    {
        auto problem_id = _problem_to_id[subproblem_name];
        file << ite << ";";
        print_cut_csv(file,
                      subproblem_data,
                      subproblem_name,
                      problem_id,
                      _data.single_subpb_costs_under_approx[problem_id]);
    }
}

/*!
 *  \brief Print in a file master's information
 *
 *  \param stream : output stream
 *
 *  \param trace : storage of problem data
 *
 *  \param x_cut : cut point determined after the master resolution
 */
void BendersBase::print_master_csv(std::ostream& stream,
                                   const WorkerMasterData& trace,
                                   const Point& x_cut) const
{
    stream << "Master" << ";";
    stream << _options.MASTER_NAME << ";";
    stream << _data.nsubproblem << ";";
    stream << trace._ub << ";";
    stream << trace._lb << ";";
    stream << trace._best_ub << ";";
    stream << ";";
    stream << norm_point(x_cut, trace.get_x_cut()) << ";";
    stream << ";";
    stream << trace._master_duration << ";";
    stream << std::endl;
}

/*!
 *  \brief Update best upper bound and best optimal variables
 *
 *	Function to update best upper bound and best optimal variables regarding
 *the current ones
 */
void BendersBase::update_best_ub()
{
    if (_data.ub < _data.best_ub)
    {
        _data.x_in = _data.x_cut;
        _data.master_only_vars_in = _data.master_only_vars_cut;
        _data.best_ub = _data.ub;
        _data.best_it = _data.it;
        FillWorkerMasterData(relevantIterationData_.best);
        _data.criteria_current_iteration_data.max_criterion_best_it
          = _data.criteria_current_iteration_data.max_criterion;
        _data.criteria_current_iteration_data.max_criterion_area_best_it
          = _data.criteria_current_iteration_data.max_criterion_area;
        relevantIterationData_.best._cut_trace = relevantIterationData_.last._cut_trace;
        best_iteration_data = bendersDataToLogData(_data);
    }
}

/*!
 *  \brief Check if initial relaxation should stop
 */
bool BendersBase::ShouldRelaxationStop() const
{
    return (_data.stopping_criterion != StoppingCriterion::empty)
           || (((_data.best_ub - _data.lb) / _data.best_ub) <= _options.RELAXED_GAP);
}

/*!
 *  \brief Update stopping criterion
 *
 *  Method updating the stopping criterion and reinitializing some datas
 *
 */
void BendersBase::UpdateStoppingCriterion()
{
    if (_data.benders_time > _options.TIME_LIMIT)
    {
        _data.stopping_criterion = StoppingCriterion::timelimit;
    }
    else if ((_options.MAX_ITERATIONS != -1) && (_data.it >= _options.MAX_ITERATIONS))
    {
        _data.stopping_criterion = StoppingCriterion::max_iteration;
    }
    else if (_data.lb + _options.ABSOLUTE_GAP >= _data.best_ub)
    {
        _data.stopping_criterion = StoppingCriterion::absolute_gap;
    }
    // keep parentheses around (std::max) to prevent build failure on windows
    else if (((_data.best_ub - _data.lb) / (std::max)(std::abs(_data.best_ub), std::abs(_data.lb)))
             <= _options.RELATIVE_GAP)
    {
        _data.stopping_criterion = StoppingCriterion::relative_gap;
    }
}

bool BendersBase::ShouldBendersStop()
{
    UpdateStoppingCriterion();
    return (_data.stopping_criterion != StoppingCriterion::empty)
           && !_data.is_in_initial_relaxation;
}

void BendersBase::FillWorkerMasterData(WorkerMasterData& data) const
{
    data._lb = _data.lb;
    data._ub = _data.ub;
    data._best_ub = _data.best_ub;
    data._x_in = std::make_shared<Point>(_data.x_in);
    data._x_out = std::make_shared<Point>(_data.x_out);
    data._x_cut = std::make_shared<Point>(_data.x_cut);
    data._max_invest = std::make_shared<Point>(_data.max_invest);
    data._min_invest = std::make_shared<Point>(_data.min_invest);
    data._master_duration = _data.timer_master;
    data._subproblem_duration = _data.subproblems_walltime;
    data._invest_cost = _data.invest_cost;
    data._operational_cost = _data.subproblem_cost;
    data._valid = true;
}

/*!
 *  \brief Update trace of the Benders for the current iteration
 *
 *  Fonction to store the current Benders data in the trace
 */
void BendersBase::UpdateTrace()
{
    FillWorkerMasterData(relevantIterationData_.last);
    // TODO Outer loop --> de-comment for general case
    // workerMasterDataVect_.push_back(relevantIterationData_.last);
}

bool BendersBase::is_initial_relaxation_requested() const
{
    return (_options.MASTER_FORMULATION == MasterFormulation::INTEGER
            && _options.SEPARATION_PARAM < 1);
}

bool BendersBase::SwitchToIntegerMaster(bool is_relaxed) const
{
    return is_initial_relaxation_requested() && is_relaxed && ShouldRelaxationStop();
}

void BendersBase::SetDataPreRelaxation()
{
    _data.is_in_initial_relaxation = true;
}

void BendersBase::ResetDataPostRelaxation()
{
    _data.is_in_initial_relaxation = false;
    _data.best_ub = 1e+20;
    _data.best_it = 0;
    _data.stopping_criterion = StoppingCriterion::empty;
    _options.SEPARATION_PARAM = 1;
}

void BendersBase::HandleInitialMasterRelaxation()
{
    if (_options.MASTER_FORMULATION == MasterFormulation::RELAXED)
    {
        DeactivateIntegrityConstraints();
    }
    else if (is_initial_relaxation_requested())
    {
        // Case of integer master with separation parameter < 1, needs to register that we are in
        // initial relaxation state to be able to fallback to integer master at the end of the
        // algorithm
        _logger->LogAtInitialRelaxation();
        DeactivateIntegrityConstraints();
        SetDataPreRelaxation();
    }
}

/*!
 *  \brief Check if every subproblem has been solved to optimality
 *
 *  \param all_package : storage of each subproblems status
 *  \param data : BendersData used to get master solving status
 */
void BendersBase::check_status(const SubProblemDataMap& subproblem_data_map) const
{
    if (_data.master_status != SOLVER_STATUS::OPTIMAL)
    {
        std::ostringstream msg;
        auto log_location = LOGLOCATION;
        msg << "Master status is " + std::to_string(_data.master_status) << std::endl;
        _logger->display_message(log_location + msg.str());
        throw InvalidSolverStatusException(msg.str(), log_location);
    }
    for (const auto& [subproblem_name, subproblemData]: subproblem_data_map)
    {
        if (subproblemData.lpstatus != SOLVER_STATUS::OPTIMAL)
        {
            std::ostringstream stream;
            auto log_location = LOGLOCATION;
            stream << "Subproblem " << subproblem_name << " status is " << subproblemData.lpstatus
                   << std::endl;
            _logger->display_message(log_location + stream.str());
            throw InvalidSolverStatusException(stream.str(), log_location);
        }
    }
}

/*!
 *  \brief Solve and get optimal variables of the Master Problem
 *
 *  Method to solve and get optimal variables of the Master Problem and update
 * upper and lower bound
 *
 */
void BendersBase::get_master_value()
{
    Timer timer_master;

    _data.single_subpb_costs_under_approx.resize(_data.nsubproblem);
    _data.master_only_vars_out.resize(_master->_id_master_only_vars.size());
    if (_options.BOUND_ALPHA)
    {
        _master->fix_alpha(_data.best_ub);
    }
    _master->solve(_data.master_status,
                   _options.OUTPUTROOT,
                   _options.LAST_MASTER_MPS + MPS_SUFFIX,
                   _writer);

    _master->get(_data.x_out,
                 _data.overall_subpb_cost_under_approx,
                 _data.single_subpb_costs_under_approx,
                 _data.master_only_vars_out); /*Get the optimal variables of the
                                                            Master Problem*/
    _master->get_value(_data.lb);             /*Get the optimal value of the Master Problem*/

    for (const auto& pairIdName: _master->_id_to_name)
    {
        _master->_solver->get_ub(&_data.max_invest[pairIdName.second],
                                 pairIdName.first,
                                 pairIdName.first);
        _master->_solver->get_lb(&_data.min_invest[pairIdName.second],
                                 pairIdName.first,
                                 pairIdName.first);
    }

    _data.timer_master = timer_master.elapsed();
}

void BendersBase::DeactivateIntegrityConstraints() const
{
    _master->DeactivateIntegrityConstraints();
}

void BendersBase::ActivateIntegrityConstraints() const
{
    _master->ActivateIntegrityConstraints();
}

void BendersBase::ComputeXCut()
{
    if (_data.it == 1)
    {
        _data.x_in = _data.x_out;
        _data.x_cut = _data.x_out;
        _data.master_only_vars_in = _data.master_only_vars_out;
        _data.master_only_vars_cut = _data.master_only_vars_out;
    }
    else
    {
        for (const auto& [name, value]: _data.x_out)
        {
            _data.x_cut[name] = _options.SEPARATION_PARAM * _data.x_out[name]
                                + (1 - _options.SEPARATION_PARAM) * _data.x_in[name];
        }
        for (int i(0); i < _data.master_only_vars_out.size(); ++i)
        {
            _data.master_only_vars_cut[i] = Options().SEPARATION_PARAM
                                              * _data.master_only_vars_out[i]
                                            + (1 - Options().SEPARATION_PARAM)
                                                * _data.master_only_vars_in[i];
        }
    }
    roundXCut();
}

void BendersBase::ComputeInvestCost()
{
    _data.invest_cost = 0;

    std::vector<double> obj(MasterObjectiveFunctionCoeffs());

    for (const auto& [col_name, value]: _data.x_cut)
    {
        int col_id = _master->_name_to_id[col_name];
        _data.invest_cost += obj[col_id] * _data.x_cut[col_name];
    }
    for (int i(0); i < _data.master_only_vars_cut.size(); ++i)
    {
        int col_id = _master->_id_master_only_vars[i];
        _data.invest_cost += obj[col_id] * _data.master_only_vars_cut[i];
    }
}

void BendersBase::compute_ub()
{
    ComputeInvestCost();
    _data.ub += _data.invest_cost;
}

/*!
 *  \brief Solve and store optimal variables of all Subproblem Problems
 *
 *  Method to solve and store optimal variables of all Subproblem Problems
 * after fixing trial values
 *
 *  \param subproblem_cut_package : map storing for each subproblem its cut
 */
void BendersBase::GetSubproblemCut(SubProblemDataMap& subproblem_data_map)
{
    if (Options().CACHE_PROBLEMS)
    {
        GetSubproblemCutCache(subproblem_data_map);
    }
    else
    {
        GetSubproblemCutFast(subproblem_data_map);
    }
}

void BendersBase::GetSubproblemCutFast(SubProblemDataMap& subproblem_data_map)
{
    // With gcc9 there was no parallelisation when iterating on the map directly
    // so with project it in a vector
    std::vector<std::pair<std::string, SubproblemWorkerPtr>> nameAndWorkers;
    nameAndWorkers.reserve(subproblem_map.size());
    for (const auto& [name, worker]: subproblem_map)
    {
        nameAndWorkers.emplace_back(name, worker);
    }

    std::mutex m;
    selectPolicy(
      [this, &nameAndWorkers, &m, &subproblem_data_map](auto& policy)
      {
          std::for_each(policy,
                        nameAndWorkers.begin(),
                        nameAndWorkers.end(),
                        [this, &m, &subproblem_data_map](
                          const std::pair<std::string, SubproblemWorkerPtr>& kvp)
                        {
                            PlainData::SubProblemData subproblem_data;
                            const auto& [name, worker] = kvp;
                            SolveSubproblem(subproblem_data, name, worker);

                            std::lock_guard guard(m);
                            subproblem_data_map[name] = subproblem_data;
                        });
      },
      shouldParallelize());
}

namespace
{
template<class T>
std::vector<std::pair<std::string, T&>> mapAsVectorOfPair(std::map<std::string, T>& map_to_flatten)
{
    std::vector<std::pair<std::string, T&>> flatten_result;
    flatten_result.reserve(map_to_flatten.size());
    std::ranges::for_each(map_to_flatten,
                          [&flatten_result](auto& pair)
                          { flatten_result.emplace_back(pair.first, pair.second); });
    return flatten_result;
}
} // namespace

std::pair<std::vector<int>, std::vector<int>> BendersBase::GetProblemBasis(
  const std::shared_ptr<SubproblemWorker>& worker) const
{
    int row_number = worker->_solver->get_nrows();
    int col_number = worker->_solver->get_ncols();
    std::vector<int> rstatus(row_number);
    std::vector<int> cstatus(col_number);
    worker->_solver->get_basis(rstatus.data(), cstatus.data());
    return {std::move(rstatus), std::move(cstatus)};
}

/**
 * Create a worker on a problem and reuse the basis to speed up solving
 *
 * @param kvp Problem data
 * @param name Name of the problem
 * @return Worker on the problem
 *
 */
std::shared_ptr<SubproblemWorker> BendersBase::BuildProblem(
  const std::pair<std::string, VariableMap>& kvp,
  const std::string& name)
{
    auto worker = makeSubproblemWorker(kvp);
    if (basiss_.contains(name))
    {
        auto& [rstatus, cstatus] = basiss_[name];
        worker->_solver->set_basis(rstatus, cstatus);
    }
    return worker;
}

std::shared_ptr<SubproblemWorker> BendersBase::makeSubproblemWorker(
  const std::pair<std::string, VariableMap>& kvp) const
{
    std::shared_ptr<IBendersProblemProvider>
      benders_problem_provider = std::make_shared<BendersProblemFromFile>(
        GetSubproblemPath(kvp.first));
    return std::make_shared<SubproblemWorker>(kvp.second,
                                              SubproblemWeight(_data.nsubproblem, kvp.first),
                                              Options().SOLVER_NAME,
                                              Options().LOG_LEVEL,
                                              solver_log_manager_,
                                              _logger,
                                              _options.PROBLEMS_FORMAT,
                                              benders_problem_provider.get(),
                                              _options.CUT_COEFFICIENT_TOLERANCE);
}

void BendersBase::SetBasisForSubproblem(const std::string& name,
                                        const std::vector<int>& rstatus,
                                        const std::vector<int>& cstatus)
{
    basiss_[name] = std::make_pair(rstatus, cstatus);
}

void BendersBase::GetSubproblemCutCache(SubProblemDataMap& subproblem_data_map)
{
    auto&& nameAndVariableMap = mapAsVectorOfPair(coupling_map_);
    std::mutex m;
    selectPolicy(
      [this, &nameAndVariableMap, &m, &subproblem_data_map](auto& policy)
      {
          std::for_each(policy,
                        nameAndVariableMap.begin(),
                        nameAndVariableMap.end(),
                        [this, &m, &subproblem_data_map](
                          const std::pair<std::string, VariableMap>& kvp)
                        {
                            const auto& [name, variables] = kvp;
                            std::shared_ptr<SubproblemWorker> worker = BuildProblem(kvp, name);
                            PlainData::SubProblemData subproblem_data;
                            SolveSubproblem(subproblem_data, name, worker);
                            auto [rstatus, cstatus] = GetProblemBasis(worker);
                            std::lock_guard guard(m);
                            subproblem_data_map[name] = subproblem_data;
                            SetBasisForSubproblem(name, rstatus, cstatus);
                            std::call_once(
                              variable_indice_once_flag,
                              [&](const auto& worker_) { SetSubproblemVariablesIndices(worker_); },
                              *worker);
                        });
      },
      shouldParallelize());
}

void BendersBase::SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                                  const std::string& name,
                                  const std::shared_ptr<SubproblemWorker>& worker)
{
    Timer subproblem_timer;
    worker->fix_to(_data.x_cut);

    benders_plugin_->OnBendersMicroIterationStart();

    worker->solve(subproblem_data.lpstatus,
                  _options.OUTPUTROOT,
                  _options.LAST_MASTER_MPS + MPS_SUFFIX,
                  _writer);
    worker->get_value(subproblem_data.subproblem_cost);
    worker->get_subgradient(subproblem_data.var_name_and_subgradient);
    worker->get_splex_num_of_ite_last(subproblem_data.simplex_iter);

    benders_plugin_->OnBendersMicroIterationEnd();

    subproblem_data.subproblem_timer = subproblem_timer.elapsed();

    std::vector<double> solution = worker->get_solution();
    criterion_computation_.ComputeCriterion(SubproblemWeight(_data.nsubproblem, name),
                                            solution,
                                            subproblem_data.criteria,
                                            subproblem_data.patterns_values);
}

void BendersBase::SetSubproblemVariablesIndices(const SubproblemWorker& subproblem)
{
    auto&& col_names = subproblem._solver->get_col_names();
    criterion_computation_.SearchVariables(col_names);
}

// Search for variables in sub problems that satisfy patterns
// var_indices is a vector(for each patterns p) of vector (var indices related
// to p)
void BendersBase::SetSubproblemsVariablesIndices()
{
    if (!subproblem_map.empty())
    {
        auto subproblem = subproblem_map.begin();
        SetSubproblemVariablesIndices(*subproblem->second);
    }
}

void compute_cut_val(const Point& var_name_subgradient, const Point& x_cut, Point& s)
{
    for (const auto& [cand_name, cand_value]: x_cut)
    {
        const auto cand_name_and_subgradient = var_name_subgradient.find(cand_name);
        if (cand_name_and_subgradient != var_name_subgradient.end())
        {
            s[cand_name] += cand_name_and_subgradient->second;
        }
    }
}

/*!
 *  \brief Add aggregated cut to Master Problem and store it in a set
 *
 *  Method to add aggregated cut from subproblems to Master Problem and store
 * it in a map linking each subproblem to its set of non-aggregated cut
 *
 *  \param subproblem_data_map : map storing all cuts information for each
 * subproblem
 */
void BendersBase::compute_cut_aggregate(const SubProblemDataMap& subproblem_data_map)
{
    Point s;
    double rhs(0);
    for (const auto& [name, subproblem_data]: subproblem_data_map)
    {
        _data.ub += subproblem_data.subproblem_cost;
        rhs += subproblem_data.subproblem_cost;

        compute_cut_val(subproblem_data.var_name_and_subgradient, _data.x_cut, s);

        relevantIterationData_.last._cut_trace[name] = subproblem_data;
    }
    _master->add_cut(s, _data.x_cut, rhs);
}

void BendersBase::build_all_aggregated_cuts(
  const std::vector<SubProblemNamesInCut>& subproblem_names,
  const std::vector<SubProblemDataMap>& gathered_subproblem_map)
{
    std::vector<int> subproblem_ids_per_cut;
    for (const auto& subproblem_names_in_cut: subproblem_names)
    {
        Point s;
        double rhs{0};
        std::vector<int> subproblem_ids_per_cut;

        for (const auto& [sub_problem_name, position_in_gathered]: subproblem_names_in_cut)
        {
            subproblem_ids_per_cut.push_back(_problem_to_id[sub_problem_name]);

            auto subproblem_data_pair = gathered_subproblem_map[position_in_gathered].find(
              sub_problem_name);

            if (subproblem_data_pair != gathered_subproblem_map[position_in_gathered].end())
            {
                auto& subproblem_data = subproblem_data_pair->second;
                _data.ub += subproblem_data.subproblem_cost;
                rhs += subproblem_data.subproblem_cost;
                compute_cut_val(subproblem_data.var_name_and_subgradient, _data.x_cut, s);
                relevantIterationData_.last._cut_trace[sub_problem_name] = subproblem_data;
            }
        }

        _master->addGroupSubproblemCut(subproblem_ids_per_cut, s, _data.x_cut, rhs);
    }
}

/*!
 *  \brief Add cut to Master Problem and store the cut in a set
 *
 *  Method to add cut from a subproblem to the Master Problem and store this
 * cut in a map linking each subproblem to its set of cuts.
 *
 *  \param all_package : vector storing all cuts information for each
 * subproblem problem
 *
 */
void BendersBase::compute_cut(const SubProblemDataMap& subproblem_data_map)
{
    // current_outer_loop_criterion_ = 0.0;
    for (const auto& [subproblem_name, subproblem_data]: subproblem_data_map)
    {
        _data.ub += subproblem_data.subproblem_cost;

        _master->addSubproblemCut(_problem_to_id[subproblem_name],
                                  subproblem_data.var_name_and_subgradient,
                                  _data.x_cut,
                                  subproblem_data.subproblem_cost);

        relevantIterationData_.last._cut_trace[subproblem_name] = subproblem_data;
    }
}

int BendersBase::SetAggregation(int max_aggregation) const
{
    if (max_aggregation < _options.NB_CUTS_PER_ITER)
    {
        std::string logging_str = "NB_CUTS_PER_ITER : " + std::to_string(_options.NB_CUTS_PER_ITER)
                                  + " is larger than the number of subproblems solved at this "
                                    "iteration : "
                                  + std::to_string(max_aggregation) + "setting NB_CUTS_PER_ITER to "
                                  + std::to_string(max_aggregation);
        _logger->display_message(logging_str);
        return max_aggregation;
    }
    else if (_options.NB_CUTS_PER_ITER <= 0)
    {
        std::string logging_str = "NB_CUTS_PER_ITER is <= 0. By default it will be equal to : "
                                  + std::to_string(max_aggregation);
        _logger->display_message(logging_str);
        return max_aggregation;
    }
    return _options.NB_CUTS_PER_ITER;
}

/*!
 *  \brief Add cuts in master problem
 *
 *  \param subproblem_data_map : storage of every subproblem information
 */
void BendersBase::BuildCutFull(const SubProblemDataMap& subproblem_data_map)
{
    check_status(subproblem_data_map);
    if (_options.NB_CUTS_PER_ITER)
    {
        compute_cut_aggregate(subproblem_data_map);
    }
    else
    {
        compute_cut(subproblem_data_map);
    }
}

LogData BendersBase::build_log_data_from_data() const
{
    auto logData = FinalLogData();
    logData.optimality_gap = _options.ABSOLUTE_GAP;
    logData.relative_gap = _options.RELATIVE_GAP;
    logData.max_iterations = _options.MAX_ITERATIONS;
    return logData;
}

LogData BendersBase::FinalLogData() const
{
    LogData result;
    result.it = _data.it + iterations_before_resume;
    result.best_it = _data.best_it + iterations_before_resume;

    result.subproblem_cost = best_iteration_data.subproblem_cost;
    result.invest_cost = best_iteration_data.invest_cost;
    result.cumulative_number_of_subproblem_resolved
      = _data.cumulative_number_of_subproblem_solved
        + cumulative_number_of_subproblem_resolved_before_resume;

    return result;
}

void BendersBase::post_run_actions() const
{
    LogData logData = build_log_data_from_data();

    _logger->log_stop_criterion_reached(_data.stopping_criterion);
    _logger->log_at_ending(logData);
}

void BendersBase::SaveCurrentIterationInOutputFile() const
{
    if (!_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP)
    {
        auto& LastWorkerMasterData = relevantIterationData_.last;
        if (LastWorkerMasterData._valid)
        {
            _writer->write_iteration(iteration(LastWorkerMasterData),
                                     _data.it + iterations_before_resume);
            _writer->dump();
        }
    }
}

void BendersBase::SaveCurrentOuterLoopIterationInOutputFile() const
{
    auto& LastWorkerMasterData = relevantIterationData_.last;
    if (LastWorkerMasterData._valid)
    {
        _writer->write_iteration(iteration(LastWorkerMasterData),
                                 _data.criteria_current_iteration_data.benders_num_run);
        _writer->dump();
    }
}

void BendersBase::SaveSolutionInOutputFile() const
{
    _writer->write_solution(solution());
    _writer->dump();
}

void BendersBase::SaveOuterLoopSolutionInOutputFile() const
{
    _writer->write_solution(GetOuterLoopSolution());
    _writer->dump();
}

Output::CandidatesVec candidates_data(const WorkerMasterData& masterDataPtr_l)
{
    Output::CandidatesVec candidates_vec;
    for (const auto& [cand_name, cand_value]: masterDataPtr_l.get_x_cut())
    {
        Output::CandidateData candidate_data;
        candidate_data.name = cand_name;
        candidate_data.invest = cand_value;
        candidate_data.min = masterDataPtr_l.get_min_invest()[cand_name];
        candidate_data.max = masterDataPtr_l.get_max_invest()[cand_name];
        candidates_vec.push_back(candidate_data);
    }

    return candidates_vec;
}

Output::Iteration BendersBase::iteration(const WorkerMasterData& masterDataPtr_l) const
{
    Output::Iteration iteration;
    iteration.master_duration = masterDataPtr_l._master_duration;
    iteration.subproblem_duration = masterDataPtr_l._subproblem_duration;
    iteration.lb = masterDataPtr_l._lb;
    iteration.ub = masterDataPtr_l._ub;
    iteration.best_ub = masterDataPtr_l._best_ub;
    iteration.optimality_gap = masterDataPtr_l._best_ub - masterDataPtr_l._lb;
    iteration.relative_gap = (masterDataPtr_l._best_ub - masterDataPtr_l._lb)
                             / masterDataPtr_l._best_ub;
    iteration.investment_cost = masterDataPtr_l._invest_cost;
    iteration.operational_cost = masterDataPtr_l._operational_cost;
    iteration.overall_cost = masterDataPtr_l._invest_cost + masterDataPtr_l._operational_cost;
    iteration.candidates = candidates_data(masterDataPtr_l);
    iteration.cumulative_number_of_subproblem_resolved
      = _data.cumulative_number_of_subproblem_solved
        + cumulative_number_of_subproblem_resolved_before_resume;
    return iteration;
}

Output::SolutionData BendersBase::solution() const
{
    auto solution_data = BendersSolution();
    solution_data.best_it = _data.best_it + iterations_before_resume;

    return solution_data;
}

void BendersBase::UpdateOuterLoopSolution()
{
    outer_loop_solution_data_ = BendersSolution();
    outer_loop_solution_data_.best_it = _data.criteria_current_iteration_data.benders_num_run;
}

Output::SolutionData BendersBase::GetOuterLoopSolution() const
{
    return outer_loop_solution_data_;
}

Output::SolutionData BendersBase::BendersSolution() const
{
    Output::SolutionData solution_data;
    solution_data.nbWeeks_p = _totalNbProblems;
    solution_data.problem_status = status_from_criterion();
    const auto optimal_gap(_data.best_ub - _data.lb);
    const auto relative_gap(optimal_gap / _data.best_ub);

    if (IsResumeMode())
    {
        // solution may not be in relevantIterationData_
        Output::CandidatesVec candidates_vec;
        std::transform(best_iteration_data.x_cut.cbegin(),
                       best_iteration_data.x_cut.cend(),
                       std::back_inserter(candidates_vec),
                       [this](
                         const std::pair<std::string, double>& name_invest) -> Output::CandidateData
                       {
                           const auto& [name, invest] = name_invest;
                           return {name,
                                   invest,
                                   best_iteration_data.min_invest.at(name),
                                   best_iteration_data.max_invest.at(name)};
                       });
        solution_data.solution = {best_iteration_data.master_time,
                                  best_iteration_data.subproblem_time,
                                  best_iteration_data.lb,
                                  best_iteration_data.ub,
                                  best_iteration_data.best_ub,
                                  optimal_gap,
                                  relative_gap,
                                  best_iteration_data.invest_cost,
                                  best_iteration_data.subproblem_cost,
                                  best_iteration_data.invest_cost
                                    + best_iteration_data.subproblem_cost,
                                  candidates_vec,
                                  0};
    }
    else
    {
        const auto& best_iteration_worker_master_data = relevantIterationData_.best;
        solution_data.solution = iteration(best_iteration_worker_master_data);
        solution_data.solution.optimality_gap = optimal_gap;
        solution_data.solution.relative_gap = relative_gap;
    }
    solution_data.stopping_criterion = criterion_to_str(_data.stopping_criterion);
    return solution_data;
}

std::string BendersBase::status_from_criterion() const
{
    switch (_data.stopping_criterion)
    {
    case StoppingCriterion::absolute_gap:
    case StoppingCriterion::relative_gap:
        return Output::OPTIMAL_C;
    case StoppingCriterion::max_iteration:
    case StoppingCriterion::timelimit:
        return Output::LIMIT_REACHED_C;
    default:
        return Output::ERROR_C;
    }
}

/*!
 *  \brief Get path to subproblem mps file from options
 */
std::filesystem::path BendersBase::GetSubproblemPath(const std::string& slave_name) const
{
    return std::filesystem::path(_options.INPUTROOT) / slave_name;
}

/*!
 *  \brief Return subproblem weight value
 *
 *  \param subproblem_count : total number of subproblems
 *
 *  \param name : subproblem name
 */
double BendersBase::SubproblemWeight(int subproblem_count, const std::string& name) const
{
    if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR)
    {
        return 1 / static_cast<double>(subproblem_count);
    }
    else if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR)
    {
        const double weight(_options.SLAVE_WEIGHT_VALUE);
        return 1 / weight;
    }
    else
    {
        return _options.weights.find(name)->second;
    }
}

/*!
 *  \brief Get path to master problem mps file from options
 */
std::filesystem::path BendersBase::get_master_path() const
{
    if (_options.PROBLEMS_FORMAT == ProblemsFormat::OPTIMIZED && _options.SOLVER_NAME == "XPRESS")
    {
        return std::filesystem::path(_options.INPUTROOT) / (_options.MASTER_NAME + SAVE_SUFFIX);
    }
    else
    {
        return std::filesystem::path(_options.INPUTROOT) / (_options.MASTER_NAME + MPS_SUFFIX);
    }
}

LogData BendersBase::bendersDataToLogData(const CurrentIterationData& data) const
{
    auto optimal_gap(data.best_ub - data.lb);
    return {data.lb,
            data.best_ub,
            data.ub,
            data.it + iterations_before_resume,
            data.best_it + iterations_before_resume,
            data.subproblem_cost,
            data.invest_cost,
            data.x_in,
            data.x_out,
            data.x_cut,
            data.min_invest,
            data.max_invest,
            optimal_gap,
            optimal_gap / data.best_ub,
            _options.MAX_ITERATIONS,
            data.benders_time,
            data.timer_master,
            data.subproblems_walltime,
            data.cumulative_number_of_subproblem_solved
              + cumulative_number_of_subproblem_resolved_before_resume};
}

void BendersBase::set_solver_log_file(const std::filesystem::path& log_file)
{
    solver_log_manager_ = SolverLogManager(log_file);
}

/*!
 *  \brief set the input
 *
 *  \param coupling_map : CouplingMap
 */
void BendersBase::set_input_map(const CouplingMap& coupling_map)
{
    coupling_map_ = coupling_map;
    _totalNbProblems = static_cast<int>(coupling_map_.size());
    _writer->write_nbweeks(_totalNbProblems);
    _data.nsubproblem = _totalNbProblems - 1;
    master_variable_map_ = get_master_variable_map(coupling_map_);
    coupling_map_.erase(get_master_name());
}

std::map<std::string, int> BendersBase::get_master_variable_map(
  const std::map<std::string, std::map<std::string, int>>& input_map) const
{
    const auto it_master(input_map.find(get_master_name()));
    if (it_master == input_map.end())
    {
        _logger->display_message(LOGLOCATION + "UNABLE TO FIND " + get_master_name() + "\n");
        std::exit(1);
    }
    return it_master->second;
}

void BendersBase::free_master()
{
    _master->free();
    master_is_empty_ = true;
}

WorkerMasterPtr BendersBase::get_master() const
{
    return _master;
}

void BendersBase::AddSubproblem(const std::pair<std::string, VariableMap>& kvp)
{
    std::shared_ptr<IBendersProblemProvider>
      benders_problem_provider = std::make_shared<BendersProblemFromFile>(
        GetSubproblemPath(kvp.first));
    subproblem_map[kvp.first] = std::make_shared<SubproblemWorker>(
      kvp.second,
      SubproblemWeight(_data.nsubproblem, kvp.first),
      _options.SOLVER_NAME,
      _options.LOG_LEVEL,
      solver_log_manager_,
      _logger,
      _options.PROBLEMS_FORMAT,
      benders_problem_provider.get(),
      _options.CUT_COEFFICIENT_TOLERANCE);
}

void BendersBase::free_subproblems()
{
    for (auto& ptr: subproblem_map)
    {
        ptr.second->free();
    }
}

void BendersBase::MatchProblemToId()
{
    int count = 0;
    for (const auto& problem: coupling_map_)
    {
        _problem_to_id[problem.first] = count;
        count++;
    }
}

void BendersBase::AddSubproblemName(const std::string& name)
{
    subproblems.push_back(name);
}

std::string BendersBase::get_master_name() const
{
    return _options.MASTER_NAME;
}

std::string BendersBase::get_solver_name() const
{
    return _options.SOLVER_NAME;
}

int BendersBase::get_log_level() const
{
    return _options.LOG_LEVEL;
}

bool BendersBase::is_trace() const
{
    return _options.TRACE;
}

Point BendersBase::get_x_cut() const
{
    return _data.x_cut;
}

void BendersBase::set_x_cut(const Point& x_cut)
{
    _data.x_cut = x_cut;
}

Point BendersBase::get_x_out() const
{
    return _data.x_out;
}

void BendersBase::set_x_out(const Point& x_out)
{
    _data.x_out = x_out;
}

double BendersBase::GetSubproblemCost() const
{
    return _data.subproblem_cost;
}

void BendersBase::SetSubproblemCost(const double& subproblem_cost)
{
    _data.subproblem_cost = subproblem_cost;
}

/*!
 *	\brief Update maximum and minimum of simplex iterations
 *
 *	\param subproblem_iterations : number of iterations done with the
 *subproblem
 *
 */
void BendersBase::BoundSimplexIterations(int subproblem_iterations)
{
    _data.max_simplexiter = (_data.max_simplexiter < subproblem_iterations) ? subproblem_iterations
                                                                            : _data.max_simplexiter;
    _data.min_simplexiter = (_data.min_simplexiter > subproblem_iterations) ? subproblem_iterations
                                                                            : _data.min_simplexiter;
}

void BendersBase::ResetSimplexIterationsBounds()
{
    _data.max_simplexiter = 0;
    // Tbb 2020 includes Windows min max defines that's why we don't write
    // std::numeric_limits<int>::max();
    _data.min_simplexiter = (std::numeric_limits<int>::max)();
}

bool BendersBase::IsResumeMode() const
{
    return _options.RESUME;
}

void BendersBase::UpdateMaxNumberIterationResumeMode(int nb_iteration_done)
{
    if (_options.MAX_ITERATIONS == -1)
    {
        return;
    }
    else if (_options.MAX_ITERATIONS - nb_iteration_done <= 0)
    {
        _data.stop = true;
    }
    else
    {
        _options.MAX_ITERATIONS -= nb_iteration_done;
    }
}

double BendersBase::execution_time() const
{
    return _data.benders_time;
}

void BendersBase::ChecksResumeMode()
{
    benders_timer = Timer();
    if (IsResumeMode())
    {
        auto reader = LastIterationReader(LastIterationFile());
        LogData last_iter;
        if (reader.IsLastIterationFileValid())
        {
            const auto [lastIter, bestIter] = reader.LastIterationData();
            best_iteration_data = bestIter;
            last_iter = lastIter;
        }
        else
        {
            best_iteration_data = bendersDataToLogData(_data);
            last_iter = best_iteration_data;
        }
        auto restart_data_printer = LastIterationPrinter(_logger, best_iteration_data, last_iter);
        restart_data_printer.Print();
        UpdateMaxNumberIterationResumeMode(last_iter.it);
        benders_timer = Timer(last_iter.benders_elapsed_time);
        _data.stop = ShouldBendersStop();
        iterations_before_resume = last_iter.it;
        cumulative_number_of_subproblem_resolved_before_resume
          = last_iter.cumulative_number_of_subproblem_resolved;
    }
}

void BendersBase::SaveCurrentBendersData()
{
    LastIterationWriter last_iteration_writer(LastIterationFile());
    const auto last = (_data.it == best_iteration_data.it) ? best_iteration_data
                                                           : bendersDataToLogData(_data);
    last_iteration_writer.SaveBestAndLastIterations(best_iteration_data, last);
    SaveCurrentIterationInOutputFile();
    if (_options.TRACE)
    {
        PrintCurrentIterationCsv();
    }
}

void BendersBase::ClearCurrentIterationCutTrace()
{
    relevantIterationData_.last._cut_trace.clear();
}

void BendersBase::EndWritingInOutputFile() const
{
    _writer->updateEndTime();
    // TODO duration for outer loop
    _writer->write_duration(_data.benders_time);
    if (!_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP)
    {
        SaveSolutionInOutputFile();
    }
}

double BendersBase::GetBendersTime() const
{
    return benders_timer.elapsed();
}

void BendersBase::write_basis() const
{
    const auto filename(std::filesystem::path(_options.OUTPUTROOT) / (_options.LAST_MASTER_BASIS));
    _master->write_basis(filename);
}

void BendersBase::MasterChangeRhs(int id_row, double val) const
{
    _master->ChangeRhs(id_row, val);
}

void BendersBase::MasterGetRhs(double& rhs, int id_row) const
{
    _master->GetRhs(&rhs, id_row);
}

void BendersBase::MasterAddRows(const std::vector<char>& qrtype_p,
                                const std::vector<double>& rhs_p,
                                const std::vector<double>& range_p,
                                const std::vector<int>& mstart_p,
                                const std::vector<int>& mclind_p,
                                const std::vector<double>& dmatval_p,
                                const std::vector<std::string>& row_names) const
{
    _master->AddRows(qrtype_p, rhs_p, range_p, mstart_p, mclind_p, dmatval_p, row_names);
}

bool BendersBase::MasterIsEmpty() const
{
    return master_is_empty_;
}

std::vector<double> BendersBase::MasterObjectiveFunctionCoeffs() const
{
    int ncols = _master->_solver->get_ncols();
    std::vector<double> obj(ncols);
    _master->_solver->get_obj(obj.data(), 0, ncols - 1);
    return obj;
}

void BendersBase::MasterRowsCoeffs(std::vector<int>& mstart,
                                   std::vector<int>& mclind,
                                   std::vector<double>& dmatval,
                                   int size,
                                   std::vector<int>& nels,
                                   int first,
                                   int last) const
{
    _master->_solver
      ->get_rows(mstart.data(), mclind.data(), dmatval.data(), size, nels.data(), first, last);
}

int BendersBase::MasterGetNElems() const
{
    return _master->_solver->get_nelems();
}

void BendersBase::SetMasterObjectiveFunctionCoeffsToZeros() const
{
    // assuming that master var id are in [0, size-1]
    auto master_vars_size = master_variable_map_.size();
    std::vector<double> zeros(master_vars_size, 0.0);
    SetMasterObjectiveFunction(zeros.data(), 0, static_cast<int>(master_vars_size) - 1);
}

void BendersBase::SetMasterObjectiveFunction(const double* coeffs, int first, int last) const
{
    assert(last >= first);
    _master->_solver->set_obj(coeffs, first, last);
}

int BendersBase::MasterGetnrows() const
{
    return _master->Getnrows();
}

int BendersBase::MasterGetncols() const
{
    return _master->Getncols();
}

void BendersBase::MasterGetRowType(std::vector<char>& qrtype, int first, int last) const
{
    _master->_solver->get_row_type(qrtype.data(), first, last);
}

WorkerMasterData BendersBase::BestIterationWorkerMaster() const
{
    return relevantIterationData_.best;
}

CurrentIterationData BendersBase::GetCurrentIterationData() const
{
    return _data;
}

CriteriaCurrentIterationData BendersBase::GetOuterLoopData() const
{
    return _data.criteria_current_iteration_data;
}

std::vector<double> BendersBase::GetOuterLoopCriterionAtBestBenders() const
{
    return ((criteria_vector_for_each_iteration_.empty())
              ? std::vector<double>() // Unnamed RVO
              : criteria_vector_for_each_iteration_[_data.best_it - 1]);
}

void BendersBase::init_data(double external_loop_lambda,
                            double external_loop_lambda_min,
                            double external_loop_lambda_max)
{
    benders_timer.restart();
    auto benders_num_run = _data.criteria_current_iteration_data.benders_num_run;
    auto outer_loop_bilevel_best_ub = _data.criteria_current_iteration_data
                                        .outer_loop_bilevel_best_ub;
    init_data();
    _data.criteria_current_iteration_data.criteria.clear();
    _data.criteria_current_iteration_data.benders_num_run = benders_num_run;
    _data.criteria_current_iteration_data.outer_loop_bilevel_best_ub = outer_loop_bilevel_best_ub;
    _data.criteria_current_iteration_data.lambda = external_loop_lambda;
    _data.criteria_current_iteration_data.lambda_min = external_loop_lambda_min;
    _data.criteria_current_iteration_data.lambda_max = external_loop_lambda_max;
}

bool BendersBase::isExceptionRaised() const
{
    return exception_raised_;
}

/*
 * after the 1st loop of the outer loop, we must  re-build the objective
 * function and costs
 */
void BendersBase::UpdateOverallCosts()
{
    auto obj = MasterObjectiveFunctionCoeffs();
    _data.invest_cost = 0;
    for (const auto& [var_name, var_id]: MasterVariables())
    {
        _data.invest_cost += obj[var_id] * _data.x_cut.at(var_name);
    }
    for (int i(0); i < _data.master_only_vars_cut.size(); ++i)
    {
        int col_id = _master->_id_master_only_vars[i];
        _data.invest_cost += obj[col_id] * _data.master_only_vars_cut[i];
    }

    relevantIterationData_.best._invest_cost = _data.invest_cost;
}

void BendersBase::SetBilevelBestub(double bilevel_best_ub)
{
    _data.criteria_current_iteration_data.outer_loop_bilevel_best_ub = bilevel_best_ub;
}

void BendersBase::setCriterionComputationInputs(
  const Benders::Criterion::CriterionInputData& criterion_input_data)
{
    criterion_computation_ = Benders::Criterion::CriterionComputation(criterion_input_data);
}

/*!
 *  \brief  _data.x_in is within the bounds thanks to restoreFeasibility called in
WorkerMaster::get(...). This function helps to avoid x_cut getting inifinitely close to a bound due
to the way it is updated using x_in:
    - Suppose x_in = x_out = 1 in the first iteration
    - Suppose x_out always 0 in the following iterations
    - Then x_cut will be always divided by 2 (if separation_parameter = 0.5) each time, becoming
inifinitely small. At some point we want to round it to the bound to avoid numerical issues. We
reuse the setting MASTER_SOLUTION_TOLERANCE
 */
void BendersBase::roundXCut()
{
    for (auto& kvp: _data.x_cut)
    {
        double value = kvp.second;
        double lb = _data.min_invest.at(kvp.first);
        double ub = _data.max_invest.at(kvp.first);

        if (std::abs(value - lb) < _options.MASTER_SOLUTION_TOLERANCE)
        {
            kvp.second = lb;
        }
        else if (std::abs(value - ub) < _options.MASTER_SOLUTION_TOLERANCE)
        {
            kvp.second = ub;
        }
    }
}

void BendersBase::SetPlugin(std::shared_ptr<BendersPlugin> benders_plugin)
{
    benders_plugin_ = benders_plugin;
}

std::string BendersBase::BendersName() const
{
    return GetCommunicationStrategy()->Name();
}

void BendersBase::InitializeMaster()
{
    if (GetCommunicationStrategy()->IsMaster())
    {
        std::shared_ptr<IBendersProblemProvider>
          benders_problem_provider = std::make_shared<BendersProblemFromFile>(get_master_path());
        reset_master<WorkerMaster>(master_variable_map_,
                                   get_solver_name(),
                                   get_log_level(),
                                   _data.nsubproblem,
                                   solver_log_manager_,
                                   IsResumeMode(),
                                   _logger,
                                   Options().PROBLEMS_FORMAT,
                                   benders_problem_provider.get(),
                                   Options().MASTER_SOLUTION_TOLERANCE,
                                   Options().CUT_COEFFICIENT_TOLERANCE);
    }
}

void BendersBase::BuildMasterProblem()
{
    InitializeMaster();
    if (GetCommunicationStrategy()->IsMaster())
    {
        _master->addAlphasFixingConstraints(subproblem_per_cut_indices_, _problem_to_id);
    }
}

void BendersBase::BroadCastVariablesIndices()
{
    if (GetCommunicationStrategy()->IsMaster())
    {
        SetSubproblemsVariablesIndices();
    }
    GetCommunicationStrategy()->Broadcast(criterion_computation_.getVarIndices());
}

void BendersBase::InitializeProblems()
{
    MatchProblemToId();
    SubProblemNamesInCut subs_per_proc;
    if (_options.CACHE_PROBLEMS)
    {
        int current_problem_id = 0;
        for (auto it = coupling_map_.begin(); it != coupling_map_.end();)
        {
            auto process_rank = current_problem_id % WorldSize();
            if (process_rank != Rank())
            {
                it = coupling_map_.erase(it);
            }
            else
            {
                subs_per_proc.emplace_back(it->first, process_rank);
                ++it;
            }
            ++current_problem_id;
        }
    }
    else
    {
        int current_problem_id = 0;
        for (const auto& problem: coupling_map_)
        {
            auto process_rank = current_problem_id % WorldSize();
            if (process_rank == Rank())
            {
                subs_per_proc.push_back({problem.first, process_rank});
                AddSubproblem(problem);
                AddSubproblemName(problem.first);
            }
            ++current_problem_id;
        }
    }

    std::vector<SubProblemNamesInCut> gathered_subs_per_proc;
    GetCommunicationStrategy()->Gather(subs_per_proc, gathered_subs_per_proc);
    if (GetCommunicationStrategy()->IsMaster())
    {
        subproblem_per_cut_indices_ = get_subs_per_cut(gathered_subs_per_proc, _data.nsubproblem);
    }
    BuildMasterProblem();
    BroadCastVariablesIndices();
    init_problems_ = false;
}

void BendersBase::free()
{
    if (GetCommunicationStrategy()->IsMaster())
    {
        if (get_master())
        {
            free_master();
        }
    }
    else
    {
        free_subproblems();
    }
    GetCommunicationStrategy()->Barrier();
}

void BendersBase::launch()
{
    if (GetCommunicationStrategy()->IsMaster())
    {
        _logger->display_message("Building input");
        _logger->display_message("Constructing workers...");
    }
    if (init_problems_)
    {
        InitializeProblems();
    }
    GetCommunicationStrategy()->Barrier();

    if (benders_plugin_)
    {
        benders_plugin_->OnBendersStart();
    }

    if (GetCommunicationStrategy()->IsMaster())
    {
        _logger->display_message("Running solver...");
    }

    try
    {
        Run();
        if (GetCommunicationStrategy()->IsMaster())
        {
            _logger->display_message(BendersName() + " solver terminated.");
        }
    }
    catch (const std::exception& ex)
    {
        if (GetCommunicationStrategy()->IsMaster())
        {
            std::string error = "Exception raised : " + std::string(ex.what());
            _logger->display_message(error);
        }
    }

    GetCommunicationStrategy()->Barrier();

    if (benders_plugin_)
    {
        benders_plugin_->OnBendersEnd();
    }

    post_run_actions();

    if (free_problems_)
    {
        free();
    }
    GetCommunicationStrategy()->Barrier();
}

void BendersBase::Run()
{
    if (init_data_)
    {
        PreRunInitialization();
    }
    else
    {
        _data.stop = false;
    }

    while (!_data.stop)
    {
        if (benders_plugin_)
        {
            benders_plugin_->OnBendersIterationStart();
        }
        ++_data.it;
        ResetSimplexIterationsBounds();

        if (benders_plugin_)
        {
            benders_plugin_->OnBendersMasterResolutionStart();
        }
        step_1_solve_master();
        if (benders_plugin_)
        {
            benders_plugin_->OnBendersMasterResolutionEnd();
        }

        if (!exception_raised_)
        {
            BuildCut();
        }

        if (!exception_raised_)
        {
            step_4_update_best_solution();
        }
        _data.stop |= exception_raised_;

        GetCommunicationStrategy()->Broadcast(_data.is_in_initial_relaxation);
        GetCommunicationStrategy()->Broadcast(_data.stop);

        if (GetCommunicationStrategy()->IsMaster())
        {
            if (mathLoggerDriver_)
            {
                mathLoggerDriver_->Print(_data);
            }
            SaveCurrentBendersData();
        }
    }

    if (GetCommunicationStrategy()->IsMaster())
    {
        CloseCsvFile();
        EndWritingInOutputFile();
        write_basis();
    }
    GetCommunicationStrategy()->Barrier();

    if (benders_plugin_)
    {
        benders_plugin_->OnBendersIterationEnd();
    }
}

void BendersBase::BuildCut()
{
    int success = 1;
    SubProblemDataMap subproblem_data_map;
    Timer walltime;
    Timer subproblems_timer_per_proc;
    _logger->display_message("\tSolving subproblems...");
    try
    {
        GetSubproblemCut(subproblem_data_map);
        _data.subproblems_cputime = subproblems_timer_per_proc.elapsed();
    }
    catch (const std::exception& ex)
    {
        success = 0;
        write_exception_message(ex);
    }
    check_if_some_proc_had_a_failure(success);
    if (!exception_raised_)
    {
        gather_subproblems_cut_package_and_build_cuts(subproblem_data_map, walltime);
    }
    if (GetCommunicationStrategy()->IsMaster())
    {
        _data.cumulative_number_of_subproblem_solved += _data.nsubproblem;
        _logger->cumulative_number_of_sub_problem_solved(
          _data.cumulative_number_of_subproblem_solved + GetNumOfSubProblemsSolvedBeforeResume());
    }
}

void BendersBase::PreRunInitialization()
{
    init_data();

    if (GetCommunicationStrategy()->IsMaster())
    {
        HandleInitialMasterRelaxation();
    }

    GetCommunicationStrategy()->Barrier();

    if (GetCommunicationStrategy()->IsMaster())
    {
        ChecksResumeMode();
        if (is_trace())
        {
            OpenCsvFile();
        }
    }
    if (mathLoggerDriver_)
    {
        mathLoggerDriver_->write_header();
    }
    init_data_ = false;
}

void BendersBase::step_1_solve_master()
{
    int success = 1;
    try
    {
        do_solve_master_create_trace_and_update_cuts();
    }
    catch (const std::exception& ex)
    {
        success = 0;
        write_exception_message(ex);
    }
    check_if_some_proc_had_a_failure(success);
    BroadcastXCut();
}

void BendersBase::do_solve_master_create_trace_and_update_cuts()
{
    if (GetCommunicationStrategy()->IsMaster())
    {
        if (SwitchToIntegerMaster(_data.is_in_initial_relaxation))
        {
            _logger->LogAtSwitchToInteger();
            ActivateIntegrityConstraints();
            ResetDataPostRelaxation();
        }
        solve_master_and_create_trace();
    }
}

void BendersBase::solve_master_and_create_trace()
{
    _logger->log_at_initialization(_data.it + GetNumIterationsBeforeRestart());
    _logger->display_message("\tSolving master...");
    get_master_value();
    _logger->log_master_solving_duration(_data.timer_master);
    ComputeXCut();
    _logger->log_iteration_candidates(bendersDataToLogData(_data));
}

void BendersBase::BroadcastXCut()
{
    if (!exception_raised_)
    {
        Point x_cut = get_x_cut();
        GetCommunicationStrategy()->Broadcast(x_cut);
        set_x_cut(x_cut);
    }
}

void BendersBase::step_4_update_best_solution()
{
    if (GetCommunicationStrategy()->IsMaster())
    {
        compute_ub();
        update_best_ub();
        _logger->log_at_iteration_end(bendersDataToLogData(_data));
        UpdateTrace();
        _data.iteration_time = -_data.benders_time;
        _data.benders_time = GetBendersTime();
        _data.iteration_time += _data.benders_time;
        _data.stop = ShouldBendersStop();
    }
}

void BendersBase::gather_subproblems_cut_package_and_build_cuts(
  const SubProblemDataMap& subproblem_data_map,
  const Timer& walltime)
{
    if (!exception_raised_)
    {
        GatherCuts(subproblem_data_map, walltime);
    }
}

void BendersBase::GatherCuts(const SubProblemDataMap& subproblem_data_map, const Timer& walltime)
{
    std::vector<SubProblemDataMap> gathered_subproblem_map;
    GetCommunicationStrategy()->Gather(subproblem_data_map, gathered_subproblem_map);
    _data.subproblems_walltime = walltime.elapsed();
    double cumulative_subproblems_timer_per_iter(0);
    GetCommunicationStrategy()->Reduce(_data.subproblems_cputime,
                                       cumulative_subproblems_timer_per_iter);
    _data.subproblems_cumulative_cputime = cumulative_subproblems_timer_per_iter;

    master_build_cuts(gathered_subproblem_map);

    if (!criterion_computation_.IsEmpty())
    {
        ComputeSubproblemsContributionToCriteria(subproblem_data_map);

        if (GetCommunicationStrategy()->IsMaster())
        {
            criteria_vector_for_each_iteration_.push_back(
              _data.criteria_current_iteration_data.criteria);
            UpdateMaxCriterionArea();
        }
    }
}

void BendersBase::check_if_some_proc_had_a_failure(int success)
{
    int global_success = GetCommunicationStrategy()->AllReduceBitwiseAnd(success);
    if (global_success == 0)
    {
        exception_raised_ = true;
    }
}

void BendersBase::master_build_cuts(const std::vector<SubProblemDataMap>& gathered_subproblem_map)
{
    SetSubproblemCost(0);
    SetSubproblemDataCostAndSimplexIter(gathered_subproblem_map);

    _data.ub = 0;

    if (GetCommunicationStrategy()->IsMaster())
    {
        build_all_aggregated_cuts(subproblem_per_cut_indices_, gathered_subproblem_map);
    }

    _logger->LogSubproblemsSolvingCumulativeCpuTime(_data.subproblems_cumulative_cputime);
    _logger->LogSubproblemsSolvingWalltime(_data.subproblems_walltime);
}

void BendersBase::SetSubproblemDataCostAndSimplexIter(
  const std::vector<SubProblemDataMap>& gathered_subproblem_map)
{
    for (const auto& subproblem_data_map: gathered_subproblem_map)
    {
        for (auto&& [sub_problem_name, subproblem_data]: subproblem_data_map)
        {
            SetSubproblemCost(GetSubproblemCost() + subproblem_data.subproblem_cost);
            BoundSimplexIterations(subproblem_data.simplex_iter);
        }
    }
}

void BendersBase::ComputeSubproblemsContributionToCriteria(
  const SubProblemDataMap& subproblem_data_map)
{
    const auto vars_size = criterion_computation_.getVarIndices().size();
    std::vector<double> criteria_per_sub_problem_per_pattern(vars_size, {});
    _data.criteria_current_iteration_data.criteria.resize(vars_size, 0.);
    std::vector<double> patterns_values_per_sub_problem_per_pattern(vars_size, {});
    _data.criteria_current_iteration_data.patterns_values.resize(vars_size, 0.);

    for (const auto& [subproblem_name, subproblem_data]: subproblem_data_map)
    {
        AddVectors<double>(criteria_per_sub_problem_per_pattern, subproblem_data.criteria);
        AddVectors<double>(patterns_values_per_sub_problem_per_pattern,
                           subproblem_data.patterns_values);
    }

    GetCommunicationStrategy()->Reduce(criteria_per_sub_problem_per_pattern,
                                       _data.criteria_current_iteration_data.criteria);
    GetCommunicationStrategy()->Reduce(patterns_values_per_sub_problem_per_pattern,
                                       _data.criteria_current_iteration_data.patterns_values);
}

void BendersBase::UpdateMaxCriterionArea()
{
    auto criteria_begin = _data.criteria_current_iteration_data.criteria.cbegin();
    auto criteria_end = _data.criteria_current_iteration_data.criteria.cend();
    auto max_criterion_it = std::max_element(criteria_begin, criteria_end);
    if (max_criterion_it != criteria_end)
    {
        _data.criteria_current_iteration_data.max_criterion = *max_criterion_it;
        auto max_criterion_index = std::distance(criteria_begin, max_criterion_it);
        _data.criteria_current_iteration_data.max_criterion_area = criterion_computation_
                                                                     .getCriterionInputData()
                                                                     .Criteria()
                                                                       [max_criterion_index]
                                                                     .Pattern()
                                                                     .GetBody();
    }
}

void BendersBase::write_exception_message(const std::exception& ex) const
{
    std::string error = "Exception raised : " + std::string(ex.what());
    _logger->display_message(error);
}

std::vector<SubProblemNamesInCut> BendersBase::get_subs_per_cut(
  const std::vector<SubProblemNamesInCut>& gathered_sub_per_proc,
  int max_aggregation)
{
    struct Entry
    {
        const std::string* name = nullptr;
        int vecPos = -1;
    };

    int n_cuts = SetAggregation(max_aggregation);

    if (n_cuts == 0)
    {
        return {};
    }

    std::vector<Entry> ordered(_data.nsubproblem);

    for (const auto& proc_subs_vec: gathered_sub_per_proc)
    {
        for (const auto& sub: proc_subs_vec)
        {
            auto it = _problem_to_id.find(sub.first);
            if (it == _problem_to_id.end())
            {
                continue;
            }
            ordered[it->second] = {&sub.first, sub.second};
        }
    }

    std::vector<SubProblemNamesInCut> cuts;
    cuts.reserve(n_cuts);

    SubProblemNamesInCut cut;
    cut.reserve((_data.nsubproblem + n_cuts - 1) / n_cuts);

    for (const auto& e: ordered)
    {
        if (!e.name)
        {
            continue;
        }

        cut.emplace_back(*e.name, e.vecPos);
        if (cut.size() == static_cast<size_t>((_data.nsubproblem + n_cuts - 1) / n_cuts))
        {
            cuts.emplace_back(std::move(cut));
            cut.clear();
            cut.reserve((_data.nsubproblem + n_cuts - 1) / n_cuts);
        }
    }

    if (!cut.empty())
    {
        cuts.emplace_back(std::move(cut));
    }
    return cuts;
}
