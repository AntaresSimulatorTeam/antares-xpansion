#include "antares-xpansion/benders/benders_core/BendersOutputManager.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <utility>

#include "antares-xpansion/benders/benders_core/LastIterationPrinter.h"
#include "antares-xpansion/benders/benders_core/LastIterationReader.h"
#include "antares-xpansion/benders/benders_core/LastIterationWriter.h"
#include "antares-xpansion/benders/output/OutputWriter.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

BendersOutputManager::BendersOutputManager(
  Logger logger,
  std::shared_ptr<Output::OutputWriter> writer,
  std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
  const CurrentIterationData& data,
  const BendersBaseOptions& options,
  const VariableMap& problem_to_id,
  const BendersRelevantIterationsData& relevant_iteration_data):
    logger_(std::move(logger)),
    writer_(std::move(writer)),
    mathLoggerDriver_(std::move(mathLoggerDriver)),
    data_(data),
    options_(options),
    problem_to_id_(problem_to_id),
    relevant_iteration_data_(relevant_iteration_data),
    csv_file_path_(std::filesystem::path(options_.OUTPUTROOT) / (options_.CSV_NAME + ".csv"))
{
}

Logger BendersOutputManager::GetLogger() const
{
    return logger_;
}

std::shared_ptr<Output::OutputWriter> BendersOutputManager::GetWriter() const
{
    return writer_;
}

std::shared_ptr<MathLoggerDriver> BendersOutputManager::GetMathLoggerDriver() const
{
    return mathLoggerDriver_;
}

// CSV trace

void BendersOutputManager::OpenCsvFile()
{
    if (!csv_file_.is_open())
    {
        const auto opening_mode = options_.RESUME ? std::ios::app : std::ios::trunc;
        csv_file_.open(csv_file_path_, std::ios::out | opening_mode);
        if (csv_file_ && !options_.RESUME)
        {
            csv_file_ << "Ite;Worker;Problem;Id;UB;LB;bestUB;simplexiter;jump;single_"
                         "subpb_costs_under_approx;"
                         "time;basis;"
                      << std::endl;
        }
        else
        {
            using namespace std::string_literals;
            logger_->display_message("Impossible to open the .csv file: "s
                                     + csv_file_path_.string());
        }
    }
}

void BendersOutputManager::CloseCsvFile()
{
    if (csv_file_.is_open())
    {
        csv_file_.close();
    }
}

void BendersOutputManager::PrintCurrentIterationCsv()
{
    if (relevant_iteration_data_.last._valid)
    {
        auto ite = data_.control.it - 1;
        Point x_cut;
        if (ite == 0)
        {
            int best_it_index = data_.control.best_it - 1;
            if (best_it_index >= 0)
            {
                x_cut = relevant_iteration_data_.best.get_x_cut();
            }
        }
        else
        {
            x_cut = relevant_iteration_data_.last.get_x_cut();
        }
        print_master_and_cut(csv_file_,
                             ite + 1 + iterations_before_resume_,
                             const_cast<WorkerMasterData&>(relevant_iteration_data_.last),
                             x_cut);
    }
}

static void print_cut_csv(std::ostream& stream,
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

void BendersOutputManager::print_master_and_cut(std::ostream& file,
                                                int ite,
                                                WorkerMasterData& trace,
                                                const Point& x_cut)
{
    file << ite << ";";
    print_master_csv(file, trace, x_cut);

    for (auto& [subproblem_name, subproblem_data]: trace._cut_trace)
    {
        auto problem_id = problem_to_id_.at(subproblem_name);
        file << ite << ";";
        print_cut_csv(file,
                      subproblem_data,
                      subproblem_name,
                      problem_id,
                      data_.master.single_subpb_costs_under_approx[problem_id]);
    }
}

void BendersOutputManager::print_master_csv(std::ostream& stream,
                                            const WorkerMasterData& trace,
                                            const Point& x_cut) const
{
    stream << "Master" << ";";
    stream << options_.MASTER_NAME << ";";
    stream << data_.control.nsubproblem << ";";
    stream << trace._ub << ";";
    stream << trace._lb << ";";
    stream << trace._best_ub << ";";
    stream << ";";
    stream << norm_point(x_cut, trace.get_x_cut()) << ";";
    stream << ";";
    stream << trace._master_duration << ";";
    stream << std::endl;
}

// OutputWriter

void BendersOutputManager::SaveCurrentIterationInOutputFile() const
{
    if (!options_.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP)
    {
        auto& LastWorkerMasterData = relevant_iteration_data_.last;
        if (LastWorkerMasterData._valid)
        {
            writer_->write_iteration(iteration(LastWorkerMasterData),
                                     data_.control.it + iterations_before_resume_);
            writer_->dump();
        }
    }
}

void BendersOutputManager::SaveSolutionInOutputFile() const
{
    writer_->write_solution(BuildSolution(0));
    writer_->dump();
}

void BendersOutputManager::EndWritingInOutputFile(double benders_time, bool do_outer_loop)
{
    writer_->updateEndTime();
    writer_->write_duration(benders_time);
    if (!do_outer_loop)
    {
        SaveSolutionInOutputFile();
    }
}

void BendersOutputManager::PostRunActions(StoppingCriterion stopping_criterion)
{
    LogData logData = BuildFinalLogData();
    logger_->log_stop_criterion_reached(stopping_criterion);
    logger_->log_at_ending(logData);
}

// MathLogger

void BendersOutputManager::MathLoggerPrint()
{
    mathLoggerDriver_->Print(data_);
}

void BendersOutputManager::MathLoggerWriteHeader()
{
    mathLoggerDriver_->write_header();
}

// Per-iteration orchestration

void BendersOutputManager::SaveCurrentBendersData(const std::filesystem::path& last_iteration_file,
                                                  bool trace)
{
    LastIterationWriter last_iteration_writer(last_iteration_file);
    const auto last = (data_.control.it == best_iteration_data_.it) ? best_iteration_data_
                                                                    : bendersDataToLogData(data_);
    last_iteration_writer.SaveBestAndLastIterations(best_iteration_data_, last);
    SaveCurrentIterationInOutputFile();
    if (trace)
    {
        PrintCurrentIterationCsv();
    }
}

// Data conversion

static Output::CandidatesVec candidates_data(const WorkerMasterData& masterDataPtr_l)
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

Output::Iteration BendersOutputManager::iteration(const WorkerMasterData& masterDataPtr_l) const
{
    Output::Iteration iter;
    iter.master_duration = masterDataPtr_l._master_duration;
    iter.subproblem_duration = masterDataPtr_l._subproblem_duration;
    iter.lb = masterDataPtr_l._lb;
    iter.ub = masterDataPtr_l._ub;
    iter.best_ub = masterDataPtr_l._best_ub;
    iter.optimality_gap = masterDataPtr_l._best_ub - masterDataPtr_l._lb;
    iter.relative_gap = (masterDataPtr_l._best_ub - masterDataPtr_l._lb) / masterDataPtr_l._best_ub;
    iter.investment_cost = masterDataPtr_l._invest_cost;
    iter.operational_cost = masterDataPtr_l._operational_cost;
    iter.overall_cost = masterDataPtr_l._invest_cost + masterDataPtr_l._operational_cost;
    iter.candidates = candidates_data(masterDataPtr_l);
    iter.cumulative_number_of_subproblem_resolved
      = data_.control.cumulative_number_of_subproblem_solved
        + cumulative_number_of_subproblem_resolved_before_resume_;
    return iter;
}

Output::SolutionData BendersOutputManager::BuildSolution(int totalNbProblems) const
{
    auto solution_data = BendersSolution(totalNbProblems);
    solution_data.best_it = data_.control.best_it + iterations_before_resume_;
    return solution_data;
}

Output::SolutionData BendersOutputManager::BendersSolution(int totalNbProblems) const
{
    Output::SolutionData solution_data;
    solution_data.nbWeeks_p = totalNbProblems;
    solution_data.problem_status = status_from_criterion();
    const auto optimal_gap(data_.control.best_ub - data_.master.lb);
    const auto relative_gap(optimal_gap / data_.control.best_ub);

    if (options_.RESUME)
    {
        Output::CandidatesVec candidates_vec;
        std::transform(best_iteration_data_.x_cut.cbegin(),
                       best_iteration_data_.x_cut.cend(),
                       std::back_inserter(candidates_vec),
                       [this](
                         const std::pair<std::string, double>& name_invest) -> Output::CandidateData
                       {
                           const auto& [name, invest] = name_invest;
                           return {name,
                                   invest,
                                   best_iteration_data_.min_invest.at(name),
                                   best_iteration_data_.max_invest.at(name)};
                       });
        solution_data.solution = {best_iteration_data_.master_time,
                                  best_iteration_data_.subproblem_time,
                                  best_iteration_data_.lb,
                                  best_iteration_data_.ub,
                                  best_iteration_data_.best_ub,
                                  optimal_gap,
                                  relative_gap,
                                  best_iteration_data_.invest_cost,
                                  best_iteration_data_.subproblem_cost,
                                  best_iteration_data_.invest_cost
                                    + best_iteration_data_.subproblem_cost,
                                  candidates_vec,
                                  0};
    }
    else
    {
        const auto& best_iteration_worker_master_data = relevant_iteration_data_.best;
        solution_data.solution = iteration(best_iteration_worker_master_data);
        solution_data.solution.optimality_gap = optimal_gap;
        solution_data.solution.relative_gap = relative_gap;
    }
    solution_data.stopping_criterion = criterion_to_str(data_.control.stopping_criterion);
    return solution_data;
}

std::string BendersOutputManager::status_from_criterion() const
{
    switch (data_.control.stopping_criterion)
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

LogData BendersOutputManager::bendersDataToLogData(const CurrentIterationData& data) const
{
    auto optimal_gap(data.control.best_ub - data.master.lb);
    return {data.master.lb,
            data.control.best_ub,
            data.cuts.ub,
            data.control.it + iterations_before_resume_,
            data.control.best_it + iterations_before_resume_,
            data.cuts.subproblem_cost,
            data.master.invest_cost,
            data.solution.x_in,
            data.solution.x_out,
            data.solution.x_cut,
            data.solution.min_invest,
            data.solution.max_invest,
            optimal_gap,
            optimal_gap / data.control.best_ub,
            options_.MAX_ITERATIONS,
            data.control.benders_time,
            data.master.timer_master,
            data.cuts.subproblems_walltime,
            data.control.cumulative_number_of_subproblem_solved
              + cumulative_number_of_subproblem_resolved_before_resume_};
}

LogData BendersOutputManager::BuildFinalLogData() const
{
    auto logData = FinalLogData();
    logData.optimality_gap = options_.ABSOLUTE_GAP;
    logData.relative_gap = options_.RELATIVE_GAP;
    logData.max_iterations = options_.MAX_ITERATIONS;
    return logData;
}

LogData BendersOutputManager::FinalLogData() const
{
    LogData result;
    result.it = data_.control.it + iterations_before_resume_;
    result.best_it = data_.control.best_it + iterations_before_resume_;

    result.subproblem_cost = best_iteration_data_.subproblem_cost;
    result.invest_cost = best_iteration_data_.invest_cost;
    result.cumulative_number_of_subproblem_resolved
      = data_.control.cumulative_number_of_subproblem_solved
        + cumulative_number_of_subproblem_resolved_before_resume_;

    return result;
}

// Resume support

void BendersOutputManager::LoadResumeData(const std::filesystem::path& last_iteration_file,
                                          Logger& logger)
{
    auto reader = LastIterationReader(last_iteration_file);
    LogData last_iter;
    if (reader.IsLastIterationFileValid())
    {
        const auto [lastIter, bestIter] = reader.LastIterationData();
        best_iteration_data_ = bestIter;
        last_iter = lastIter;
    }
    else
    {
        best_iteration_data_ = bendersDataToLogData(data_);
        last_iter = best_iteration_data_;
    }
    auto restart_data_printer = LastIterationPrinter(logger, best_iteration_data_, last_iter);
    restart_data_printer.Print();
    iterations_before_resume_ = last_iter.it;
    cumulative_number_of_subproblem_resolved_before_resume_
      = last_iter.cumulative_number_of_subproblem_resolved;
}

int BendersOutputManager::GetNumIterationsBeforeRestart() const
{
    return iterations_before_resume_;
}

int BendersOutputManager::GetNumOfSubProblemsSolvedBeforeResume() const
{
    return cumulative_number_of_subproblem_resolved_before_resume_;
}

void BendersOutputManager::UpdateBestIterationData(const LogData& data)
{
    best_iteration_data_ = data;
}

const LogData& BendersOutputManager::GetBestIterationData() const
{
    return best_iteration_data_;
}

void BendersOutputManager::SetIterationsBeforeResume(int value)
{
    iterations_before_resume_ = value;
}

void BendersOutputManager::SetCumulativeSubproblemsSolvedBeforeResume(int value)
{
    cumulative_number_of_subproblem_resolved_before_resume_ = value;
}

// Setup

void BendersOutputManager::WriteNbWeeks(int total_nb_problems)
{
    writer_->write_nbweeks(total_nb_problems);
}
