#include <iostream>

#include "catch2.hpp"
#include "define_datas.hpp"
#include "multisolver_interface/Solver.h"

TEST_CASE("Un objet solveur peut etre cree et detruit", "[read][init]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP);
  SECTION("Construction and destruction") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;

      //========================================================================================
      // solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);

      //========================================================================================
      // solver destruction
      solver.reset();
      REQUIRE(solver == nullptr);
    }
  }
}

TEST_CASE("MPS file can be read and we can get number of columns",
          "[read][read-cols]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    namespace fs = std::filesystem;
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      std::cout << "Current dir " << fs::current_path() << std::endl;
      std::cout << instance << std::endl;
      //========================================================================================
      // Solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);

      //========================================================================================
      // initalization and read problem
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get number of columns
      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
    }
  }
}

TEST_CASE("MPS file can be read and we can get number of rows",
          "[read][read-rows]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get numer of rows
      REQUIRE(solver->get_nrows() == datas[inst]._nrows);
    }
  }
}

TEST_CASE("MPS file can be read and we can get number of integer variables",
          "[read][read-integer-vars]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get number of integer variables
      REQUIRE(solver->get_n_integer_vars() == datas[inst]._nintegervars);
    }
  }
}

TEST_CASE(
    "MPS file can be read and we can get number of non zero elements in the "
    "matrix",
    "[read][read-elements]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get number of non zero elements in matrix
      REQUIRE(solver->get_nelems() == datas[inst]._nelems);
    }
  }
}

TEST_CASE("MPS file can be read and we can get objective function coefficients",
          "[read][read-obj]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get objective function
      int n_vars = solver->get_ncols();
      REQUIRE(n_vars == datas[inst]._ncols);
      std::vector<double> obj(n_vars);

      solver->get_obj(obj.data(), 0, n_vars - 1);

      REQUIRE(obj == datas[inst]._obj);
    }
  }
}

TEST_CASE("MPS file can be read and we can get matrix coefficients",
          "[read][read-rows]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get necessary datas from solver
      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
      REQUIRE(solver->get_nrows() == datas[inst]._nrows);
      REQUIRE(solver->get_nelems() == datas[inst]._nelems);

      //========================================================================================
      // Get rows
      int n_elems = solver->get_nelems();
      int n_cstr = solver->get_nrows();

      std::vector<double> matval(n_elems);
      std::vector<int> mstart(n_cstr + 1);
      std::vector<int> mind(n_elems);

      int n_returned(0);
      solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                       &n_returned, 0, n_cstr - 1);

      REQUIRE(n_returned == datas[inst]._nelems);
      REQUIRE(matval == datas[inst]._matval);
      REQUIRE(mind == datas[inst]._mind);
      REQUIRE(mstart == datas[inst]._mstart);
    }
  }
}

TEST_CASE("MPS file can be read and we can get right hand side",
          "[read][read-rhs]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get Constraints Right Hand Sides
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

TEST_CASE("MPS file can be read and we can get row types",
          "[read][read-rowtypes]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver Declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      REQUIRE(solver->get_nrows() == datas[inst]._nrows);
      //========================================================================================
      // Get rowtypes
      int n_cstr = solver->get_nrows();
      std::vector<char> rtypes(n_cstr);
      if (n_cstr > 0) {
        solver->get_row_type(rtypes.data(), 0, n_cstr - 1);
      }
      REQUIRE(rtypes == datas[inst]._rowtypes);
    }
  }
}

TEST_CASE("MPS file can be read and we can get types of columns",
          "[read][read-coltypes]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver Declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get column types
      int n_vars = solver->get_ncols();
      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
      std::vector<char> coltype(n_vars);
      solver->get_col_type(coltype.data(), 0, n_vars - 1);

      REQUIRE(coltype == datas[inst]._coltypes);
    }
  }
}

TEST_CASE("MPS file can be read and we can get lower bounds on variables",
          "[read][read-lb]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get lower bounds on variables
      int n_vars = solver->get_ncols();
      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
      std::vector<double> lb(n_vars);
      solver->get_lb(lb.data(), 0, n_vars - 1);

      REQUIRE(lb == datas[inst]._lb);
    }
  }
}

TEST_CASE("MPS file can be read and we can get upper bounds on variables",
          "[read][read-ub]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Get upper bounds on variables
      int n_vars = solver->get_ncols();
      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
      std::vector<double> ub(n_vars);
      solver->get_ub(ub.data(), 0, n_vars - 1);

      // Hand management of infinites
      // +infty is set to 1e20
      for (int i(0); i < solver->get_ncols(); i++) {
        if (datas[inst]._ub[i] == 1e20 && ub[i] > 1e20) {
          ub[i] = 1e20;
        }
      }
      REQUIRE(ub == datas[inst]._ub);
    }
  }
}

TEST_CASE(
    "MPS file can be read and we can get every information about the problem",
    "[read]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB, NET_MASTER,
                       NET_SP1, NET_SP2, SLACKS, REDUCED);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      //========================================================================================
      // Required datas
      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
      REQUIRE(solver->get_nrows() == datas[inst]._nrows);
      REQUIRE(solver->get_n_integer_vars() == datas[inst]._nintegervars);
      REQUIRE(solver->get_nelems() == datas[inst]._nelems);

      //========================================================================================
      // Read objective
      int n_vars = solver->get_ncols();
      std::vector<double> obj(n_vars);

      solver->get_obj(obj.data(), 0, n_vars - 1);

      REQUIRE(obj == datas[inst]._obj);

      //========================================================================================
      // Read constraints
      int n_elems = solver->get_nelems();
      int n_cstr = solver->get_nrows();

      std::vector<double> matval(n_elems);
      std::vector<int> mstart(n_cstr + 1);
      std::vector<int> mind(n_elems);

      int n_returned(0);
      solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                       &n_returned, 0, n_cstr - 1);

      REQUIRE(n_returned == datas[inst]._nelems);
      REQUIRE(matval == datas[inst]._matval);
      REQUIRE(mind == datas[inst]._mind);
      REQUIRE(mstart == datas[inst]._mstart);

      //========================================================================================
      // Read constraints RHS
      n_cstr = solver->get_nrows();
      std::vector<double> rhs(n_cstr);
      if (n_cstr > 0) {
        solver->get_rhs(rhs.data(), 0, n_cstr - 1);
      }

      REQUIRE(rhs == datas[inst]._rhs);

      //========================================================================================
      // Read row types
      n_cstr = solver->get_nrows();
      std::vector<char> rtypes(n_cstr);
      if (n_cstr > 0) {
        solver->get_row_type(rtypes.data(), 0, n_cstr - 1);
      }
      REQUIRE(rtypes == datas[inst]._rowtypes);

      //========================================================================================
      // Read Column types
      n_vars = solver->get_ncols();
      std::vector<char> coltype(n_vars);
      solver->get_col_type(coltype.data(), 0, n_vars - 1);

      REQUIRE(coltype == datas[inst]._coltypes);

      //========================================================================================
      // Read lower bounds on variables
      n_vars = solver->get_ncols();
      std::vector<double> lb(n_vars);
      solver->get_lb(lb.data(), 0, n_vars - 1);

      REQUIRE(lb == datas[inst]._lb);

      //========================================================================================
      // Read upper bounds on variables
      n_vars = solver->get_ncols();
      std::vector<double> ub(n_vars);
      solver->get_ub(ub.data(), 0, n_vars - 1);

      for (int i(0); i < solver->get_ncols(); i++) {
        if (datas[inst]._ub[i] == 1e20 && ub[i] > 1e20) {
          ub[i] = 1e20;
        }
      }
      REQUIRE(ub == datas[inst]._ub);
    }
  }
}

TEST_CASE(
    "We can get the names of variables and constraints present in MPS file "
    "after read",
    "[read][read-names]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;
  int ind = 0;
  auto inst = GENERATE(MIP_TOY, MULTIKP);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      if (solver_name == "XPRESS") {
        auto prb_name = std::filesystem::path("test" + ind);
        solver->write_prob_mps(prb_name);
        ind++;
      }
      //========================================================================================
      // Get names of columns and rows
      int n_vars = solver->get_ncols();
      int n_rows = solver->get_nrows();

      std::vector<std::string> rowNames = solver->get_row_names(0, n_rows - 1);
      std::vector<std::string> colNames = solver->get_col_names(0, n_vars - 1);

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

TEST_CASE("We can get the indices of rows and columns by their names",
          "[read][get-indices]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;
  int ind = 0;
  auto inst = GENERATE(MIP_TOY, MULTIKP);
  SECTION("Reading instance") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;
      //========================================================================================
      // Solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      REQUIRE(solver->get_number_of_instances() == 1);
      
      solver->read_prob_mps(instance);

      if (solver_name == "XPRESS") {
        auto prb_name = std::filesystem::path("test" + ind);
        solver->write_prob_mps(prb_name);
        ind++;
      }
      //========================================================================================
      // Get row and columns indices by their names

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

TEST_CASE("Testing copy constructor", "[init][copy-constructor]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP);
  SECTION("Construction and destruction") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance= datas[inst]._path;

      //========================================================================================
      // Intial solver declaration and read problem
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
            solver->read_prob_mps(instance);
      REQUIRE(solver->get_number_of_instances() == 1);

      REQUIRE(solver->get_ncols() == datas[inst]._ncols);
      REQUIRE(solver->get_nrows() == datas[inst]._nrows);

      //========================================================================================
      // Declare copy prob
      SolverAbstract::Ptr solver2 = factory.copy_solver(solver);
      REQUIRE(solver2->get_number_of_instances() == 2);

      REQUIRE(solver2->get_ncols() == datas[inst]._ncols);
      REQUIRE(solver2->get_nrows() == datas[inst]._nrows);

      //========================================================================================
      // Delete initial solver
      solver.reset();
      REQUIRE(solver == nullptr);
      REQUIRE(solver2->get_number_of_instances() == 1);

      REQUIRE(solver2->get_ncols() == datas[inst]._ncols);
      REQUIRE(solver2->get_nrows() == datas[inst]._nrows);

      //========================================================================================
      // Check variable names
      int ncols_copied = solver2->get_ncols();
      std::vector<std::string> copiedNames =
          solver2->get_col_names(0, ncols_copied - 1);

      std::string cur_name = "";
      for (int i = 0; i < ncols_copied; i++) {
        cur_name = datas[inst]._col_names[i];
        REQUIRE(copiedNames[i].compare(0, cur_name.size(), cur_name) == 0);
      }

      //========================================================================================
      // solvers destruction
      solver2.reset();
      REQUIRE(solver2 == nullptr);
    }
  }
}