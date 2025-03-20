#include "antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h"

#include <antares-xpansion/multisolver_interface/SolverConfig.h>
#include <fstream>
#include <numeric>
#include <ranges>

#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

YearlyWeightsWriter::YearlyWeightsWriter(
  const std::filesystem::path& xpansion_output_dir,
  const std::vector<double>& weights_vector,
  const std::filesystem::path& output_file,
  const std::vector<int>& active_years,
  const std::string& solver_name,
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger):
    xpansion_output_dir_(xpansion_output_dir),
    weights_vector_(weights_vector),
    output_file_(output_file),
    active_years_(active_years),
    solver_name_(solver_name),
    logger_(logger)
{
    xpansion_lp_dir_ = xpansion_output_dir / LP_DIR;
    if (!std::filesystem::is_directory(xpansion_lp_dir_))
    {
        std::filesystem::create_directory(xpansion_lp_dir_);
    }
}

void YearlyWeightsWriter::CreateWeightFile(
  const std::vector<std::pair<int, ProblemData>>& problems_and_data)
{
    FillMpsWeightsMap(problems_and_data);
    DumpMpsWeightsToFile();
}

void YearlyWeightsWriter::FillMpsWeightsMap(
  const std::vector<std::pair<int, ProblemData>>& problems_and_data)
{
    problem_filename_to_weight_.clear();

    for (const auto& [mc_year, data]: problems_and_data)
    {
        if (auto it = std::find(active_years_.begin(), active_years_.end(), mc_year);
            it != active_years_.end())
        {
            auto year_index = std::distance(active_years_.begin(), it);
            problem_filename_to_weight_[data._problem_filename] = weights_vector_[year_index];
        }
        else
        {
            std::ostringstream msg;
            msg << "Mc year " << mc_year << " not found in the list of active years." << std::endl;
            (*logger_)(LogUtils::LOGLEVEL::FATAL) << LOGLOCATION << msg.str();
            throw McYearNotInActiveYearsListError(msg.str(), LOGLOCATION);
        }
    }
}

int YearlyWeightsWriter::GetYearFromMpsName(const std::string& file_name) const
{
    auto split_name = StringManip::split(StringManip::trim(file_name), '-');
    return std::stoi(split_name[1]);
}

void YearlyWeightsWriter::DumpMpsWeightsToFile() const
{
    auto file = xpansion_lp_dir_ / output_file_.filename().string();
    std::ofstream mps_weights_file;
    mps_weights_file.open(file);

    for (const auto& [mps_name, weight]: problem_filename_to_weight_)
    {
        mps_weights_file << SolverConfig(solver_name_).FileName(mps_name.string()).string() << " "
                         << weight << std::endl;
    }
    mps_weights_file << "WEIGHT_SUM " << std::reduce(weights_vector_.begin(), weights_vector_.end())
                     << std::endl;
    mps_weights_file.close();
}
