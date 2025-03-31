#pragma once
#include <antares-xpansion/lpnamer/model/Problem.h>
#include <filesystem>
#include <vector>

namespace ProblemGenerationLog
{
class ProblemGenerationLogger;
}

struct ProblemData;

class WeightFileProcessor
{
public:
    void ProcessWeights(const std::vector<std::pair<int, ProblemData>>& problems_and_data,
                        const std::filesystem::path& xpansion_output_dir,
                        const std::filesystem::path& weights_file,
                        const std::string& solver_name,
                        std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger);

    void ProcessWeights(
      const std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>& problems_and_data,
      const std::filesystem::path& xpansion_output_dir,
      const std::filesystem::path& weights_file,
      const std::string& solver_name,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger);
};
