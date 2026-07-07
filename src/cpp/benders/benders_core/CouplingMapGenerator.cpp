#include <antares-xpansion/benders/benders_core/CouplingMapGenerator.h>
#include <antares-xpansion/xpansion_interfaces/LoggerUtils.h>

struct InvalidStructureFile: LogUtils::XpansionError<std::runtime_error>
{
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
CouplingMap CouplingMapGenerator::BuildInput(const std::filesystem::path& structure_path,
                                             ILoggerXpansion* logger,
                                             const std::string& context)
{
    CouplingMap coupling_map;
    std::ifstream summary(structure_path, std::ios::in);
    if (!summary)
    {
        auto log_location = LOGLOCATION;
        std::ostringstream msg;
        msg << "Cannot open structure file " << structure_path << std::endl;
        logger->display_message(msg.str(), LogUtils::LOGLEVEL::FATAL, log_location);
        throw InvalidStructureFile(PrefixMessage(LogUtils::LOGLEVEL::FATAL, context),
                                   msg.str(),
                                   log_location);
    }
    std::string line;

    while (std::getline(summary, line))
    {
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

/*!
 *  \brief Build maps from subproblem names to their constraint files
 *  Warning : this is heavily dependent on the coupling_map input file format
 *
 *  Iterates over the coupling_map keys (subproblem names) and derives the
 *  path to each subproblem's constraint file. Entries whose extracted number
 *  equals "master" are skipped.
 *
 *  Expected subproblem name formats (as found in coupling_map keys):
 *    - MPS_FILE:  "<prefix>_<num>.<ext>"  e.g. "problem_0.mps"
 *                 Extracts <num> between the first '_' and the first '.'.
 *                 Constraint path: "constraints/constraints_<num>.<ext>"
 *    - Other (SVF, extension not present in the coupling_map):     "<prefix>_<num>"        e.g.
 * "problem_0" Extracts <num> after the first '_'. Constraint path:
 * "constraints/constraints_<num>.svf"
 *
 *  \param coupling_map              : map from subproblem name to variable map
 *  \param subproblem_constraint_map : filled with subproblem -> constraint path
 *  \param constraints_coupling_map  : filled with constraint path -> variable map
 *  \param options                   : simulation options (determines format)
 */
void CouplingMapGenerator::BuildSubProblemConstraintMap(
  const CouplingMap& coupling_map,
  SubProblemConstraintMap& subproblem_constraint_map,
  CouplingMap& constraints_coupling_map,
  const SimulationOptions& options)
{
    for (auto&& [subProblemName, variable_map]: coupling_map)
    {
        if (options.PROBLEMS_FORMAT == ProblemsFormat::MPS_FILE)
        {
            size_t underscore_pos = subProblemName.find('_');
            size_t dot_pos = subProblemName.find('.');

            std::string subproblem_num = subProblemName.substr(underscore_pos + 1,
                                                               dot_pos - underscore_pos - 1);

            std::string extension = subProblemName.substr(dot_pos + 1);

            if (subproblem_num != "master")
            {
                std::string constraint_str = "constraints/constraints_" + subproblem_num + "."
                                             + extension;
                subproblem_constraint_map[subProblemName] = constraint_str;
                constraints_coupling_map[constraint_str] = variable_map;
            }
        }
        else
        {
            std::cout<<"building the svf case !!!! "<<std::endl ; 
            size_t underscore_pos = subProblemName.find('_');
            std::string subproblem_num = subProblemName.substr(underscore_pos + 1);

            if (subproblem_num != "master")
            {
                std::string constraint_str = "constraints/constraints_" + subproblem_num + ".svf";
                subproblem_constraint_map[subProblemName] = constraint_str;
                constraints_coupling_map[constraint_str] = variable_map;
            }
        }
    }
}
