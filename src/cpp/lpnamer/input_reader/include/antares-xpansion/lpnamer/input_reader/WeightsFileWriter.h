#ifndef SRC_CPP_LPNAMER_INPUTREADER_YEARLYWEIGHTSWRITER_H
#define SRC_CPP_LPNAMER_INPUTREADER_YEARLYWEIGHTSWRITER_H
#include <antares-xpansion/lpnamer/model/Problem.h>
#include <filesystem>
#include <map>
#include <vector>

#include "MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

class YearlyWeightsWriter
{
public:
    explicit YearlyWeightsWriter(
      const std::filesystem::path& xpansion_output_dir,
      const std::vector<double>& weights_vector,
      const std::filesystem::path& output_file,
      const std::vector<int>& active_years,
      const std::string& solver_name,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger);

    void CreateWeightFile(const std::vector<std::pair<int, ProblemData>>& problems_and_data);

    class McYearNotInActiveYearsListError: public LogUtils::XpansionError<std::runtime_error>
    {
        using LogUtils::XpansionError<std::runtime_error>::XpansionError;
    };

private:
    std::filesystem::path xpansion_output_dir_;
    std::filesystem::path xpansion_lp_dir_ = "";
    std::filesystem::path antares_archive_path_;
    const std::string LP_DIR = "lp";
    std::map<std::filesystem::path, double> problem_filename_to_weight_ = {};
    std::vector<double> weights_vector_;
    std::filesystem::path output_file_;
    std::vector<int> active_years_;
    std::string solver_name_;
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
    void FillMpsWeightsMap(const std::vector<std::pair<int, ProblemData>>& problems_and_data);
    int GetYearFromMpsName(const std::string& file_name) const;
    void DumpMpsWeightsToFile() const;
};
#endif // SRC_CPP_LPNAMER_INPUTREADER_YEARLYWEIGHTSWRITER_H
