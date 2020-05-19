// projet_benders.cpp : définit le point d'entrée pour l'application console.
//
#include "launcher.h"
#include "Worker.h"
#include "BendersOptions.h"
#include "BendersFunctions.h"


int main(int argc, char** argv)
{
	usage(argc);
	BendersOptions options(build_benders_options(argc, argv));
	options.print(std::cout);

	XPRSinit("");
	CouplingMap input;
	build_input(options, input);
	XPRSprob full;
	XPRScreateprob(&full);
	XPRSsetcbmessage(full, optimizermsg, NULL);
	XPRSsetintcontrol(full, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSloadlp(full, "full", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	Str2Int _decalage;
	int ncols(0);
	int nslaves(input.size());
	CouplingMap x_mps_id;
	for (auto const & kvp : input) {
		std::string problem_name(options.INPUTROOT + PATH_SEPARATOR + kvp.first);
		XPRSgetintattrib(full, XPRS_COLS, &ncols);
		_decalage[kvp.first] = ncols;

		XPRSprob prob;
		XPRScreateprob(&prob);
		XPRSsetcbmessage(prob, optimizermsg, NULL);
		XPRSsetintcontrol(prob, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
		XPRSreadprob(prob, problem_name.c_str(), "");
		if (kvp.first != options.MASTER_NAME) {

			int mps_ncols(0);
			XPRSgetintattrib(prob, XPRS_COLS, &mps_ncols);
			DblVector o(mps_ncols, 0);
			IntVector sequence(mps_ncols);
			for (int i(0); i < mps_ncols; ++i) {
				sequence[i] = i;
			}
			XPRSgetobj(prob, o.data(), 0, mps_ncols - 1);
			double const weigth = options.slave_weight(nslaves, problem_name);
			for (auto & c : o) {
				c *= weigth;
			}
			XPRSchgobj(prob, mps_ncols, sequence.data(), o.data());
		}
		StandardLp lpData(prob);
		lpData.append_in(full);

		XPRSdestroyprob(prob);
		for (auto const & x : kvp.second) {
			x_mps_id[x.first][kvp.first] = x.second;
		}
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
		std::string const name(kvp.first);
		std::cout << name << std::endl;
		bool is_first(true);
		int id1(-1);
		std::string first_mps;
		for (auto const & mps : kvp.second) {
			if (is_first) {
				is_first = false;
				first_mps = mps.first;
				id1 = mps.second + _decalage.find(first_mps)->second;
			}
			else {
				int id2 = mps.second + _decalage.find(mps.first)->second;
				//std::cout << id1 << " - " << id2 << std::endl;
				// x[id1] - x[id2] = 0
				mstart.push_back(neles);
				cindex.push_back(id1);
				values.push_back(1);
				neles += 1;

				cindex.push_back(id2);
				values.push_back(-1);
				neles += 1;
				nrows += 1;
			}
		}
	}
	DblVector rhs(nrows, 0);
	CharVector sense(nrows, 'E');
	XPRSaddrows(full, nrows, neles, sense.data(), rhs.data(), NULL, mstart.data(), cindex.data(), values.data());

	//std::cout << "Writting mps file" << std::endl;
	//XPRSwriteprob(full, "full.mps", "");
	//std::cout << "Writting lp file" << std::endl; 
	//XPRSwriteprob(full, "full.lp", "l");
	std::cout << "Solving" << std::endl;
	XPRSsetintcontrol(full, XPRS_BARTHREADS, 16);
	XPRSsetintcontrol(full, XPRS_BARCORES, 16);
	XPRSlpoptimize(full, "-b");

	Point x0;
	XPRSgetintattrib(full, XPRS_COLS, &ncols);
	DblVector ptr(ncols, 0);
	XPRSgetlpsol(full, ptr.data(), NULL, NULL, NULL);
	for (auto const & kvp : input[options.MASTER_NAME]) {
		x0[kvp.first] = ptr[kvp.second];
	}
	print_solution(std::cout, x0, true);

	XPRSdestroyprob(full);
	XPRSfree();

	return 0;
}