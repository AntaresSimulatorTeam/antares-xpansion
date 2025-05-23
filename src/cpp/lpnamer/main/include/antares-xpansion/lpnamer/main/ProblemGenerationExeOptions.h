#ifndef ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
#define ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
#include <filesystem>
#include <vector>

#include "ProblemGenerationOptions.h"
#include "antares-xpansion/helpers/OptionsParser.h"

class ProblemGenerationExeOptions: public OptionsParser, public ProblemGenerationOptions
{
private:
    std::filesystem::path xpansion_output_dir_;
    std::string master_formulation_;
    std::string additional_constraintFilename_l_;
    std::filesystem::path archive_path_;
    std::filesystem::path weights_file_;
    std::vector<int> active_years_;
    bool unnamed_problems_ = false;
    std::filesystem::path study_path_;

public:
    ProblemGenerationExeOptions();

    ~ProblemGenerationExeOptions() override = default;

    [[nodiscard]] std::filesystem::path XpansionOutputDir() const override
    {
        return xpansion_output_dir_;
    }

    [[nodiscard]] std::string MasterFormulation() const override
    {
        return master_formulation_;
    }

    [[nodiscard]] std::string AdditionalConstraintsFilename() const override
    {
        return additional_constraintFilename_l_;
    }

    [[nodiscard]] std::filesystem::path ArchivePath() const override
    {
        return archive_path_;
    }

    [[nodiscard]] std::filesystem::path WeightsFile() const override
    {
        return weights_file_;
    }

    [[nodiscard]] std::vector<int> ActiveYears() const override
    {
        return active_years_;
    }

    [[nodiscard]] bool UnnamedProblems() const override
    {
        return unnamed_problems_;
    }

    void Parse(unsigned int argc, const char* const* argv) override;

    [[nodiscard]] std::filesystem::path deduceXpansionDirIfEmpty(
      std::filesystem::path xpansion_output_dir,
      const std::filesystem::path& archive_path) const override;
    [[nodiscard]] std::filesystem::path StudyPath() const override;
    void checkMandatoryOptions(const std::string& log_location) const;
    [[nodiscard]] auto exclusiveMandatoryParameters() const;

    // Returns the relevant path (should be the only non-empty one)
    std::filesystem::path getRelevantPath() const;
    // Sets the relevant path (should be the only non-empty one before changing the value)
    void setRelevantPath(const std::filesystem::path& path);
};
#endif // ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONEXEOPTIONS_H
