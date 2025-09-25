#pragma once
#include "LinkProblemsGenerator.h"

class StructureGeneration
{
public:
    StructureGeneration(std::filesystem::path output_path,
                        std::string solver_name,
                        ProblemsFormat format = ProblemsFormat::OPTIMIZED);

    void operator()(const std::vector<Candidate>& candidates, const Couplings& couplings) const;

private:
    void write_structure_file(const std::vector<Candidate>& candidates,
                              const Couplings& couplings) const;
    std::string solver_name_;
    ProblemsFormat format_;
    std::filesystem::path output_path_;
};
