#pragma once

#include "antares-xpansion/core/ProblemFormat.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

class FileWriter: public IProblemWriter
{
public:
    void Write_problem(Problem* in_prblm, const std::filesystem::path& output_file) override;
    FileWriter(std::filesystem::path lp_dir, ProblemsFormat format = ProblemsFormat::SAVED_FILE);
    std::filesystem::path lp_dir_;
    ProblemsFormat format_;
};
