#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"

struct SolverRepresentedRows
{
    std::vector<int> mstart;
    std::vector<int> mclind;
    std::vector<double> dmatval;
    std::vector<double> range_p;
    std::vector<char> qrtype_p;
    std::vector<double> rhs;
    std::vector<std::string> row_names;
};

class ConstraintsFileReader
{
public:
    ConstraintsFileReader(const std::filesystem::path& constraint_file_path ,
                          const std::string& solver_name,
                          const SolverLogManager& solver_log_manager,
                          Logger& logger,
                          int log_level,
                          ProblemsFormat format);

    ConstraintsFileReader(std::shared_ptr<SolverAbstract> solver);

    SolverRepresentedRows get_row(const std::string& name);

private:
    int get_row_index(const std::string& name);

    Logger logger_;
    std::shared_ptr<SolverAbstract> solver_;
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_;
    SolverIO solver_IO_;
};
