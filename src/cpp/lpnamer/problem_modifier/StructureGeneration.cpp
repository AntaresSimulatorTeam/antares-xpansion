
#include "antares-xpansion/lpnamer/problem_modifier/StructureGeneration.h"

#include <utility>

namespace
{
using ProblemName = std::string;
using CandidateName = std::string;
using Structure = std::map<ProblemName, std::map<CandidateName, ColId>>;
} // namespace

std::filesystem::path FileNameForStructureFile(const std::string& problemName,
                                               std::string solverName,
                                               ProblemsFormat format)
{
    if (problemName == "master")
    {
        return {"master"};
    }
    // Force mps file extension for MPS format
    auto fileName = SolverConfig(std::move(solverName)).FileName(problemName);
    if (format == ProblemsFormat::MPS_FILE)
    {
        return fileName.replace_extension(".mps");
    }
    return fileName;
}

StructureGeneration::StructureGeneration(std::filesystem::path output_path,
                                         std::string solver_name,
                                         ProblemsFormat format):
    output_path_(std::move(output_path)),
    solver_name_(std::move(solver_name)),
    format_(format)
{
}

void StructureGeneration::operator()(const std::vector<Candidate>& candidates,
                                     const Couplings& couplings) const
{
    write_structure_file(candidates, couplings);
}

void StructureGeneration::write_structure_file(const std::vector<Candidate>& candidates,
                                               const Couplings& couplings) const
{
    Structure structure;
    for (const auto& [candidate_name_and_mps_filePath, colId]: couplings)
    {
        structure[candidate_name_and_mps_filePath.second][candidate_name_and_mps_filePath.first]
          = colId;
    }
    unsigned int i = 0;
    for (const auto& candidate: candidates)
    {
        structure["master"][candidate.get_name()] = i;
        ++i;
    }

    std::ofstream coupling_file(output_path_ / "lp" / STRUCTURE_FILE);
    for (const auto& [mps_file_path, candidates_name_and_colId]: structure)
    {
        for (const auto& [candidate_name, colId]: candidates_name_and_colId)
        {
            coupling_file << std::setw(
              50) << FileNameForStructureFile(mps_file_path, solver_name_, format_).string();
            coupling_file << std::setw(50) << candidate_name;
            coupling_file << std::setw(10) << colId;
            coupling_file << std::endl;
        }
    }
    coupling_file.close();
}
