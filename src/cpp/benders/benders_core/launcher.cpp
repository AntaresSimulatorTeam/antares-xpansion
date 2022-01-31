#include "glog/logging.h"

#include "launcher.h"
#include "SimulationOptions.h"

#include "Timer.h"

/*!
 *  \brief Get Benders Options from command line
 *
 *  \param argc : number of elements
 *
 *  \param argv : elements on command line
 */
SimulationOptions build_benders_options(int argc, char **argv)
{
	SimulationOptions result;
	result.read(argv[1]);
	return result;
}

/*!
 *  \brief Build the input from the structure file
 *
 *	Function to build the map linking each problem name to its variables and their id
 *
 *  \param root : root of the structure file
 *
 *  \param summary_name : name of the structure file
 *
 *  \param coupling_map : empty map to increment
 *
 *  \note The id in the coupling_map is that of the variable in the solver responsible for the creation of
 *  the structure file.
 */
CouplingMap build_input(const std::string &structure_path, const int slave_number, const std::string &master_name)
{
	CouplingMap coupling_map;
	std::ifstream summary(structure_path, std::ios::in);
	if (!summary)
	{
		// TODO JMK : gestion cas d'erreur si pas de structure d'entrée
		std::cout << "Cannot open file summary " << structure_path << std::endl;
		return coupling_map;
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

	if (slave_number >= 0)
	{
		int n(0);
		CouplingMap trimmer;
		for (auto const &problem : coupling_map)
		{
			if (problem.first == master_name)
				trimmer.insert(problem);
			else if (n < slave_number)
			{
				trimmer.insert(problem);
				++n;
			}
		}
		coupling_map = trimmer;
	}
	summary.close();
	return coupling_map;
}

/*!
 *  \brief How to call for the algorithm
 *
 *  \param argc : number of arguments in command line
 */
void usage(int argc)
{
	if (argc < 2)
	{
		std::cout << "usage is : <exe> <option_file> " << std::endl;
		SimulationOptions input;
		input.write_default();
		std::exit(1);
	}
}

BendersBaseOptions SimuOptToBendersBaseOpt(const SimulationOptions &options)
{
	BendersBaseOptions result;

	result.LOG_LEVEL = options.LOG_LEVEL;
	result.MAX_ITERATIONS = options.MAX_ITERATIONS;
	result.SLAVE_NUMBER = options.SLAVE_NUMBER;

	result.ABSOLUTE_GAP = options.ABSOLUTE_GAP;
	result.RELATIVE_GAP = options.RELATIVE_GAP;
	result.SLAVE_WEIGHT_VALUE = options.SLAVE_WEIGHT_VALUE;
	result.TIME_LIMIT = options.TIME_LIMIT;

	result.AGGREGATION = options.AGGREGATION;
	result.TRACE = options.TRACE;
	result.BOUND_ALPHA = options.BOUND_ALPHA;

	result.OUTPUTROOT = options.OUTPUTROOT;
	result.SLAVE_WEIGHT = options.SLAVE_WEIGHT;
	result.MASTER_NAME = options.MASTER_NAME;
	result.STRUCTURE_FILE = options.STRUCTURE_FILE;
	result.INPUTROOT = options.INPUTROOT;
	result.CSV_NAME = options.CSV_NAME;
	result.SOLVER_NAME = options.SOLVER_NAME;
	result.weights = options.weights();

	return result;
}