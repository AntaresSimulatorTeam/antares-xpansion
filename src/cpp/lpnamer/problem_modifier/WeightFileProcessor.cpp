
#include "antares-xpansion/lpnamer/problem_modifier/WeightFileProcessor.h"

#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/input_reader/WeightsFileReader.h"
#include "antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h"

void WeightFileProcessor::ProcessWeights(
  const std::vector<std::pair<int, ProblemData>>& year_and_data,
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& weights_file,
  const std::string& solver_name,
  const std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger)
{
    if (weights_file.empty())
    {
        return;
    }
    const auto settings_dir = xpansion_output_dir / ".." / ".." / "settings";
    const auto general_data_file = settings_dir / "generaldata.ini";
    auto genera_data_reader = GeneralDataIniReader(general_data_file, logger);
    auto active_years = genera_data_reader.GetActiveYears();
    WeightsFileReader weights_file_reader(weights_file, active_years.size(), logger);
    weights_file_reader.CheckWeightsFile();
    auto weights_vector = weights_file_reader.WeightsList();
    auto yearly_weight_writer = YearlyWeightsWriter(xpansion_output_dir,
                                                    weights_vector,
                                                    weights_file.filename(),
                                                    active_years,
                                                    solver_name,
                                                    logger);
    yearly_weight_writer.asmps = asmps; // Set the asmps flag if needed
    yearly_weight_writer.CreateWeightFile(year_and_data);
}

void WeightFileProcessor::ProcessWeights(
  const std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>& problems_and_data,
  const std::filesystem::path& xpansion_output_dir,
  const std::filesystem::path& weights_file,
  const std::string& solver_name,
  const std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger)
{
    if (weights_file.empty())
    {
        return;
    }
    std::vector<std::pair<int, ProblemData>> year_and_data;
    std::ranges::transform(problems_and_data,
                           std::back_inserter(year_and_data),
                           [](const std::pair<std::shared_ptr<Problem>, ProblemData>& pair)
                           { return std::pair{pair.first->mc_year, pair.second}; });
    ProcessWeights(year_and_data, xpansion_output_dir, weights_file, solver_name, logger);
}
