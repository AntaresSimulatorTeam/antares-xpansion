#include "antares-xpansion/bellman_values/ProblemManager.h"

ProblemManager::ProblemManager(const std::string& solverName,
                               const std::string& problemFormat,
                               bool writePbFiles,
                               bool cacheProblems,
                               std::optional<std::filesystem::path> problemsPath):
    solverName_(solverName),
    problemFormat_(problemsFormatFromString(
      "OPTIMIZED")), // here, can be updated to choose the file format for real
    writePbFiles_(writePbFiles),
    cacheProblems_(cacheProblems),
    problemsPath_(problemsPath),
    solverLogManager_(),
    solverFactory_()
{
    if (solverName != "xpress" && problemFormat_ == ProblemsFormat::OPTIMIZED)
    {
        std::cout << "The optimized problem format is only compatible with Xpress solver. MPS "
                     "format will be used.\n";
        problemFormat_ = ProblemsFormat::MPS_FILE;
    }
    if (cacheProblems && problemsPath == std::nullopt)
    {
        throw std::runtime_error(
          "Error: trying to stream problems from disk without specify a folder.");
    }
    if (cacheProblems && !std::filesystem::exists(problemsPath.value()))
    {
        std::filesystem::create_directories(problemsPath.value());
    }
}

void ProblemManager::setProblems(
  std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>& problems)
{
    problems_.clear();
    problemIds.clear();
    if (cacheProblems_)
    {
        for (auto& [pbId, problem]: problems)
        {
            problemIds.emplace(pbId);
            saveProblemToFile(pbId, problem, problemsPath_.value());
        }
    }
    else
    {
        problems_ = std::move(problems);
        for (auto& [pbId, problem]: problems)
        {
            problemIds.emplace(pbId);
        }
    }
}

std::shared_ptr<Problem> ProblemManager::readProblemFromDisk(const std::string& problemName) const
{
    if (!cacheProblems_)
    {
        throw std::runtime_error("Error: trying to stream problem from disk.");
    }
    try
    {
        auto problem = std::make_shared<Problem>(solverFactory_.create_solver(
          solverName_ == SolverConfig("xpress")
            ? "xpress"
            : "CBC", // not optimal, but similar to what is done in ProblemGenerationForWaterValues
          solverLogManager_));
        // std::cout << "New problem created\n";
        switch (problemFormat_)
        {
        case ProblemsFormat::MPS_FILE:
            problem->read_prob_mps(problemsPath_.value() / (problemName + ".mps"));
            break;
        case ProblemsFormat::OPTIMIZED:
            problem->restore_prob(problemsPath_.value() / (problemName + ".svf"));
            break;
            // potential errors are handled by
            // problemsFormatFromString in constructor
        }
        return problem;
    }
    catch (const std::exception& e)
    {
        std::string message = "Error reading problem " + problemName + " from file:\n"
                              + std::string(e.what());
        throw std::runtime_error(message);
    }
}

void ProblemManager::saveProblemToFile(const Antares::Solver::WeeklyProblemId& pbId,
                                       std::shared_ptr<Problem> problem,
                                       std::filesystem::path& folder) const
{
    switch (problemFormat_)
    {
    case ProblemsFormat::MPS_FILE:
        problem->write_prob_mps(folder / (getPbNameFromId(pbId) + ".mps"));
        // std::cout << "Problem saved to " + folder.string() + "/" + getPbNameFromId(pbId)
        //                + ".mps\n";
        break;
    case ProblemsFormat::OPTIMIZED:
        problem->save_prob(folder / (getPbNameFromId(pbId) + ".svf"));
        // std::cout << "Problem saved to " + folder.string() + "/" + getPbNameFromId(pbId)
        //                + ".svf\n";
        break;
        // potential errors are handled by
        // problemsFormatFromString in constructor
    }
}
