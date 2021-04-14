// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"

#include "ortools_utils.h"

int main(int argc, char** argv)
{
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);

	CouplingMap input; //map[probel_name][variable_name] <-> variable_id
	build_input(options, input);
	int i(0);
	std::vector<DblVector> name_rhs(input.size());
	Str2Int id_name;

	//@TODO check desynchronisation possible entre i et kvp.second : check name_rhs

	size_t n_rows(-1);
	for (auto const & kvp : input) {
		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first);
		std::cout << problem_name << std::endl;
		operations_research::MPSolver solver("analyse_mip", ORTOOLS_LP_SOLVER_TYPE);
		ORTreadmps(solver, problem_name);
		// XPRSsetcbmessage(prob, optimizermsg, NULL);
		// XPRSsetintcontrol(prob, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);

		if (kvp.first != options.MASTER_NAME) {
			StandardLp lpData(solver);
			DblVector const & rhs = std::get<Attribute::DBL_VECTOR>(lpData._data)[DblVectorAttribute::RHS];
			name_rhs[i] = rhs;
			id_name[kvp.first] = i;
			n_rows = rhs.size();
		}

		++i;
		if (i > 5)break;
	}
	std::ofstream file("toto.csv");
	for (auto const & kvp : id_name) {
		file << kvp.first << ";";
	}
	file << std::endl;
	for (size_t r(0); r < n_rows; ++r) {
		for (auto const & kvp : id_name) {
			file << name_rhs[kvp.second][r] << ";";
		}
		file << std::endl;
	}
	file.close();
	return 0;
}