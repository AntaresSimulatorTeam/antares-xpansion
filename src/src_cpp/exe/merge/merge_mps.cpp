// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"
#include "JsonWriter.h"
#include "Timer.h"

#include "ortools_utils.h"


//@suggest: create and move to standardlp.cpp
// Initialize static member
size_t StandardLp::appendCNT = 0;

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	google::SetLogDestination(google::GLOG_INFO, "./merge_mpsLog");
	LOG(INFO) << "starting merge_mps" << std::endl;

	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);

	JsonWriter jsonWriter_l;
	jsonWriter_l.write(options);
	jsonWriter_l.updateBeginTime();

	CouplingMap input;
	build_input(options, input);

	operations_research::MPSolver mergedSolver_l("full_mip", ORTOOLS_MIP_SOLVER_TYPE);
	// XPRSsetcbmessage(full, optimizermsg, NULL);
	// XPRSsetintcontrol(full, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	int ncols(0);
	int nslaves(input.size());//size-1 no ? it contains the master
	CouplingMap x_mps_id;
	int cntProblems_l(0);

	LOG(INFO) << "Merging problems..." << std::endl;
	for (auto const & kvp : input) {

		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first + ".mps");
		ncols = mergedSolver_l.NumVariables();

		operations_research::MPSolver::OptimizationProblemType solverType_l;
		if(kvp.first == options.MASTER_NAME)
		{
			solverType_l = ORTOOLS_MIP_SOLVER_TYPE;
		}
		else
		{
			solverType_l = ORTOOLS_LP_SOLVER_TYPE;
		}
		operations_research::MPSolver solver_l("toMerge", solverType_l);

		ORTreadmps(solver_l, problem_name);
		// XPRSsetcbmessage(prob, optimizermsg, NULL);
		// XPRSsetintcontrol(prob, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);

		if (kvp.first != options.MASTER_NAME) {

			int mps_ncols(solver_l.NumVariables());

			DblVector o;
			IntVector sequence(mps_ncols);
			for (int i(0); i < mps_ncols; ++i) {
				sequence[i] = i;
			}
			ORTgetobj(solver_l, o, 0, mps_ncols - 1);
			double const weigth = options.slave_weight(nslaves, problem_name);
			for (auto & c : o) {
				c *= weigth;
			}
			ORTchgobj(solver_l, sequence, o);
		}
		StandardLp lpData(solver_l);
		std::string varPrefix_l = "prob" + std::to_string(cntProblems_l) + "_";
		lpData.append_in(mergedSolver_l, varPrefix_l);

		for (auto const & x : kvp.second) {
			operations_research::MPVariable const * const var_l = mergedSolver_l.LookupVariableOrNull(varPrefix_l+x.first);
			if (nullptr == var_l)
			{
				std::cerr << "missing variable " << x.first << " in " << kvp.first << " supposedly renamed to " << varPrefix_l+x.first << ".";
				ORTwritelp(mergedSolver_l, "mergeError.lp");
				std::exit(1);
			}
			else
			{
				x_mps_id[x.first][kvp.first] = var_l->index();//x_mps_id[var_name_in_structure][problem_name] = var_id_in_merged_solver
			}
		}

		++cntProblems_l;
	}

	IntVector mstart;
	IntVector cindex;
	DblVector values;
	int nrows(0);
	int neles(0);
	size_t neles_reserve(0);
	size_t nrows_reserve(0);
	for (auto const & kvp : x_mps_id) {
		neles_reserve += kvp.second.size()*(kvp.second.size() - 1);
		nrows_reserve += kvp.second.size()*(kvp.second.size() - 1) / 2;
	}
	std::cout << "About to add " << nrows_reserve << " coupling constraints" << std::endl;
	values.reserve(neles_reserve);
	cindex.reserve(neles_reserve);
	mstart.reserve(nrows_reserve + 1);
	// adding coupling constraints
	for (auto const & kvp : x_mps_id) {
		std::string const var_name(kvp.first);
		std::cout << var_name << std::endl;
		bool is_first(true);
		int id1(-1);
		std::string first_mps;
		for (auto const & mps : kvp.second) {
			if (is_first) {
				is_first = false;
				first_mps = mps.first;
				id1 = mps.second;
			}
			else {
				int id2 = mps.second;
				//std::cout << id1 << " - " << id2 << std::endl;
				// x[id1] - x[id2] = 0
				mstart.push_back(neles);
				cindex.push_back(id1);
				values.push_back(1);
				++neles;

				cindex.push_back(id2);
				values.push_back(-1);
				++neles;
				++nrows;
			}
		}
	}
	DblVector rhs(nrows, 0);
	CharVector sense(nrows, 'E');
	ORTaddrows(mergedSolver_l, sense, rhs, {}, mstart, cindex, values);

	LOG(INFO) << "Problems merged." << std::endl;
	std::cout << "Writting mps file" << std::endl;
	ORTwritelp(mergedSolver_l, "log_merged.mps");
	std::cout << "Writting lp file" << std::endl;
	ORTwritelp(mergedSolver_l, "log_merged.lp");

	// XPRSsetintcontrol(full, XPRS_BARTHREADS, 16);
	// XPRSsetintcontrol(full, XPRS_BARCORES, 16);
	// XPRSlpoptimize(full, "-b");
	mergedSolver_l.SetNumThreads(16);

	std::cout << "Solving" << std::endl;
	LOG(INFO) << "Solving..." << std::endl;
	Timer timer;
	int status_l = mergedSolver_l.Solve();
	std::cout << "Problem solved in " << timer.elapsed() << " seconds" << std::endl;
	LOG(INFO) << "Problem solved in " << timer.elapsed() << " seconds" << std::endl;

	jsonWriter_l.updateEndTime();

	Point x0;
	DblVector ptr;
	double investCost_l(0);
	ORTgetlpsolution(mergedSolver_l, ptr);
	for (auto const & pairNameId : input[options.MASTER_NAME]) {
		int varIndexInMerged_l = x_mps_id[pairNameId.first][options.MASTER_NAME];
		x0[pairNameId.first] = ptr[varIndexInMerged_l];
		double costCoeff_l = mergedSolver_l.Objective().GetCoefficient(mergedSolver_l.variables()[varIndexInMerged_l]);
		investCost_l += x0[pairNameId.first] * costCoeff_l;
	}
	std::ostringstream oss_l;
	print_solution(oss_l, x0, true);
	std::cout << oss_l.str();
	LOG(INFO) << oss_l.str() << std::endl;

	bool optimality_l = (status_l == operations_research::MPSolver::OPTIMAL);
	jsonWriter_l.write(input.size(), mergedSolver_l.Objective().BestBound(), mergedSolver_l.Objective().Value(), investCost_l, x0, optimality_l);
	jsonWriter_l.dump("out.json");

	return 0;
}
