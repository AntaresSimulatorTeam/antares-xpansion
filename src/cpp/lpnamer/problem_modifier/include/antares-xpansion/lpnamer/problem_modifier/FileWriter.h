#pragma once

#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

class FileWriter: public IProblemWriter
{
public:
    void Write_problem(Problem* in_prblm, const std::filesystem::path& output_file) override;
    FileWriter(std::filesystem::path lp_dir);
    std::filesystem::path lp_dir_;
};
