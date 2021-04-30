#include "catch2.hpp"
#include "WorkerSlave.h"
#include "define_datas.hpp"

TEST_CASE("Worker Slave instanciation", "[wrk-slave][wrk-slave-init]") {

	// =======================================================================================
	// Default constructor, worker's one
	WorkerSlavePtr worker = std::make_shared<WorkerSlave>();
	REQUIRE(worker->_is_master == false);
	REQUIRE(worker->_solver == nullptr);

	worker.reset();

	// =======================================================================================
	// Full constructor
	Str2Int varmap;
	varmap["1"] = 1;
	varmap["2"] = 2;
	varmap["3"] = 3;

	BendersOptions opt;

	AllDatas datas;
	fill_datas(datas);

	std::string instance_path("");
	DblVector neededObj;
	DblVector actualObj;

	auto inst = GENERATE(MIP_TOY, MULTIKP);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Idem to worker
		SolverFactory factory;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			// Testing 3 differents slave_weights 0, 1.0, and 10
			for (auto const& slave_wieght : DblVector({ 0.0, 1.0, 10.0 })) {

				worker = std::make_unique<WorkerSlave>(varmap, instance_path,
					slave_wieght, opt);

				//1. varmap
				REQUIRE(worker->_name_to_id == varmap);

				for (auto const& kvp : varmap) {
					REQUIRE(kvp.first == worker->_id_to_name[kvp.second]);
				}

				//2. obj
				neededObj.clear();
				neededObj.resize(datas[inst]._ncols);
				for (int i(0); i < datas[inst]._obj.size(); i++) {
					neededObj[i] = datas[inst]._obj[i] * slave_wieght;
				}

				actualObj.clear();
				actualObj.resize(worker->_solver->get_ncols());
				worker->_solver->get_obj(actualObj.data(), 0, worker->_solver->get_ncols() - 1 );
				REQUIRE(actualObj == neededObj);

				worker.reset();
			}

		}
	}
}

TEST_CASE("Worker Slave methods : fix_to", "[wrk-slave]") {

	WorkerSlavePtr worker = std::make_shared<WorkerSlave>();

	AllDatas datas;
	fill_datas(datas);
	BendersOptions opt;
	std::string instance_path("");
	Str2Int varmap;

	// Loop over the instances
	auto inst = GENERATE(NET_SP1, NET_SP2);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Loop over the solvers
		SolverFactory factory;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			varmap = datas[inst]._varmap;
			worker = std::make_unique<WorkerSlave>(varmap, instance_path,
				1.0, opt);
				
			// method fixto
			for (int val(0); val <= 4; val += 2) {
				Point x0;
				for (auto const& kvp : varmap) {
					x0[kvp.first] = double(val);
				}
				worker->fix_to(x0);

				int nmastervars = varmap.size();
				DblVector lbs(nmastervars);
				DblVector ubs(nmastervars);
				worker->_solver->get_lb(lbs.data(), 0, nmastervars - 1);
				worker->_solver->get_ub(ubs.data(), 0, nmastervars - 1);
				for (auto const& lb : lbs) {
					REQUIRE(lb == double(val) );
					
				}
				for (auto const& ub : ubs) {
					REQUIRE(ub == double(val));
				}
			}

			worker->free();
		}
	}
}

TEST_CASE("Worker Slave methods : get_subgradient", "[wrk-slave]") {

	WorkerSlavePtr worker = std::make_shared<WorkerSlave>();

	AllDatas datas;
	fill_datas(datas);
	BendersOptions opt;
	std::string instance_path("");
	Str2Int varmap;

	// Loop over the instances
	auto inst = GENERATE(NET_SP1, NET_SP2);
	SECTION("Loop over the instances") {

		instance_path = datas[inst]._path;

		// Loop over the solvers
		SolverFactory factory;
		for (auto const& solver_name : factory.get_solvers_list()) {

			opt.SOLVER_NAME = solver_name;

			varmap = datas[inst]._varmap;
			worker = std::make_unique<WorkerSlave>(varmap, instance_path,
				1.0, opt);

			// method fixto
			Point x0;
			for (auto const& kvp : varmap) {
				x0[kvp.first] = 0.0;
			}
			worker->fix_to(x0);

			// Solving, and getting subgradients
			int status;
			worker->solve(status, opt);
			REQUIRE(status == OPTIMAL);

			Point s;
			worker->get_subgradient(s);
			for (auto const& kvp : s) {
				REQUIRE(s[kvp.first] <= datas[inst]._subgrad[kvp.first] + 1e-3);
				REQUIRE(s[kvp.first] >= datas[inst]._subgrad[kvp.first] - 1e-3);
			}

			std::pair<IntVector, IntVector> basis = worker->get_basis();

			worker->free();
		}
	}
}