#pragma once
#include "catch2.hpp"
#include <iostream>
#include "Solver.h"
#include "define_datas.hpp"

TEST_CASE("Un objet solveur peut etre cree et detruit", "[read][init]") {
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP);
    SECTION("Construction and destruction") {

        for (auto const& solver_name : factory.get_solvers_list()) {
            std::string instance = datas[inst]._path;

            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. destruction de l'objet pointe
            solver.reset();
            REQUIRE(solver == nullptr);
        }
    }
}

TEST_CASE("MPS file can be read and we can get number of columns", "[read][read-cols]") {
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());
            
            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
        }
    }
}

TEST_CASE("MPS file can be read and we can get number of rows", "[read][read-rows]") {
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_nrows() == datas[inst]._nrows);
        }
    }
}

TEST_CASE("MPS file can be read and we can get number of integer variables", "[read][read-integer-vars]") {
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_n_integer_vars() == datas[inst]._nintegervars);
        }
    }
}

TEST_CASE("MPS file can be read and we can get number of non zero elements in the matrix", "[read][read-elements]") {
    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_nelems() == datas[inst]._nelems);
        }
    }
}


TEST_CASE("MPS file can be read and we can get objective function coefficients", "[read][read-obj]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 4. Recuperation de la fonction objectif
            int n_vars = solver->get_ncols();
            REQUIRE(n_vars == datas[inst]._ncols);
            std::vector<double> obj(n_vars);

            solver->get_obj(obj.data(), 0, n_vars - 1);

            REQUIRE(obj == datas[inst]._obj);
        }
    }
}

TEST_CASE("MPS file can be read and we can get matrix coefficients", "[read][read-rows]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            REQUIRE(solver->get_nrows() == datas[inst]._nrows);
            REQUIRE(solver->get_nelems() == datas[inst]._nelems);

            //========================================================================================
            // 5. Recuperation des contraintes
            int n_elems = solver->get_nelems();
            int n_cstr = solver->get_nrows();

            std::vector<double> matval(n_elems);
            std::vector<int>	mstart(n_cstr + 1);
            std::vector<int>	mind(n_elems);

            int n_returned(0);
            solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);

            REQUIRE(n_returned == datas[inst]._nelems);
            REQUIRE(matval == datas[inst]._matval);
            REQUIRE(mind == datas[inst]._mind);
            REQUIRE(mstart == datas[inst]._mstart);
        }
    }
}

TEST_CASE("MPS file can be read and we can get right hand side", "[read][read-rhs]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());          

            //========================================================================================
            // 6. Recuperation des seconds membres
            int n_cstr = solver->get_nrows();
            REQUIRE(solver->get_nrows() == datas[inst]._nrows);

            std::vector<double> rhs(n_cstr);
            if (n_cstr > 0) {
                solver->get_rhs(rhs.data(), 0, n_cstr - 1);
            }

            REQUIRE(rhs == datas[inst]._rhs);
        }
    }
}


TEST_CASE("MPS file can be read and we can get row types", "[read][read-rowtypes]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());


            REQUIRE(solver->get_nrows() == datas[inst]._nrows);
            //========================================================================================
            // 7. Recuperation des types de contraintes
            int n_cstr = solver->get_nrows();
            std::vector<char> rtypes(n_cstr);
            if (n_cstr > 0) {
                solver->get_row_type(rtypes.data(), 0, n_cstr - 1);
            }
            REQUIRE(rtypes == datas[inst]._rowtypes);
        }
    }
}


TEST_CASE("MPS file can be read and we can get types of columns", "[read][read-coltypes]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 8. Recuperation des types de variables
            int n_vars = solver->get_ncols();
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            std::vector<char> coltype(n_vars);
            solver->get_col_type(coltype.data(), 0, n_vars - 1);

            REQUIRE(coltype == datas[inst]._coltypes);
        }
    }
}

TEST_CASE("MPS file can be read and we can get lower bounds on variables", "[read][read-lb]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 9. Recuperation des bornes inf sur les variables
            int n_vars = solver->get_ncols();
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            std::vector<double> lb(n_vars);
            solver->get_lb(lb.data(), 0, n_vars - 1);

            REQUIRE(lb == datas[inst]._lb);
        }
    }
}

TEST_CASE("MPS file can be read and we can get upper bounds on variables", "[read][read-ub]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 10. Recuperation des bornes sup sur les variables
            int n_vars = solver->get_ncols();
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            std::vector<double> ub(n_vars);
            solver->get_ub(ub.data(), 0, n_vars - 1);

            // gestion a la main des infinis differents selon les solveurs
            // tous les infinis mis a 1e20 pour les tests
            for (int i(0); i < solver->get_ncols(); i++) {
                if (datas[inst]._ub[i] == 1e20 && ub[i] > 1e20) {
                    ub[i] = 1e20;
                }
            }
            REQUIRE(ub == datas[inst]._ub);
        }
    }
}


TEST_CASE("MPS file can be read and we can get every information about the problem", "[read]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;

    auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2, SLACKS, REDUCED);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur

            
            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            //========================================================================================
            // 3. Recuperation des donnees de base du probleme
            REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            REQUIRE(solver->get_nrows() == datas[inst]._nrows);
            REQUIRE(solver->get_n_integer_vars() == datas[inst]._nintegervars);
            REQUIRE(solver->get_nelems() == datas[inst]._nelems);

            //========================================================================================
            // 4. Recuperation de la fonction objectif
            int n_vars = solver->get_ncols();
            std::vector<double> obj(n_vars);

            solver->get_obj(obj.data(), 0, n_vars - 1);

            REQUIRE(obj == datas[inst]._obj);


            //========================================================================================
            // 5. Recuperation des contraintes
            int n_elems = solver->get_nelems();
            int n_cstr = solver->get_nrows();

            std::vector<double> matval(n_elems);
            std::vector<int>	mstart(n_cstr + 1);
            std::vector<int>	mind(n_elems);

            int n_returned(0);
            solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems, &n_returned, 0, n_cstr - 1);

            REQUIRE(n_returned == datas[inst]._nelems);
            REQUIRE(matval == datas[inst]._matval);
            REQUIRE(mind == datas[inst]._mind);
            REQUIRE(mstart == datas[inst]._mstart);

            //========================================================================================
            // 6. Recuperation des seconds membres
            n_cstr = solver->get_nrows();
            std::vector<double> rhs(n_cstr);
            if (n_cstr > 0) {
                solver->get_rhs(rhs.data(), 0, n_cstr - 1);
            }

            REQUIRE(rhs == datas[inst]._rhs);

            //========================================================================================
            // 7. Recuperation des types de contraintes
            n_cstr = solver->get_nrows();
            std::vector<char> rtypes(n_cstr);
            if (n_cstr > 0) {
                solver->get_row_type(rtypes.data(), 0, n_cstr - 1);
            }
            REQUIRE(rtypes == datas[inst]._rowtypes);


            //========================================================================================
            // 8. Recuperation des types de variables
            n_vars = solver->get_ncols();
            std::vector<char> coltype(n_vars);
            solver->get_col_type(coltype.data(), 0, n_vars - 1);

            REQUIRE(coltype == datas[inst]._coltypes);

            //========================================================================================
            // 9. Recuperation des bornes inf sur les variables
            n_vars = solver->get_ncols();
            std::vector<double> lb(n_vars);
            solver->get_lb(lb.data(), 0, n_vars - 1);

            REQUIRE(lb == datas[inst]._lb);

            //========================================================================================
            // 10. Recuperation des bornes sup sur les variables
            n_vars = solver->get_ncols();
            std::vector<double> ub(n_vars);
            solver->get_ub(ub.data(), 0, n_vars - 1);

            // gestion a la main des infinis differents selon les solveurs
            // tous les infinis mis a 1e20 pour les tests
            for (int i(0); i < solver->get_ncols(); i++) {
                if (datas[inst]._ub[i] == 1e20 && ub[i] > 1e20) {
                    ub[i] = 1e20;
                }
            }
            REQUIRE(ub == datas[inst]._ub);
        }
    }
}

TEST_CASE("We can get the names of variables and constraints present in MPS file after read", "[read][read-names]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;
    int ind = 0;
    //auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER, NET_SP1, NET_SP2);
    auto inst = GENERATE(MIP_TOY, MULTIKP);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());

            
            if (solver_name == "XPRESS") {
                std::string prb_name = "test" + ind;
                solver->write_prob(prb_name.c_str(), "MPS");
                ind++;
            }
            //========================================================================================
            // 10. Recuperation des noms
            int n_vars = solver->get_ncols();
            int n_rows = solver->get_nrows();
            //REQUIRE(solver->get_ncols() == datas[inst]._ncols);
            //REQUIRE(solver->get_nrows() == datas[inst]._nrows);

            std::vector<std::string> rowNames(n_rows);
            std::vector<std::string> colNames(n_vars);

            solver->get_row_names(0, n_rows - 1, rowNames);
            solver->get_col_names(0, n_vars - 1, colNames);

            int ind = 0;
            std::string cur_name;
            for (auto const& name : rowNames) {
                cur_name = datas[inst]._row_names[ind];
                REQUIRE(name.compare(0, cur_name.size(), cur_name) == 0);
                ind++;
            }
            ind = 0;
            for (auto const& name : colNames) {
                cur_name = datas[inst]._col_names[ind];
                REQUIRE(name.compare(0, cur_name.size(), cur_name) == 0);
                ind++;
            }
        }
    }
}


TEST_CASE("We can get the indices of rows and columns by their names", "[read][get-indices]") {

    AllDatas datas;
    fill_datas(datas);

    SolverFactory factory;
    int ind = 0;
    auto inst = GENERATE(MIP_TOY, MULTIKP);
    SECTION("Reading instance") {

        for (auto const& solver_name : factory.get_solvers_list()) {

            std::string instance = datas[inst]._path;
            //========================================================================================
            // 1. declaration d'un objet solveur


            SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            REQUIRE(solver->get_number_of_instances() == 1);

            //========================================================================================
            // 2. initialisation d'un probleme et lecture
            solver->init();

            const std::string flags = "MPS";
            solver->read_prob(instance.c_str(), flags.c_str());


            if (solver_name == "XPRESS") {
                std::string prb_name = "test" + ind;
                solver->write_prob(prb_name.c_str(), "MPS");
                ind++;
            }
            //========================================================================================
            // Indices
            int n_vars = solver->get_ncols();
            int n_rows = solver->get_nrows();
            
            int col_index = -1;
            std::string cur_name;
            for (int i(0); i < 2; i++) {
                cur_name = datas[inst]._col_names[i];
                col_index = solver->get_col_index(cur_name);
                REQUIRE(col_index == i);
            }

            int row_index = -1;
            for (int i(0); i < 2; i++) {
                cur_name = datas[inst]._row_names[i];
                row_index = solver->get_row_index(cur_name);
                REQUIRE(row_index == i);
            }
        }
    }
}