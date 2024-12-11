#include <antares-xpansion/lpnamer/input_reader/SettingsReader.h>

#include <regex>

#include "antares-xpansion/lpnamer/input_reader/Exceptions.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

SettingsReader::SettingsReader(const std::filesystem::path& file_path,
                               ProblemGenerationLog::ProblemGenerationLogger* logger)
  : logger_(logger)
{
  check_file_exist(file_path);

  parse_file(file_path);
  settings_.try_emplace("solver", "CBC");
  solver_name_ = settings_.at("solver");

}
void SettingsReader::parse_file(const std::filesystem::path& file_path) {
  std::ifstream file(file_path);
  std::string line;
  std::regex regex_token("=");

  while (std::getline(file, line)) {
    if (std::find_if(line.begin(), line.end(), [](auto c) { return !std::isspace(c); }) == line.end()) {
      continue; // skip empty lines
    }
    std::vector<std::string> values;
    std::sregex_token_iterator it(line.begin(), line.end(), regex_token, -1);
    std::copy(it, std::sregex_token_iterator(), std::back_inserter(values));
    if (values.size() != 2) {
      std::ostringstream msg;
      msg << "Invalid line in settings file : " << line
          << std::endl;
      (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << msg.str();
      throw LogUtils::XpansionError<std::runtime_error>(msg.str(), LOGLOCATION);
    }
    this->settings_[StringManip::trim(values[0])] = StringManip::trim(values[1]);
  }
}
void SettingsReader::check_file_exist(
    const std::filesystem::path& file_path) const {
  if (file_path.empty() || !std::filesystem::exists(file_path)) {
    std::ostringstream msg;
    msg << LOGLOCATION
        << "Settings file is not found : " << file_path.string()
        << std::endl;
    (*this->logger_)(LogUtils::LOGLEVEL::FATAL) << msg.str();
    throw IniFileNotFound(msg.str());
  }
}
std::string SettingsReader::Solver() {
  return solver_name_;
}