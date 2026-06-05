#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>

#include <antares/solver/lps/LpsFromAntares.h>

#include "antares-xpansion/core/ProblemFormat.h"
#include "antares-xpansion/lpnamer/model/Problem.h"

class ProblemManager
{
public:
    // TODO: add a logger
    ProblemManager(const std::string& solverName = "xpress",
                   const std::string& problemFormat = "OPTIMIZED",
                   bool writePbFiles = false,
                   bool cacheProblems = false,
                   std::optional<std::filesystem::path> problemsPath = std::nullopt);

    ProblemManager(const ProblemManager& problemManagerToCopy);

    ~ProblemManager();

    /**
     * @brief Get the Problems object, either read from disk or as stored in memory.
     *
     * @return std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
     */
    const std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>& getProblems() const
    {
        return problems_;
        // todo: read from disk
    }

    std::set<Antares::Solver::WeeklyProblemId> getProblemIds() const
    {
        return problemIds;
    }

    /**
     * @brief Set the Problems object
     *
     * @param problems a map of problems, moved from their origin
     */
    void setProblems(
      std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>& problems);

    bool writePbFiles() const
    {
        return writePbFiles_;
    }

    ProblemsFormat problemFormat() const
    {
        return problemFormat_;
    }

    std::string solverName() const
    {
        return solverName_;
    }

    SolverLogManager& solverLogManager()
    {
        return solverLogManager_;
    }

    std::string getPbNameFromId(const Antares::Solver::WeeklyProblemId& pbId) const
    {
        return "problem-" + std::to_string(pbId.year) + "-" + std::to_string(pbId.week)
               + "--optim-nb-1";
    }

    std::shared_ptr<Problem> getProblemFromId(const Antares::Solver::WeeklyProblemId& pbId) const
    {
        if (cacheProblems_)
        {
            return readProblemFromDisk(getPbNameFromId(pbId));
        }
        else
        {
            return problems_.at(pbId);
        }
    }

    std::shared_ptr<Problem> getProblemCloneFromId(
      const Antares::Solver::WeeklyProblemId& pbId) const
    {
        if (cacheProblems_)
        {
            return readProblemFromDisk(getPbNameFromId(pbId));
        }
        else
        {
            return std::shared_ptr<Problem>(problems_.at(pbId)->clone());
        }
    }

    void setProblem(const Antares::Solver::WeeklyProblemId& pbId, std::shared_ptr<Problem> pb)
    {
        if (cacheProblems_)
        {
            problemIds.emplace(pbId);
            saveProblemToFile(pbId, pb, problemsPath_.value());
        }
        else
        {
            problems_[pbId] = pb;
            problemIds.emplace(pbId);
        }
    }

    void setProblemsPath(const std::filesystem::path& problemsPath)
    {
        problemsPath_ = problemsPath;
    }

    void saveProblemToFile(const Antares::Solver::WeeklyProblemId& pbId,
                           std::shared_ptr<Problem> problem,
                           std::filesystem::path& folder) const;

private:
    std::shared_ptr<Problem> readProblemFromDisk(const std::string& problemName) const;

    bool cacheProblems_ = false; // for optimization purposes, problems can be read
                                 // from/saved on disk instead of in memory
    bool writePbFiles_ = false;  // save problem files to disk, for analysis purposes for example
    std::string solverName_ = "xpress";
    ProblemsFormat problemFormat_ = ProblemsFormat::OPTIMIZED; // can be MPS_FILE or OPTIMIZED (SVF)
    std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
      problems_;                                           // a map storing all problems
    std::set<Antares::Solver::WeeklyProblemId> problemIds; // a set holding all problem Ids
    std::optional<std::filesystem::path>
      problemsPath_ = std::nullopt;     // needed only when reading problems from disk
    SolverFactory solverFactory_;       // needed to construct problems from disk
    SolverLogManager solverLogManager_; // needed to construct problems from disk
};
