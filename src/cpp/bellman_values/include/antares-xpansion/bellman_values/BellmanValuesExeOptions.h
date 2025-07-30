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
};
