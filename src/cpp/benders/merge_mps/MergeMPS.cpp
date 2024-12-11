#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/helpers/Timer.h"

MergeMPS::MergeMPS(const MergeMPSOptions &options, Logger &logger,
                   Writer writer)
    : _options(options), _logger(logger), _writer(writer) {}

void MergeMPS::launch() {
  const auto inputRootDir = std::filesystem::path(_options.INPUTROOT);
  auto structure_path(inputRootDir / _options.STRUCTURE_FILE);
  CouplingMap input =
      CouplingMapGenerator::BuildInput(structure_path, _logger, "Merge mps");

  SolverFactory factory;
  std::string solver_to_use =
      (_options.SOLVER_NAME == "COIN") ? "CBC" : _options.SOLVER_NAME;
  SolverAbstract::Ptr mergedSolver_l = factory.create_solver(solver_to_use);
  mergedSolver_l->set_output_log_level(_options.LOG_LEVEL);

  int nslaves = input.size() - 1;
  CouplingMap x_mps_id;
  int cntProblems_l(0);

  _logger->display_message("Merging problems...");
  for (auto const &kvp : input) {
    auto problem_name(inputRootDir / (kvp.first));
    SolverAbstract::Ptr solver_l = factory.create_solver(solver_to_use);
    solver_l->set_output_log_level(_options.LOG_LEVEL);

    if (kvp.first != _options.MASTER_NAME) {
      solver_l->read_prob_mps(problem_name);
      std::filesystem::remove(problem_name);
      int mps_ncols(solver_l->get_ncols());

      DblVector o(mps_ncols);
      IntVector sequence(mps_ncols);
      for (int i(0); i < mps_ncols; ++i) {
        sequence[i] = i;
      }
      solver_get_obj_func_coeffs(*solver_l, o, 0, mps_ncols - 1);
      double const weigth = slave_weight(nslaves, kvp.first);
      for (auto &c : o) {
        c *= weigth;
      }
      solver_l->chg_obj(sequence, o);
    } else {
      solver_l->read_prob_mps(problem_name);
    }
    StandardLp lpData(*solver_l);
    std::string varPrefix_l = "prob" + std::to_string(cntProblems_l) + "_";

    lpData.append_in(mergedSolver_l, varPrefix_l);

    for (auto const &x : kvp.second) {
      int col_index = mergedSolver_l->get_col_index(varPrefix_l + x.first);
      if (col_index == -1) {
        std::cerr << LOGLOCATION << "missing variable " << x.first << " in "
                  << kvp.first << " supposedly renamed to "
                  << varPrefix_l + x.first << ".";
        mergedSolver_l->write_prob_lp(
            std::filesystem::path(_options.OUTPUTROOT) / "mergeError.lp");
        mergedSolver_l->write_prob_mps(
            std::filesystem::path(_options.OUTPUTROOT) /
            ("mergeError" + MPS_SUFFIX));
        std::exit(1);
      } else {
        x_mps_id[x.first][kvp.first] = col_index;
      }
    }

    ++cntProblems_l;
  }

  IntVector mstart;
  IntVector cindex;
  DblVector values;
  int nrows(0);
  int neles(0);
  size_t neles_reserve(0);
  size_t nrows_reserve(0);
  for (auto const &kvp : x_mps_id) {
    neles_reserve += kvp.second.size() * (kvp.second.size() - 1);
    nrows_reserve += kvp.second.size() * (kvp.second.size() - 1) / 2;
  }
  _logger->display_message("About to add " + std::to_string(nrows_reserve) +
                           " coupling constraints");
  values.reserve(neles_reserve);
  cindex.reserve(neles_reserve);
  mstart.reserve(nrows_reserve + 1);

  // adding coupling constraints
  for (auto const &kvp : x_mps_id) {
    std::string const var_name(kvp.first);
    _logger->display_message(var_name);
    bool is_first(true);
    int id1(-1);
    std::string first_mps;
    for (auto const &mps : kvp.second) {
      if (is_first) {
        is_first = false;
        first_mps = mps.first;
        id1 = mps.second;
      } else {
        int id2 = mps.second;
        mstart.push_back(neles);
        cindex.push_back(id1);
        values.push_back(1);
        ++neles;

        cindex.push_back(id2);
        values.push_back(-1);
        ++neles;
        ++nrows;
      }
    }
  }
  mstart.push_back(neles);

  DblVector rhs(nrows, 0);
  CharVector sense(nrows, 'E');
  solver_addrows(*mergedSolver_l, sense, rhs, {}, mstart, cindex, values);

  _logger->display_message("Problems merged.");
  _logger->display_message("Writing mps file");
  mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT) /
                                 ("log_merged" + MPS_SUFFIX));
  _logger->display_message("Writing lp file");
  mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT) /
                                "log_merged.lp");

  mergedSolver_l->set_threads(16);

  _logger->display_message("Solving...");
  Timer timer;
  int status_l = 0;
  if (mergedSolver_l->get_n_integer_vars() > 0) {
    status_l = mergedSolver_l->solve_mip();
  } else {
    status_l = mergedSolver_l->solve_lp();
  }

  _logger->log_total_duration(timer.elapsed());

  Point x0;
  DblVector ptr(mergedSolver_l->get_ncols());
  double investCost_l(0);
  if (mergedSolver_l->get_n_integer_vars() > 0) {
    mergedSolver_l->get_mip_sol(ptr.data());
  } else {
    mergedSolver_l->get_lp_sol(ptr.data(), nullptr, nullptr);
  }

  std::vector<double> obj_coef(mergedSolver_l->get_ncols());
  mergedSolver_l->get_obj(obj_coef.data(), 0, mergedSolver_l->get_ncols() - 1);
  for (auto const &pairNameId : input[_options.MASTER_NAME]) {
    int varIndexInMerged_l = x_mps_id[pairNameId.first][_options.MASTER_NAME];
    x0[pairNameId.first] = ptr[varIndexInMerged_l];
    investCost_l += x0[pairNameId.first] * obj_coef[varIndexInMerged_l];
  }

  double overallCost_l;
  if (mergedSolver_l->get_n_integer_vars() > 0) {
    overallCost_l = mergedSolver_l->get_mip_value();
  } else {
    overallCost_l = mergedSolver_l->get_lp_value();
  }
  double operationalCost_l = overallCost_l - investCost_l;

  bool optimality_l = (status_l == SOLVER_STATUS::OPTIMAL);

  Output::SolutionData sol_infos;
  sol_infos.nbWeeks_p = static_cast<int>(input.size());

  sol_infos.solution.lb = overallCost_l;
  sol_infos.solution.ub = overallCost_l;
  sol_infos.solution.investment_cost = investCost_l;
  sol_infos.solution.operational_cost = operationalCost_l;
  sol_infos.solution.overall_cost = overallCost_l;

  Output::CandidatesVec candidates_vec;
  for (const auto &pairNameValue_l : x0) {
    Output::CandidateData candidate_data;
    candidate_data.name = pairNameValue_l.first;
    candidate_data.invest = pairNameValue_l.second;
    candidate_data.min = -1;
    candidate_data.max = -1;
    candidates_vec.push_back(candidate_data);
  }
  sol_infos.solution.candidates = candidates_vec;
  if (optimality_l) {
    sol_infos.problem_status = "OPTIMAL";
  } else {
    sol_infos.problem_status = "ERROR";
  }

  _writer->update_solution(sol_infos);
  _writer->dump();
}

/*!
 *  \brief Return slave weight value
 *
 *  \param nslaves : total number of slaves
 *
 *  \param name : slave name
 */
double MergeMPS::slave_weight(int nslaves, std::string const &name) const {
  if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_UNIFORM_CST_STR) {
    return 1 / static_cast<double>(nslaves);
  } else if (_options.SLAVE_WEIGHT == SUBPROBLEM_WEIGHT_CST_STR) {
    double const weight(_options.SLAVE_WEIGHT_VALUE);
    return 1 / weight;
  } else {
    return _options.weights.find(name)->second;
  }
}