#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"

#include "antares-xpansion/core/ProblemFormat.h"
#include "antares-xpansion/core/ProblemFormatStream.h"

namespace po = boost::program_options;
using namespace std::string_literals;

ProblemGenerationExeOptions::ProblemGenerationExeOptions():
    OptionsParser("Problem Generation exe"s)
{
    AddOptions()("help,h",
                 "produce help message")("output,o",
                                         po::value<std::filesystem::path>(&xpansion_output_dir_),
                                         "antares-xpansion study output")(
      "archive,a",
      po::value<std::filesystem::path>(&archive_path_),
      "antares-xpansion study zip")("study",
                                    po::value<std::filesystem::path>(&study_path_),
                                    "antares study")(
      "formulation,f",
      po::value<std::string>(&master_formulation_)->default_value("relaxed"),
      "master formulation (relaxed or integer)")("exclusion-files,e",
                                                 po::value<std::string>(
                                                   &additional_constraintFilename_l_),
                                                 "path to exclusion files")(
      "weights-file,w",
      po::value<std::filesystem::path>(&weights_file_)->default_value(""),
      "user weights file")("unnamed-problems,n",
                           po::bool_switch(&unnamed_problems_),
                           "use this option if unnamed problems are provided")(
      "format",
      po::value<ProblemsFormat>(&format_)->default_value(ProblemsFormat::SAVED_FILE),
      "output format (MPS or SAVED)")

      ;
}

void ProblemGenerationExeOptions::Parse(unsigned int argc, const char* const* argv)
{
    OptionsParser::Parse(argc, argv);
    auto log_location = LOGLOCATION;
    checkMandatoryOptions(log_location);
}

auto ProblemGenerationExeOptions::exclusiveMandatoryParameters() const
{
    return std::vector{this->XpansionOutputDir().string(),
                       this->ArchivePath().string(),
                       this->StudyPath().string()};
}

namespace
{
auto notEmpty = [](const auto& k) { return !k.empty(); };
} // namespace

void ProblemGenerationExeOptions::checkMandatoryOptions(const std::string& log_location) const
{
    auto args = exclusiveMandatoryParameters();
    auto count = std::ranges::count_if(args, notEmpty);
    if (count > 1)
    {
        auto msg = "Only one of [archive, output, study] parameters is accepted"s;
        throw ProblemGenerationOptions::ConflictingParameters(msg, log_location);
    }
    if (count == 0)
    {
        auto msg = "Need to give at least on of [OutputDir, Archive, Study] options"s;
        throw ProblemGenerationOptions::MissingParameters(msg, log_location);
    }
}

std::filesystem::path ProblemGenerationExeOptions::deduceXpansionDirIfEmpty(
  std::filesystem::path xpansion_output_dir,
  const std::filesystem::path& archive_path) const
{
    if (xpansion_output_dir.empty() && !archive_path.empty())
    {
        auto deduced_dir = archive_path;
        deduced_dir = deduced_dir.replace_filename(deduced_dir.stem().replace_extension("").string()
                                                   + "-Xpansion"s);
        return deduced_dir;
    }
    return xpansion_output_dir;
}

std::filesystem::path ProblemGenerationExeOptions::StudyPath() const
{
    return study_path_;
}

std::filesystem::path ProblemGenerationExeOptions::getRelevantPath() const
{
    if (!study_path_.empty())
    {
        return study_path_;
    }
    if (!archive_path_.empty())
    {
        return archive_path_;
    }
    if (!xpansion_output_dir_.empty())
    {
        return xpansion_output_dir_;
    }
    throw LogUtils::XpansionError<std::runtime_error>("SimulationInputMode is unknown",
                                                      LOGLOCATION);
}

void ProblemGenerationExeOptions::setRelevantPath(const std::filesystem::path& path)
{
    if (!study_path_.empty())
    {
        study_path_ = path;
    }
    else if (!archive_path_.empty())
    {
        archive_path_ = path;
    }
    else if (!xpansion_output_dir_.empty())
    {
        xpansion_output_dir_ = path;
    }
    else
    {
        throw LogUtils::XpansionError<std::runtime_error>("SimulationInputMode is unknown",
                                                          LOGLOCATION);
    }
}
