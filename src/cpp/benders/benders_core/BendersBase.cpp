#include "BendersBase.h"

#include <execution>
#include <memory>
#include <mutex>
#include <numeric>
#include <utility>

#include "LastIterationPrinter.h"
#include "LastIterationReader.h"
#include "LastIterationWriter.h"
#include "glog/logging.h"
#include "solver_utils.h"

BendersBase::BendersBase(BendersBaseOptions options, Logger logger,
                         Writer writer)
    : _options(std::move(options)),
      _csv_file_path(std::filesystem::path(_options.OUTPUTROOT) /
                     (_options.CSV_NAME + ".csv")),
      _logger(std::move(logger)),
      _writer(std::move(writer)) {}

/*!
 *  \brief Initialize set of data used in the loop
 */
void BendersBase::init_data() {
  _data.nbasis = 0;
  _data.lb = -1e20;
  _data.ub = +1e20;
  _data.best_ub = +1e20;
  _data.stop = false;
  _data.it = 0;
  _data.alpha = 0;
  _data.invest_cost = 0;
  _data.deletedcut = 0;
  _data.maxsimplexiter = 0;
  _data.minsimplexiter = std::numeric_limits<int>::max();
  _data.best_it = 0;
  _data.stopping_criterion = StoppingCriterion::empty;
  _data.is_in_initial_relaxation = false;
}

void BendersBase::OpenCsvFile() {
  if (!_csv_file.is_open()) {
    const auto opening_mode = _options.RESUME ? std::ios::app : std::ios::trunc;
    _csv_file.open(_csv_file_path, std::ios::out | opening_mode);
    if (_csv_file && !_options.RESUME) {
      _csv_file
          << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;alpha_i;"
             "deletedcut;time;basis;"
          << std::endl;
    } else {
      LOG(INFO) << "Impossible to open the .csv file: " << _csv_file_path
                << std::endl;
    }
  }
}

void BendersBase::CloseCsvFile() {
  if (_csv_file.is_open()) {
    _csv_file.close();
  }
}
void BendersBase::PrintCurrentIterationCsv() {
  print_csv_iteration(_csv_file, _data.it - 1);
}

/*!
 *  \brief Print the trace of the Benders algorithm in a csv file
 *
 *  Method to print trace of the Benders algorithm in a csv file
 *
 */
void BendersBase::print_csv() {
  const auto output =
      std::filesystem::path(_options.OUTPUTROOT) / (_options.CSV_NAME + ".csv");
  std::ofstream file(output, std::ios::out | std::ios::trunc);
  if (file) {
    file << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;alpha_i;"
            "deletedcut;time;basis;"
         << std::endl;
    int const nite = _trace.size();
    for (int i = 0; i < nite; i++) {
      print_csv_iteration(file, i);
    }
    file.close();
  } else {
    LOG(INFO) << "Impossible to open the .csv file" << std::endl;
  }
}

void BendersBase::print_csv_iteration(std::ostream &file, int ite) {
  if (_trace[ite]->_valid) {
    Point xopt;
    // Write first problem : use result of best iteration
    if (ite == 0) {
      int best_it_index = _data.best_it - 1;
      if (best_it_index >= 0 && _trace.size() > best_it_index) {
        xopt = _trace[best_it_index]->get_point();
      }
    } else {
      xopt = _trace[ite - 1]->get_point();
    }
    print_master_and_cut(file, ite + 1 + iterations_before_resume, _trace[ite],
                         xopt);
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
void print_cut_csv(std::ostream &stream,
                   SubproblemCutDataHandler const &handler,
                   std::string const &name, int subproblem_index) {
  stream << "Subproblem"
         << ";";
  stream << name << ";";
  stream << subproblem_index << ";";
  stream << handler.get_dbl(SUBPROBLEM_COST) << ";";
  stream << ";";
  stream << ";";
  stream << handler.get_int(SIMPLEXITER) << ";";
  stream << ";";
  stream << handler.get_dbl(ALPHA_I) << ";";
  stream << ";";
  stream << handler.get_dbl(SUBPROBLEM_TIMER) << ";";
  stream << std::endl;
}

void BendersBase::print_master_and_cut(std::ostream &file, int ite,
                                       WorkerMasterDataPtr &trace,
                                       Point const &xopt) {
  file << ite << ";";

  print_master_csv(file, trace, xopt);

  for (auto &kvp : trace->_cut_trace) {
    const SubproblemCutDataHandler handler(kvp.second);
    file << ite << ";";
    print_cut_csv(file, handler, kvp.first, _problem_to_id[kvp.first]);
  }
}

/*!
 *  \brief Print in a file master's information
 *
 *  \param stream : output stream
 *
 *  \param trace : storage of problem data
 *
 *  \param xopt : final optimal value
 */
void BendersBase::print_master_csv(std::ostream &stream,
                                   const WorkerMasterDataPtr &trace,
                                   Point const &xopt) const {
  stream << "Master"
         << ";";
  stream << _options.MASTER_NAME << ";";
  stream << _data.nsubproblem << ";";
  stream << trace->_ub << ";";
  stream << trace->_lb << ";";
  stream << trace->_best_ub << ";";
  stream << ";";
  stream << norm_point(xopt, trace->get_point()) << ";";
  stream << ";";
  stream << trace->_deleted_cut << ";";
  stream << trace->_master_duration << ";";
  stream << trace->_nbasis << ";" << std::endl;
}

/*!
 *  \brief Update best upper bound and best optimal variables
 *
 *	Function to update best upper bound and best optimal variables regarding
 *the current ones
 */
void BendersBase::update_best_ub() {
  if (_data.best_ub > _data.ub) {
    _data.best_ub = _data.ub;
    _data.bestx = _data.x0;
    _data.best_it = _data.it;
    best_iteration_data = bendersDataToLogData(_data);
  }
}

/*!
 *	\brief Update maximum and minimum of a set of int
 *
 *	\param simplexiter : int to compare to current max and min
 *
 */
void BendersBase::bound_simplex_iter(int simplexiter) {
  if (_data.maxsimplexiter < simplexiter) {
    _data.maxsimplexiter = simplexiter;
  }
  if (_data.minsimplexiter > simplexiter) {
    _data.minsimplexiter = simplexiter;
  }
}

/*!
 *  \brief Check if initial relaxation should stop
 */
bool BendersBase::relaxation_stopping_criterion() const {
  return (((_data.best_ub - _data.lb) / _data.best_ub) <= _options.RELAXED_GAP);
}

/*!
 *  \brief Update stopping criterion
 *
 *  Method updating the stopping criterion and reinitializing some datas
 *
 */
bool BendersBase::stopping_criterion() {
  _data.deletedcut = 0;
  _data.maxsimplexiter = 0;
  _data.minsimplexiter = std::numeric_limits<int>::max();
  if (_data.elapsed_time > _options.TIME_LIMIT)
    _data.stopping_criterion = StoppingCriterion::timelimit;
  else if ((_options.MAX_ITERATIONS != -1) &&
           (_data.it >= _options.MAX_ITERATIONS))
    _data.stopping_criterion = StoppingCriterion::max_iteration;
  else if (_data.lb + _options.ABSOLUTE_GAP >= _data.best_ub)
    _data.stopping_criterion = StoppingCriterion::absolute_gap;
  else if (((_data.best_ub - _data.lb) / _data.best_ub) <=
           _options.RELATIVE_GAP)
    _data.stopping_criterion = StoppingCriterion::relative_gap;

  return _data.stopping_criterion != StoppingCriterion::empty;
}

/*!
 *  \brief Update trace of the Benders for the current iteration
 *
 *  Fonction to store the current Benders data in the trace
 */
void BendersBase::update_trace() {
  auto &LastWorkerMasterDataPtr = _trace[_data.it - 1];
  LastWorkerMasterDataPtr->_lb = _data.lb;
  LastWorkerMasterDataPtr->_ub = _data.ub;
  LastWorkerMasterDataPtr->_best_ub = _data.best_ub;
  LastWorkerMasterDataPtr->_x0 = std::make_shared<Point>(_data.x0);
  LastWorkerMasterDataPtr->_max_invest =
      std::make_shared<Point>(_data.max_invest);
  LastWorkerMasterDataPtr->_min_invest =
      std::make_shared<Point>(_data.min_invest);
  LastWorkerMasterDataPtr->_deleted_cut = _data.deletedcut;
  LastWorkerMasterDataPtr->_master_duration = _data.timer_master;
  LastWorkerMasterDataPtr->_subproblem_duration = _data.subproblem_timers;
  LastWorkerMasterDataPtr->_nbasis = _data.nbasis;
  LastWorkerMasterDataPtr->_invest_cost = _data.invest_cost;
  LastWorkerMasterDataPtr->_operational_cost = _data.subproblem_cost;
  LastWorkerMasterDataPtr->_valid = true;
}

bool BendersBase::is_initial_relaxation_requested() const {
  return (_options.MASTER_FORMULATION == MasterFormulation::INTEGER &&
          _options.INITIAL_MASTER_RELAXATION);
}

bool BendersBase::switch_to_integer_master(bool is_relaxed) const {
  return is_initial_relaxation_requested() && is_relaxed &&
         relaxation_stopping_criterion();
}

void BendersBase::set_data_pre_relaxation() {
  _data.is_in_initial_relaxation = true;
}

void BendersBase::reset_data_post_relaxation() {
  _data.is_in_initial_relaxation = false;
  _data.best_ub = 1e+20;
  _data.best_it = 0;
}

/*!
 *  \brief Check if every subproblem has been solved to optimality
 *
 *  \param all_package : storage of each subproblems status
 *  \param data : BendersData used to get master solving status
 */
void BendersBase::check_status(AllCutPackage const &all_package) const {
  if (_data.master_status != SOLVER_STATUS::OPTIMAL) {
    LOG(INFO) << "Master status is " << _data.master_status << std::endl;
    throw InvalidSolverStatusException("Master status is " +
                                       std::to_string(_data.master_status));
  }
  for (const auto &package : all_package) {
    for (const auto &kvp : package) {
      SubproblemCutDataPtr subproblem_cut_data(
          new SubproblemCutData(kvp.second));
      SubproblemCutDataHandlerPtr const handler(
          new SubproblemCutDataHandler(subproblem_cut_data));
      if (handler->get_int(LPSTATUS) != SOLVER_STATUS::OPTIMAL) {
        std::stringstream stream;
        stream << "Subproblem " << kvp.first << " status is "
               << handler->get_int(LPSTATUS);
        LOG(INFO) << stream.str() << std::endl;

        throw InvalidSolverStatusException(stream.str());
      }
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
void BendersBase::get_master_value() {
  Timer timer_master;
  _data.alpha_i.resize(_data.nsubproblem);
  if (_options.BOUND_ALPHA) {
    _master->fix_alpha(_data.best_ub);
  }
  _master->solve(_data.master_status, _options.OUTPUTROOT,
                 _options.LAST_MASTER_MPS + MPS_SUFFIX);
  _master->get(
      _data.x0, _data.alpha,
      _data.alpha_i); /*Get the optimal variables of the Master Problem*/
  _master->get_value(_data.lb); /*Get the optimal value of the Master Problem*/

  _data.invest_cost = _data.lb - _data.alpha;

  for (const auto &pairIdName : _master->_id_to_name) {
    _master->_solver->get_ub(&_data.max_invest[pairIdName.second],
                             pairIdName.first, pairIdName.first);
    _master->_solver->get_lb(&_data.min_invest[pairIdName.second],
                             pairIdName.first, pairIdName.first);
  }

  _data.ub = _data.invest_cost;
  _data.timer_master = timer_master.elapsed();
}

void BendersBase::deactivate_integrity_constraints() const {
  _master->deactivate_integrity_constraints();
}

void BendersBase::activate_integrity_constraints() const {
  _master->activate_integrity_constraints();
}

/**
 * std execution policies don't share a base type so we can't just select them
 *in place in the foreach This function allow the selection of policy via
 *template deduction
 **/
template <class lambda>
auto selectPolicy(lambda f, bool shouldParallelize) {
  if (shouldParallelize)
    return f(std::execution::par_unseq);
  else
    return f(std::execution::seq);
}

/*!
 *  \brief Solve and store optimal variables of all Subproblem Problems
 *
 *  Method to solve and store optimal variables of all Subproblem Problems
 * after fixing trial values
 *
 *  \param subproblem_cut_package : map storing for each subproblem its cut
 */
void BendersBase::getSubproblemCut(
    SubproblemCutPackage &subproblem_cut_package) {
  // With gcc9 there was no parallelisation when iterating on the map directly
  // so with project it in a vector
  std::vector<std::pair<std::string, SubproblemWorkerPtr>> nameAndWorkers;
  nameAndWorkers.reserve(subproblem_map.size());
  for (const auto &[name, worker] : subproblem_map) {
    nameAndWorkers.emplace_back(name, worker);
  }
  std::mutex m;
  selectPolicy(
      [this, &nameAndWorkers, &m, &subproblem_cut_package](auto &policy) {
        std::for_each(
            policy, nameAndWorkers.begin(), nameAndWorkers.end(),
            [this, &m, &subproblem_cut_package](
                const std::pair<std::string, SubproblemWorkerPtr> &kvp) {
              const auto &[name, worker] = kvp;
              Timer subproblem_timer;
              auto subproblem_cut_data(std::make_shared<SubproblemCutData>());
              auto handler(std::make_shared<SubproblemCutDataHandler>(
                  subproblem_cut_data));
              worker->fix_to(_data.x0);
              worker->solve(handler->get_int(LPSTATUS), _options.OUTPUTROOT,
                            _options.LAST_MASTER_MPS + MPS_SUFFIX);
              worker->get_value(handler->get_dbl(SUBPROBLEM_COST));
              worker->get_subgradient(handler->get_subgradient());
              worker->get_splex_num_of_ite_last(handler->get_int(SIMPLEXITER));
              handler->get_dbl(SUBPROBLEM_TIMER) = subproblem_timer.elapsed();
              std::lock_guard guard(m);
              subproblem_cut_package[name] = *subproblem_cut_data;
            });
      },
      shouldParallelize());
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
void BendersBase::compute_cut(AllCutPackage const &all_package) {
  std::for_each(
      all_package.begin(), all_package.end(),
      [this](const SubproblemCutPackage &i) {
        for (auto const &[name, data] : i) {
          auto subproblem_cut_data(std::make_shared<SubproblemCutData>(data));
          auto handler(
              std::make_shared<SubproblemCutDataHandler>(subproblem_cut_data));
          handler->get_dbl(ALPHA_I) = _data.alpha_i[_problem_to_id[name]];
          _data.ub += handler->get_dbl(SUBPROBLEM_COST);
          SubproblemCutTrimmer cut(handler, _data.x0);

          _master->addSubproblemCut(_problem_to_id[name],
                                    handler->get_subgradient(), _data.x0,
                                    handler->get_dbl(SUBPROBLEM_COST));
          _all_cuts_storage[name].insert(cut);
          _trace[_data.it - 1]->_cut_trace[name] = subproblem_cut_data;

          bound_simplex_iter(handler->get_int(SIMPLEXITER));
        }
      });
}

void compute_cut_val(const SubproblemCutDataHandlerPtr &handler,
                     const Point &x0, Point &s) {
  for (auto const &var : x0) {
    if (handler->get_subgradient().find(var.first) !=
        handler->get_subgradient().end()) {
      s[var.first] += handler->get_subgradient().find(var.first)->second;
    }
  }
}

/*!
 *  \brief Add aggregated cut to Master Problem and store it in a set
 *
 *  Method to add aggregated cut from subproblems to Master Problem and store
 * it in a map linking each subproblem to its set of non-aggregated cut
 *
 *  \param all_package : vector storing all cuts information for each
 * subproblem problem
 */
void BendersBase::compute_cut_aggregate(AllCutPackage const &all_package) {
  Point s;
  double rhs(0);
  for (const auto &i : all_package) {
    for (auto const &itmap : i) {
      SubproblemCutDataPtr subproblem_cut_data(
          new SubproblemCutData(itmap.second));
      SubproblemCutDataHandlerPtr handler(
          new SubproblemCutDataHandler(subproblem_cut_data));
      _data.ub += handler->get_dbl(SUBPROBLEM_COST);
      rhs += handler->get_dbl(SUBPROBLEM_COST);

      compute_cut_val(handler, _data.x0, s);

      SubproblemCutTrimmer cut(handler, _data.x0);

      _all_cuts_storage.find(itmap.first)->second.insert(cut);
      _trace[_data.it - 1]->_cut_trace[itmap.first] = subproblem_cut_data;

      bound_simplex_iter(handler->get_int(SIMPLEXITER));
    }
  }
  _master->add_cut(s, _data.x0, rhs);
}

/*!
 *  \brief Add cuts in master problem
 *
 *	Add cuts in master problem according to the selected option
 *
 *  \param all_package : storage of every subproblem information
 */
void BendersBase::build_cut_full(AllCutPackage const &all_package) {
  check_status(all_package);
  if (_options.AGGREGATION) {
    compute_cut_aggregate(all_package);
  } else {
    compute_cut(all_package);
  }
}

LogData BendersBase::build_log_data_from_data() const {
  auto logData = FinalLogData();
  logData.optimality_gap = _options.ABSOLUTE_GAP;
  logData.relative_gap = _options.RELATIVE_GAP;
  logData.max_iterations = _options.MAX_ITERATIONS;
  return logData;
}

LogData BendersBase::FinalLogData() const {
  LogData result;

  result.it = _data.it + iterations_before_resume;
  result.best_it = _data.best_it + iterations_before_resume;
  result.lb = _data.lb;
  result.best_ub = _data.best_ub;
  result.x0 = _data.bestx;

  result.subproblem_cost = best_iteration_data.subproblem_cost;
  result.invest_cost = best_iteration_data.invest_cost;
  result.min_invest = best_iteration_data.min_invest;
  result.max_invest = best_iteration_data.max_invest;

  return result;
}

void BendersBase::post_run_actions() const {
  LogData logData = build_log_data_from_data();

  _logger->log_stop_criterion_reached(_data.stopping_criterion);
  _logger->log_at_ending(logData);
}

void BendersBase::SaveCurrentIterationInOutputFile() const {
  auto &LastWorkerMasterDataPtr = _trace[_data.it - 1];
  if (LastWorkerMasterDataPtr->_valid) {
    _writer->write_iteration(iteration(LastWorkerMasterDataPtr),
                             _data.it + iterations_before_resume);
    _writer->dump();
  }
}
void BendersBase::SaveSolutionInOutputFile() const {
  _writer->write_solution(solution());
  _writer->dump();
}

Output::CandidatesVec candidates_data(
    const WorkerMasterDataPtr &masterDataPtr_l) {
  Output::CandidatesVec candidates_vec;
  for (const auto &pairNameValue_l : masterDataPtr_l->get_point()) {
    Output::CandidateData candidate_data;
    candidate_data.name = pairNameValue_l.first;
    candidate_data.invest = pairNameValue_l.second;
    candidate_data.min =
        masterDataPtr_l->get_min_invest()[pairNameValue_l.first];
    candidate_data.max =
        masterDataPtr_l->get_max_invest()[pairNameValue_l.first];
    candidates_vec.push_back(candidate_data);
  }

  return candidates_vec;
}

Output::Iteration BendersBase::iteration(
    const WorkerMasterDataPtr &masterDataPtr_l) const {
  Output::Iteration iteration;
  iteration.master_duration = masterDataPtr_l->_master_duration;
  iteration.subproblem_duration = masterDataPtr_l->_subproblem_duration;
  iteration.lb = masterDataPtr_l->_lb;
  iteration.ub = masterDataPtr_l->_ub;
  iteration.best_ub = masterDataPtr_l->_best_ub;
  iteration.optimality_gap = masterDataPtr_l->_best_ub - masterDataPtr_l->_lb;
  iteration.relative_gap = (masterDataPtr_l->_best_ub - masterDataPtr_l->_lb) /
                           masterDataPtr_l->_best_ub;
  iteration.investment_cost = masterDataPtr_l->_invest_cost;
  iteration.operational_cost = masterDataPtr_l->_operational_cost;
  iteration.overall_cost =
      masterDataPtr_l->_invest_cost + masterDataPtr_l->_operational_cost;
  iteration.candidates = candidates_data(masterDataPtr_l);
  return iteration;
}

Output::IterationsData BendersBase::output_data() const {
  Output::IterationsData iterations_data;
  Output::Iterations iters;
  iterations_data.nbWeeks_p = _totalNbProblems;
  // Iterations
  for (const auto &masterDataPtr_l : _trace) {
    if (masterDataPtr_l->_valid) {
      iters.push_back(iteration(masterDataPtr_l));
    }
  }
  iterations_data.iters = iters;
  iterations_data.solution_data = solution();
  iterations_data.elapsed_time = _data.elapsed_time;
  return iterations_data;
}

Output::SolutionData BendersBase::solution() const {
  Output::SolutionData solution_data;
  solution_data.nbWeeks_p = _totalNbProblems;
  solution_data.best_it = _data.best_it + iterations_before_resume;
  solution_data.problem_status = status_from_criterion();
  const auto optimal_gap(_data.best_ub - _data.lb);
  const auto relative_gap(optimal_gap / _data.best_ub);

  if (IsResumeMode()) {
    // solution may not be in _trace
    Output::CandidatesVec candidates_vec;
    std::transform(
        best_iteration_data.x0.cbegin(), best_iteration_data.x0.cend(),
        std::back_inserter(candidates_vec),
        [this](const std::pair<std::string, double> &name_invest)
            -> Output::CandidateData {
          const auto &[name, invest] = name_invest;
          return {name, invest, best_iteration_data.min_invest.at(name),
                  best_iteration_data.max_invest.at(name)};
        });
    solution_data.solution = {
        best_iteration_data.master_time,
        best_iteration_data.subproblem_time,
        best_iteration_data.lb,
        best_iteration_data.ub,
        best_iteration_data.best_ub,
        optimal_gap,
        relative_gap,
        best_iteration_data.invest_cost,
        best_iteration_data.subproblem_cost,
        best_iteration_data.invest_cost + best_iteration_data.subproblem_cost,
        candidates_vec};

  } else {
    size_t bestItIndex_l = _data.best_it - 1;

    if (bestItIndex_l < _trace.size()) {
      solution_data.solution = iteration(_trace[bestItIndex_l]);
      solution_data.solution.optimality_gap = optimal_gap;
      solution_data.solution.relative_gap = relative_gap;
    }
  }
  solution_data.stopping_criterion = criterion_to_str(_data.stopping_criterion);
  return solution_data;
}

std::string BendersBase::status_from_criterion() const {
  switch (_data.stopping_criterion) {
    case StoppingCriterion::absolute_gap:
    case StoppingCriterion::relative_gap:
    case StoppingCriterion::max_iteration:
    case StoppingCriterion::timelimit:
      return Output::STATUS_OPTIMAL_C;

    default:
      return Output::STATUS_ERROR_C;
  }
}

/*!
 *  \brief Get path to subproblem mps file from options
 */
std::filesystem::path BendersBase::GetSubproblemPath(
    std::string const &slave_name) const {
  return std::filesystem::path(_options.INPUTROOT) / (slave_name + MPS_SUFFIX);
}

/*!
 *  \brief Return subproblem weight value
 *
 *  \param subproblem_count : total number of subproblems
 *
 *  \param name : subproblem name
 */
double BendersBase::SubproblemWeight(int subproblem_count,
                                     std::string const &name) const {
  if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR) {
    return 1 / static_cast<double>(subproblem_count);
  } else if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR) {
    double const weight(_options.SLAVE_WEIGHT_VALUE);
    return 1 / weight;
  } else {
    return _options.weights.find(name)->second;
  }
}

/*!
 *  \brief Get path to master problem mps file from options
 */
std::filesystem::path BendersBase::get_master_path() const {
  return std::filesystem::path(_options.INPUTROOT) /
         (_options.MASTER_NAME + MPS_SUFFIX);
}

/*!
 *  \brief Get path to structure txt file from options
 */
std::filesystem::path BendersBase::get_structure_path() const {
  return std::filesystem::path(_options.INPUTROOT) / _options.STRUCTURE_FILE;
}

LogData BendersBase::bendersDataToLogData(const BendersData &data) const {
  auto optimal_gap(data.best_ub - data.lb);
  return {data.lb,
          data.best_ub,
          data.ub,
          data.it + iterations_before_resume,
          data.best_it + iterations_before_resume,
          data.subproblem_cost,
          data.invest_cost,
          data.x0,
          data.min_invest,
          data.max_invest,
          optimal_gap,
          optimal_gap / data.best_ub,
          _options.MAX_ITERATIONS,
          data.elapsed_time,
          data.timer_master,
          data.subproblem_timers};
}
void BendersBase::set_log_file(const std::filesystem::path &log_name) {
  _log_name = log_name;
}

/*!
 *  \brief Build the input from the structure file
 *
 *	Function to build the map linking each problem name to its variables and
 *their id
 *
 *  \param root : root of the structure file
 *
 *  \param summary_name : name of the structure file
 *
 *  \param coupling_map : empty map to increment
 *
 *  \note The id in the coupling_map is that of the variable in the solver
 *responsible for the creation of the structure file.
 */
void BendersBase::build_input_map() {
  auto input = build_input(get_structure_path());
  _totalNbProblems = input.size();
  _writer->write_nbweeks(_totalNbProblems);
  _data.nsubproblem = _totalNbProblems - 1;
  master_variable_map = get_master_variable_map(input);
  coupling_map = GetCouplingMap(input);
}

std::map<std::string, int> BendersBase::get_master_variable_map(
    std::map<std::string, std::map<std::string, int>> input_map) const {
  auto const it_master(input_map.find(get_master_name()));
  if (it_master == input_map.end()) {
    LOG(ERROR) << "UNABLE TO FIND " << get_master_name() << std::endl;
    std::exit(1);
  }
  return it_master->second;
}

CouplingMap BendersBase::GetCouplingMap(CouplingMap input) const {
  CouplingMap couplingMap;
  auto master_name = get_master_name();
  std::copy_if(input.begin(), input.end(),
               std::inserter(couplingMap, couplingMap.end()),
               [master_name](const CouplingMap::value_type &kvp) {
                 return kvp.first != master_name;
               });
  return couplingMap;
}

void BendersBase::push_in_trace(const WorkerMasterDataPtr &worker_master_data) {
  _trace.push_back(worker_master_data);
}

void BendersBase::reset_master(WorkerMaster *worker_master) {
  _master.reset(worker_master);
}
void BendersBase::free_master() const { _master->free(); }
WorkerMasterPtr BendersBase::get_master() const { return _master; }

void BendersBase::addSubproblem(
    const std::pair<std::string, VariableMap> &kvp) {
  subproblem_map[kvp.first] = std::make_shared<SubproblemWorker>(
      kvp.second, GetSubproblemPath(kvp.first),
      SubproblemWeight(_data.nsubproblem, kvp.first), _options.SOLVER_NAME,
      _options.LOG_LEVEL, log_name());
}

void BendersBase::free_subproblems() {
  for (auto &ptr : subproblem_map) ptr.second->free();
}
void BendersBase::match_problem_to_id() {
  int count = 0;
  for (const auto &problem : coupling_map) {
    _problem_to_id[problem.first] = count;
    count++;
  }
}
void BendersBase::set_cut_storage() {
  for (auto const &kvp : _problem_to_id) {
    _all_cuts_storage[kvp.first] = SubproblemCutStorage();
  }
}

void BendersBase::AddSubproblemName(const std::string &name) {
  subproblems.push_back(name);
}
std::string BendersBase::get_master_name() const {
  return _options.MASTER_NAME;
}
std::string BendersBase::get_solver_name() const {
  return _options.SOLVER_NAME;
}
int BendersBase::get_log_level() const { return _options.LOG_LEVEL; }
bool BendersBase::is_trace() const { return _options.TRACE; }
Point BendersBase::get_x0() const { return _data.x0; }
void BendersBase::set_x0(const Point &x0) { _data.x0 = x0; }
double BendersBase::get_timer_master() const { return _data.timer_master; }
void BendersBase::set_timer_master(const double &timer_master) {
  _data.timer_master = timer_master;
}
double BendersBase::GetSubproblemTimers() const {
  return _data.subproblem_timers;
}
void BendersBase::SetSubproblemTimers(const double &subproblem_timer) {
  _data.subproblem_timers = subproblem_timer;
}
double BendersBase::GetSubproblemCost() const { return _data.subproblem_cost; }
void BendersBase::SetSubproblemCost(const double &subproblem_cost) {
  _data.subproblem_cost = subproblem_cost;
}

bool BendersBase::IsResumeMode() const { return _options.RESUME; }

void BendersBase::UpdateMaxNumberIterationResumeMode(
    const unsigned nb_iteration_done) {
  if (_options.MAX_ITERATIONS == -1) {
    return;
  } else if (_options.MAX_ITERATIONS - nb_iteration_done <= 0) {
    _data.stop = true;

  } else {
    _options.MAX_ITERATIONS -= nb_iteration_done;
  }
}

double BendersBase::execution_time() const { return _data.elapsed_time; }
LogData BendersBase::GetBestIterationData() const {
  return best_iteration_data;
}

void BendersBase::ChecksResumeMode() {
  benders_timer = Timer();
  if (IsResumeMode()) {
    auto reader = LastIterationReader(LastIterationFile());
    LogData last_iter;
    if (reader.IsLastIterationFileValid()) {
      const auto [lastIter, bestIter] = reader.LastIterationData();
      best_iteration_data = bestIter;
      last_iter = lastIter;
    } else {
      best_iteration_data = bendersDataToLogData(_data);
      last_iter = best_iteration_data;
    }
    auto restart_data_printer =
        LastIterationPrinter(_logger, best_iteration_data, last_iter);
    restart_data_printer.Print();
    UpdateMaxNumberIterationResumeMode(last_iter.it);
    benders_timer = Timer(last_iter.benders_elapsed_time);
    _data.stop = stopping_criterion();
    iterations_before_resume = last_iter.it;
  }
}

void BendersBase::SaveCurrentBendersData() {
  LastIterationWriter last_iteration_writer(LastIterationFile());
  const auto last = (_data.it == best_iteration_data.it)
                        ? best_iteration_data
                        : bendersDataToLogData(_data);
  last_iteration_writer.SaveBestAndLastIterations(best_iteration_data, last);
  SaveCurrentIterationInOutputFile();
  if (_options.TRACE) {
    PrintCurrentIterationCsv();
  }
}

void BendersBase::EndWritingInOutputFile() const {
  _writer->updateEndTime();
  _writer->write_duration(_data.elapsed_time);
  SaveSolutionInOutputFile();
}
double BendersBase::GetBendersTime() const { return benders_timer.elapsed(); }
void BendersBase::write_basis() const {
  const auto filename(std::filesystem::path(_options.OUTPUTROOT) /
                      (_options.LAST_MASTER_BASIS));
  _master->write_basis(filename);
}
