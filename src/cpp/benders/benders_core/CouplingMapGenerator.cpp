#include <antares-xpansion/benders/benders_core/CouplingMapGenerator.h>
#include <antares-xpansion/xpansion_interfaces/LoggerUtils.h>

struct InvalidStructureFile : LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

/*!
 *  \brief Build the input from the structure file
 *
 *	Function to build the map linking each problem name to its variables and
 *their id
 *
 *  \param root : root of the structure file
 *
 *  \param summary_name : name of the structure file
 *
 *  \param coupling_map : empty map to increment
 *
 *  \note The id in the coupling_map is that of the variable in the solver
 *responsible for the creation of the structure file.
 */
CouplingMap CouplingMapGenerator::BuildInput(
    const std::filesystem::path& structure_path,
    std::shared_ptr<ILoggerXpansion> logger, const std::string& context) {
  CouplingMap coupling_map;
  std::ifstream summary(structure_path, std::ios::in);
  if (!summary) {
    auto log_location = LOGLOCATION;
    std::ostringstream msg;
    msg << "Cannot open structure file " << structure_path << std::endl;
    logger->display_message(msg.str(), LogUtils::LOGLEVEL::FATAL, log_location);
    throw InvalidStructureFile(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, context), msg.str(),
        log_location);
  }
  std::string line;

  while (std::getline(summary, line)) {
    std::stringstream buffer(line);
    std::string problem_name;
    std::string variable_name;
    int variable_id;
    buffer >> problem_name;
    buffer >> variable_name;
    buffer >> variable_id;
    coupling_map[problem_name][variable_name] = variable_id;
  }

  summary.close();
  return coupling_map;
}