#pragma once

#include "antares-xpansion/core/ProblemFormat.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

class FileWriter: public IProblemWriter
{
public:
    void Write_problem(Problem* in_prblm, const std::filesystem::path& output_file) override;
    explicit FileWriter(ProblemsFormat format = ProblemsFormat::OPTIMIZED);
    ProblemsFormat format_;
};
