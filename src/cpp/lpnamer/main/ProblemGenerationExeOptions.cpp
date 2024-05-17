#include "ProblemGenerationExeOptions.h"
namespace po = boost::program_options;
using namespace std::string_literals;

ProblemGenerationExeOptions::ProblemGenerationExeOptions()
    : OptionsParser("Problem Generation exe") {
  AddOptions()("help,h", "produce help message")(
      "output,o", po::value<std::filesystem::path>(&xpansion_output_dir_),
      "antares-xpansion study output")(
      "archive,a", po::value<std::filesystem::path>(&archive_path_),
      "antares-xpansion study zip")(
      "study", po::value<std::filesystem::path>(&study_path_), "antares study")(
      "formulation,f",
      po::value<std::string>(&master_formulation_)->default_value("relaxed"),
      "master formulation (relaxed or integer)")(
      "exclusion-files,e",
      po::value<std::string>(&additional_constraintFilename_l_),
      "path to exclusion files")(
      "weights-file,w",
      po::value<std::filesystem::path>(&weights_file_)->default_value(""),
      "user weights file")("unnamed-problems,n",
                           po::bool_switch(&unnamed_problems_),
                           "use this option if unnamed problems are provided");
}
void ProblemGenerationExeOptions::Parse(unsigned int argc,
                                        const char* const* argv) {
  OptionsParser::Parse(argc, argv);
  auto log_location = LOGLOCATION;
  checkMandatoryOptions(log_location);
}

auto ProblemGenerationExeOptions::exclusiveMandatoryParameters() const {
  return std::vector{this->XpansionOutputDir().string(),
           this->ArchivePath().string(),
           this->StudyPath().string()};
}

namespace {
auto notEmpty = [](auto k) { return !k.empty(); };
}
void ProblemGenerationExeOptions::checkMandatoryOptions(
    const std::string& log_location) const {
  auto args = exclusiveMandatoryParameters();
  auto count = std::ranges::count_if(args, notEmpty);
  if (count > 1) {
    auto msg = "Only one of [archive, output, study] parameters is accepted"s;
    throw ProblemGenerationOptions::ConflictingParameters(msg, log_location);
  }
  if (count == 0) {
    auto msg =
        "Need to give at least on of [OutputDir, Archive, Study] options"s;
    throw ProblemGenerationOptions::MissingParameters(msg, log_location);
  }
}

std::filesystem::path ProblemGenerationExeOptions::deduceArchivePathIfEmpty(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& archive_path) const {
  if (archive_path.empty() && !xpansion_output_dir.empty()) {
    if (xpansion_output_dir.string().find("-Xpansion") == std::string::npos) {
      auto log_location = LOGLOCATION;
      auto msg =
          "Archive path is missing and output path does not contains"s
          " \"-Xpansion\" suffixe. Can't deduce archive file name."s;
      throw MismatchedParameters(msg, log_location);
    }
    auto deduced_archive_path = xpansion_output_dir;
    auto dir_name = deduced_archive_path.stem().string();
    dir_name = dir_name.substr(0, dir_name.find("-Xpansion"s));
    deduced_archive_path =
        deduced_archive_path.replace_filename(dir_name).replace_extension(
            ".zip");
    return deduced_archive_path;
  }
  return archive_path;
}

std::filesystem::path ProblemGenerationExeOptions::deduceXpansionDirIfEmpty(
    std::filesystem::path xpansion_output_dir,
    const std::filesystem::path& archive_path) const {
  if (xpansion_output_dir.empty() && !archive_path.empty()) {
    auto deduced_dir = archive_path;
    deduced_dir = deduced_dir.replace_filename(
        deduced_dir.stem().replace_extension("").string() + "-Xpansion"s);
    return deduced_dir;
  }
  return xpansion_output_dir;
}
std::filesystem::path ProblemGenerationExeOptions::StudyPath() const {
  return study_path_;
}
