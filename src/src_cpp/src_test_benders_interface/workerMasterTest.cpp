#include "catch2.hpp"
#include "WorkerMaster.h"
#include "define_datas.hpp"


TEST_CASE("WorkerMaster constructor", "[wrk-master][wrk-master-init]") {

	// =======================================================================================
	// Default constructor, worker's one
	WorkerMasterPtr worker = std::make_shared<WorkerMaster>();
	REQUIRE(worker->_is_master == true);
	REQUIRE(worker->_solver == nullptr);

	worker.reset();

	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;
	varmap["3"] = 3;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");
	DblVector neededObj;
	DblVector actualObj;
	DblVector lbs;
	DblVector ubs;
	double lb = -1e10;
	double ub = 1e20;

	IntVector mstart;
	IntVector mind;
	DblVector matval;
	int nreturned;

	auto inst = GENERATE(NET_MASTER, MIP_TOY, MULTIKP);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Idem to worker
		
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			// Testing 3 differents slave_weights 0, 1.0, and 10
			for (auto const& nslaves : IntVector({ 10, 20, 30})) {

				worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
				worker = std::make_shared<WorkerMaster>(varmap, instance_path,
					opt, nslaves);

				//1. varmap
				REQUIRE(worker->_name_to_id == datas[inst]._varmap);

				for (auto const& kvp : datas[inst]._varmap) {
					REQUIRE(kvp.first == worker->_id_to_name[kvp.second]);
				}

				REQUIRE(worker->_solver->get_ncols() == datas[inst]._ncols + 1 + nslaves);
				REQUIRE(worker->_solver->get_nrows() == datas[inst]._nrows + 1);

				// ==================================================================================
				// New objective
				actualObj.clear();
				neededObj.clear();
				neededObj.resize(nslaves + 1, 0.0);
				neededObj[0] = 1.0;
				actualObj.resize(worker->_solver->get_ncols());
				worker->_solver->get_obj(actualObj.data(), 0, worker->_solver->get_ncols() - 1);
				for (int i(0); i < worker->_solver->get_ncols(); i++) {
					if (i < datas[inst]._ncols) {
						REQUIRE(actualObj[i] == datas[inst]._obj[i]);
					}
					else {
						REQUIRE(actualObj[i] == neededObj[i - datas[inst]._ncols]);
					}
				}
				actualObj.clear();
				neededObj.clear();

				// ==================================================================================
				// Bounds on epigraph variables
				lbs.clear();
				ubs.clear();
				lbs.resize(nslaves + 1);
				ubs.resize(nslaves + 1);
				worker->_solver->get_lb(lbs.data(), datas[inst]._ncols, worker->_solver->get_ncols() - 1);
				worker->_solver->get_ub(ubs.data(), datas[inst]._ncols, worker->_solver->get_ncols() - 1);
				REQUIRE( lbs == DblVector(nslaves + 1, lb) );
				REQUIRE( ubs == DblVector(nslaves + 1, ub) );
				lbs.clear();
				ubs.clear();

				// ==================================================================================
				// New row linking epigraph variables to their obj representant
				mstart.clear();
				mind.clear();
				matval.clear();

				mstart.resize(2);
				mind.resize(nslaves + 1);
				matval.resize(nslaves + 1);
				worker->_solver->get_rows(mstart.data(), mind.data(), matval.data(), nslaves + 1, &nreturned,
					datas[inst]._nrows, datas[inst]._nrows);
				REQUIRE(nreturned == (nslaves + 1) );

				mstart.clear();
				mind.clear();
				matval.clear();

				worker.reset();
			}
		}
	}
}


TEST_CASE("WorkerMaster getters", "[wrk-master][wrk-master-get]") {

	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Idem to worker

		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			// Testing 3 differents slave_weights 0, 1.0, and 10
			for (auto const& nslaves : IntVector({ 1, 10, 100 })) {

				worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
					opt, nslaves);

				int mipstatus;
				worker->solve(mipstatus, opt);

				REQUIRE(mipstatus == OPTIMAL);
				Point x0;
				double alpha;
				DblVector alpha_i;
				worker->get(x0, alpha, alpha_i);
				for (auto const& kvp : x0) {
					REQUIRE(kvp.second == 0.0);
				}
				REQUIRE(alpha == -1e10);
				REQUIRE(alpha_i.size() == nslaves);

				worker.reset();
			}
		}
	}
}

// ==================================================================================
// Coup 1: add cuts
TEST_CASE("WorkerMaster add cuts", "[wrk-master][wrk-master-addcuts]") {
	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	Point x0;
	Point s;
	double rhs;

	IntVector mstart;
	IntVector mind;
	DblVector matval;
	int nreturned;

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
				opt, 5);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 1);

			// ==================================================================================
			// coupe 1 : add_cut
			x0.clear();
			s.clear();

			for (auto const& kvp : worker->_name_to_id) {
				x0[kvp.first] = 2.0;
				s[kvp.first] = -5.0;
			}
			rhs = 10.0;
			worker->add_cut(s, x0, rhs);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 2);

			mstart.clear();
			mind.clear();
			matval.clear();

			mstart.resize(2);
			int nelsCut = worker->_name_to_id.size() + 1;
			mind.resize(nelsCut);
			matval.resize(nelsCut);

			worker->_solver->get_rows(mstart.data(), mind.data(), matval.data(), nelsCut, &nreturned,
				worker->get_number_constraint() - 1, worker->get_number_constraint() - 1);

			REQUIRE(nreturned == nelsCut);
			for (int k = 0; k < nelsCut - 1; k++) {
				REQUIRE(matval[k] == -5.0);
			}
			REQUIRE(matval[nelsCut - 1] == -1.0);
			REQUIRE(mind[nelsCut - 1] == datas[inst]._ncols);

			mstart.clear();
			mind.clear();
			matval.clear();
			
			worker.reset();
			
		}
	}
}

// ==================================================================================
// Coup 2: add_dynamic cuts
TEST_CASE("WorkerMaster add dynamic cuts", "[wrk-master][wrk-master-addcuts]") {
	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	Point x0;
	Point s;
	double rhs;

	IntVector mstart;
	IntVector mind;
	DblVector matval;
	int nreturned;

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
				opt, 5);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 1);

			// ==================================================================================
			// coupe 2 : add_dynamic
			x0.clear();
			s.clear();

			for (auto const& kvp : worker->_name_to_id) {
				s[kvp.first] = -5.0;
			}
			rhs = 10.0;
			worker->add_dynamic_cut(s, 6.0, rhs);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 2);

			mstart.clear();
			mind.clear();
			matval.clear();

			mstart.resize(2);
			int nelsCut = worker->_name_to_id.size() + 1;
			mind.resize(nelsCut);
			matval.resize(nelsCut);

			worker->_solver->get_rows(mstart.data(), mind.data(), matval.data(), nelsCut, &nreturned,
				worker->get_number_constraint() - 1, worker->get_number_constraint() - 1);

			REQUIRE(nreturned == nelsCut);
			for (int k = 0; k < nelsCut - 1; k++) {
				REQUIRE(matval[k] == -5.0);
			}
			REQUIRE(matval[nelsCut - 1] == -1.0);
			REQUIRE(mind[nelsCut - 1] == datas[inst]._ncols);

			mstart.clear();
			mind.clear();
			matval.clear();

			worker.reset();

		}
	}
}


// ==================================================================================
// Coupe 3: add cut by iter
TEST_CASE("WorkerMaster add cut by iter ", "[wrk-master][wrk-master-addcuts]") {
	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	Point x0;
	Point s;
	double rhs;

	IntVector mstart;
	IntVector mind;
	DblVector matval;
	int nreturned;

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
				opt, 5);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 1);

			// ==================================================================================
			// Coupe 3: add cut by iter
			x0.clear();
			s.clear();

			for (auto const& kvp : worker->_name_to_id) {
				s[kvp.first] = -5.0;
			}
			rhs = 10.0;
			worker->add_cut_by_iter(0, s, 6.0, rhs);
			worker->add_cut_by_iter(3, s, 6.0, rhs);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 3);

			mstart.clear();
			mind.clear();
			matval.clear();

			mstart.resize(3);
			int nelsCut = worker->_name_to_id.size() + 1;
			mind.resize(2*nelsCut);
			matval.resize(2*nelsCut);

			worker->_solver->get_rows(mstart.data(), mind.data(), matval.data(), 2*nelsCut, &nreturned,
				worker->get_number_constraint() - 2, worker->get_number_constraint() - 1);

			REQUIRE(nreturned == 2*nelsCut);

			// check cut 1: id alpha = 0
			for (int k = 0; k < nelsCut - 1; k++) {
				REQUIRE(matval[k] == -5.0);
			}
			REQUIRE(matval[nelsCut - 1] == -1.0);
			REQUIRE(mind[nelsCut - 1] == datas[inst]._ncols + 1);

			// check cut 2: id alpha = 3
			for (int k = nelsCut; k < 2*nelsCut - 1; k++) {
				REQUIRE(matval[k] == -5.0);
			}
			REQUIRE(matval[2*nelsCut - 1] == -1.0);
			REQUIRE(mind[2*nelsCut - 1] == datas[inst]._ncols + 4);

			mstart.clear();
			mind.clear();
			matval.clear();

			worker.reset();

		}
	}
}

// ==================================================================================
// Coup 4: add cut slave
TEST_CASE("WorkerMaster add cut slave", "[wrk-master][wrk-master-addcuts]") {
	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	Point x0;
	Point s;
	double rhs;

	IntVector mstart;
	IntVector mind;
	DblVector matval;
	int nreturned;

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
				opt, 5);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 1);

			// ==================================================================================
			// Coupe 3: add cut by iter
			x0.clear();
			s.clear();

			for (auto const& kvp : worker->_name_to_id) {
				x0[kvp.first] = 2.0;
				s[kvp.first] = -5.0;
			}
			rhs = 10.0;
			worker->add_cut_slave(0, s, x0, rhs);
			worker->add_cut_slave(3, s, x0, rhs);

			REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 3);

			mstart.clear();
			mind.clear();
			matval.clear();

			mstart.resize(3);
			int nelsCut = worker->_name_to_id.size() + 1;
			mind.resize(2 * nelsCut);
			matval.resize(2 * nelsCut);

			worker->_solver->get_rows(mstart.data(), mind.data(), matval.data(), 2 * nelsCut, &nreturned,
				worker->get_number_constraint() - 2, worker->get_number_constraint() - 1);

			REQUIRE(nreturned == 2 * nelsCut);

			// check cut 1: id alpha = 0
			for (int k = 0; k < nelsCut - 1; k++) {
				REQUIRE(matval[k] == -5.0);
			}
			REQUIRE(matval[nelsCut - 1] == -1.0);
			REQUIRE(mind[nelsCut - 1] == datas[inst]._ncols + 1);

			// check cut 2: id alpha = 3
			for (int k = nelsCut; k < 2 * nelsCut - 1; k++) {
				REQUIRE(matval[k] == -5.0);
			}
			REQUIRE(matval[2 * nelsCut - 1] == -1.0);
			REQUIRE(mind[2 * nelsCut - 1] == datas[inst]._ncols + 4);

			mstart.clear();
			mind.clear();
			matval.clear();

			worker.reset();

		}
	}
}

TEST_CASE("WorkerMaster del cuts", "[wrk-master][wrk-master-delcuts]") {
	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			for (int i(1) ; i < 3; i++) {

				worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
					opt);

				REQUIRE(worker->get_number_constraint() == datas[inst]._nrows + 1);

				worker->delete_constraint(i);

				REQUIRE(worker->get_number_constraint() == datas[inst]._nrows - i + 1);

				worker.reset();
			}
		}
	}
}

TEST_CASE("WorkerMaster bound alpha", "[wrk-master][wrk-master-alpha]") {
	WorkerMasterPtr worker;

	SolverFactory factory;
	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");

	auto inst = GENERATE(NET_MASTER);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;
		for (auto const& solver_name : factory.get_solvers_list()) {
			opt.SOLVER_NAME = solver_name;

			worker = std::make_shared<WorkerMaster>(datas[inst]._varmap, instance_path,
				opt);

			double UB = 150.0;
			worker->fix_alpha(UB);

			DblVector ubs(1);
			worker->_solver->get_ub(ubs.data(), worker->_id_alpha, worker->_id_alpha);

			REQUIRE(ubs[0] == UB);
		}
	}
}