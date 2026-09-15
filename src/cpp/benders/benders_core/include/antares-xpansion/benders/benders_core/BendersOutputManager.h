#pragma once

#include <filesystem>
#include <fstream>
#include <memory>

#include "BendersMathLogger.h"
#include "BendersStructsDatas.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

namespace Output
{
class OutputWriter;
}

class BendersOutputManager
{
public:
    BendersOutputManager(Logger logger,
                         std::shared_ptr<Output::OutputWriter> writer,
                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver,
                         const CurrentIterationData& data,
                         const BendersBaseOptions& options,
                         const VariableMap& problem_to_id,
                         const BendersRelevantIterationsData& relevant_iteration_data);

    [[nodiscard]] Logger GetLogger() const;
    [[nodiscard]] std::shared_ptr<Output::OutputWriter> GetWriter() const;
    [[nodiscard]] std::shared_ptr<MathLoggerDriver> GetMathLoggerDriver() const;

    // CSV trace
    void OpenCsvFile();
    void CloseCsvFile();

    // Per-iteration orchestration
    void SaveCurrentBendersData(const std::filesystem::path& last_iteration_file, bool trace);

    // End of run
    void EndWritingInOutputFile(double benders_time, bool do_outer_loop);
    void PostRunActions(StoppingCriterion stopping_criterion);

    // MathLogger
    void MathLoggerPrint();
    void MathLoggerWriteHeader();

    // Data conversion
    [[nodiscard]] LogData bendersDataToLogData(const CurrentIterationData& data) const;
    [[nodiscard]] Output::Iteration iteration(const WorkerMasterData& masterDataPtr_l) const;
    [[nodiscard]] Output::SolutionData BuildSolution(int totalNbProblems) const;
    [[nodiscard]] LogData BuildFinalLogData() const;

    // Resume support
    void LoadResumeData(const std::filesystem::path& last_iteration_file, Logger& logger);
    [[nodiscard]] int GetNumIterationsBeforeRestart() const;
    [[nodiscard]] int GetNumOfSubProblemsSolvedBeforeResume() const;
    void UpdateBestIterationData(const LogData& data);
    [[nodiscard]] const LogData& GetBestIterationData() const;
    void SetIterationsBeforeResume(int value);
    void SetCumulativeSubproblemsSolvedBeforeResume(int value);

    // Setup
    void WriteNbWeeks(int total_nb_problems);

private:
    void SaveCurrentIterationInOutputFile() const;
    void SaveSolutionInOutputFile() const;
    void PrintCurrentIterationCsv();
    void print_master_and_cut(std::ostream& file,
                              int ite,
                              WorkerMasterData& trace,
                              const Point& xopt);
    void print_master_csv(std::ostream& stream,
                          const WorkerMasterData& trace,
                          const Point& x_cut) const;

    [[nodiscard]] Output::SolutionData BendersSolution(int totalNbProblems) const;
    [[nodiscard]] std::string status_from_criterion() const;
    [[nodiscard]] LogData FinalLogData() const;

    Logger logger_;
    std::shared_ptr<Output::OutputWriter> writer_;
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver_;

    const CurrentIterationData& data_;
    const BendersBaseOptions& options_;
    const VariableMap& problem_to_id_;
    const BendersRelevantIterationsData& relevant_iteration_data_;

    LogData best_iteration_data_;
    int iterations_before_resume_ = 0;
    int cumulative_number_of_subproblem_resolved_before_resume_ = 0;

    std::ofstream csv_file_;
    std::filesystem::path csv_file_path_;
};
