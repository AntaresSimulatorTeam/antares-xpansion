#pragma once
#include <filesystem>
#include <vector>

#include "antares-xpansion/helpers/OptionsParser.h"

class BellmanValuesExeOptions: public OptionsParser
{
private:
    std::filesystem::path studyPath_;
    std::string solverName_;
    int nbThreads_;
    int startWeek_;
    int endWeek_;
    int nbLevels_;
    bool antaresFormat_;
    bool writePbFiles_;
    std::string problemFormat_;
    bool useOptimalTrajectory_;
    std::string verbosity_;
    bool streamProblemsFromDisk_;

public:
    BellmanValuesExeOptions();

    virtual ~BellmanValuesExeOptions() = default;

    std::filesystem::path StudyPath() const
    {
        return studyPath_;
    }

    std::string SolverName() const
    {
        return solverName_;
    }

    int NbThreads() const
    {
        return nbThreads_;
    }

    int StartWeek() const
    {
        return startWeek_;
    }

    int EndWeek() const
    {
        return endWeek_;
    }

    int NbLevels() const
    {
        return nbLevels_;
    }

    bool AntaresFormat() const
    {
        return antaresFormat_;
    }

    bool WritePbFiles() const
    {
        return writePbFiles_;
    }

    std::string ProblemFormat() const
    {
        return problemFormat_;
    }

    bool UseOptimalTrajectory() const
    {
        return useOptimalTrajectory_;
    }

    std::string Verbosity() const
    {
        return verbosity_;
    }

    bool StreamProblemsFromDisk() const
    {
        return streamProblemsFromDisk_;
    }
};
