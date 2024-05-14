#include <iostream>

#include "catch2.hpp"
#include "define_datas.hpp"
#include "multisolver_interface/Solver.h"

TEST_CASE("Modification: deleting rows", "[modif][del-rows]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;

      //========================================================================================
      // solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance);

      //========================================================================================
      // Deleting a row and checking new constraints matrix
      solver->del_rows(0, 0);

      int n_elems = solver->get_nelems();
      int n_cstr = solver->get_nrows();
      std::vector<double> matval(n_elems);
      std::vector<int> mstart(n_cstr + 1);
      std::vector<int> mind(n_elems);
      int n_returned(0);
      solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                       &n_returned, 0, n_cstr - 1);

      REQUIRE(solver->get_nrows() == datas[inst]._nrows - 1);
      REQUIRE(n_returned == datas[inst]._matval_delete.size());
      REQUIRE(matval == datas[inst]._matval_delete);
      REQUIRE(mind == datas[inst]._mind_delete);
      REQUIRE(mstart == datas[inst]._mstart_delete);
    }
  }
}

TEST_CASE("Modification: add rows", "[modif][add-rows]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      // trying each type of row
      for (auto const& constr_type : {'L', 'G', 'E'}) {
        std::filesystem::path instance = datas[inst]._path;

        //========================================================================================
        // solver declaration
        SolverAbstract::Ptr solver = factory.create_solver(solver_name);
        solver->read_prob_mps(instance);

        //========================================================================================
        // Add a row to problem : creating row structure
        int n_elems = solver->get_nelems();
        int n_cstr = solver->get_nrows();
        std::vector<double> matval(n_elems);
        std::vector<int> mstart(n_cstr + 1);
        std::vector<int> mind(n_elems);

        int newRows = 1;
        int newElems = 2;
        matval = {5.0, -3.2};
        mind = {0, 1};
        mstart = {0, 2};
        std::vector<char> ntype = {constr_type};
        std::vector<double> rhs = {5.0};

        //========================================================================================
        // Add the row to solver
        solver->add_rows(newRows, newElems, ntype.data(), rhs.data(), NULL,
                         mstart.data(), mind.data(), matval.data());

        //========================================================================================
        // Check new constraint matrix
        n_elems = solver->get_nelems();
        n_cstr = solver->get_nrows();
        REQUIRE(n_elems == datas[inst]._nelems + 2);

        matval.clear();
        matval.resize(n_elems);
        mstart.clear();
        mstart.resize(n_cstr + 1);
        mind.clear();
        mind.resize(n_elems);
        int n_returned(0);
        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                         &n_returned, 0, n_cstr - 1);

        std::vector<double> neededMatval = datas[inst]._matval;
        neededMatval.push_back(5.0);
        neededMatval.push_back(-3.2);

        std::vector<int> neededMatind = datas[inst]._mind;
        neededMatind.push_back(0);
        neededMatind.push_back(1);

        std::vector<int> neededMstart = datas[inst]._mstart;
        neededMstart.push_back(neededMstart.back() + 2);

        REQUIRE(solver->get_nrows() == datas[inst]._nrows + 1);
        REQUIRE(n_elems == datas[inst]._nelems + 2);
        REQUIRE(n_returned == datas[inst]._nelems + 2);
        REQUIRE(matval == neededMatval);
        REQUIRE(mind == neededMatind);
        REQUIRE(mstart == neededMstart);

        std::vector<double> rhs_get(newRows);
        solver->get_rhs(rhs_get.data(), solver->get_nrows() - 1,
                        solver->get_nrows() - 1);
        REQUIRE(rhs_get == rhs);

        std::vector<char> rtype_get(newRows);
        solver->get_row_type(rtype_get.data(), solver->get_nrows() - 1,
                             solver->get_nrows() - 1);
        REQUIRE(rtype_get == ntype);
      }
    }
  }
}

TEST_CASE("Modification: change obj", "[modif][chg-obj]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;

      //========================================================================================
      // solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance);

      //========================================================================================
      // Modify objective function
      int n_vars = solver->get_ncols();
      std::vector<double> obj(n_vars);
      std::vector<int> ids(n_vars);
      solver->get_obj(obj.data(), 0, n_vars - 1);
      for (int i(0); i < n_vars; i++) {
        ids[i] = i;
        obj[i] -= 1;
      }
      solver->chg_obj(ids, obj);

      //========================================================================================
      // Check new objective function
      solver->get_obj(obj.data(), 0, n_vars - 1);

      std::vector<double> neededObj = datas[inst]._obj;
      for (auto& o : neededObj) {
        o -= 1;
      }
      REQUIRE(obj == neededObj);
    }
  }
}

TEST_CASE("Modification: change right-hand side", "[modif][chg-rhs]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;

      //========================================================================================
      // solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance);

      //========================================================================================
      // Change constraints right hand sides
      int n_cstr = solver->get_nrows();
      std::vector<double> rhs;
      rhs.resize(n_cstr);
      solver->get_rhs(rhs.data(), 0, n_cstr - 1);
      for (int i(0); i < n_cstr; i++) {
        solver->chg_rhs(i, rhs[i] + 1);
      }

      //========================================================================================
      // Check new RHS
      solver->get_rhs(rhs.data(), 0, n_cstr - 1);

      std::vector<double> neededRhs = datas[inst]._rhs;
      for (auto& r : neededRhs) {
        r += 1;
      }
      REQUIRE(rhs == neededRhs);
    }
  }
}

TEST_CASE("Modification: change matrix coefficient", "[modif][chg-coef]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;

      //========================================================================================
      // solver declaration
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance);

      //========================================================================================
      // Change matrix coefficient
      int n_elems = solver->get_nelems();
      int n_cstr = solver->get_nrows();

      std::vector<double> matval;
      std::vector<int> mind;
      std::vector<int> mstart;

      matval.resize(n_elems);
      mstart.resize(n_cstr + 1);
      mind.resize(n_elems);

      solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                       &n_elems, 0, n_cstr - 1);
      int idcol = mind[mind.size() - 1];
      int row = n_cstr - 1;
      double val = matval[matval.size() - 1];
      solver->chg_coef(row, idcol, val / 10.0);

      //========================================================================================
      // Check new matrix
      int n_returned(0);
      solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                       &n_returned, 0, n_cstr - 1);

      std::vector<double> neededMatval = datas[inst]._matval;
      neededMatval[neededMatval.size() - 1] /= 10;

      REQUIRE(n_returned == datas[inst]._nelems);
      REQUIRE(matval == neededMatval);
      REQUIRE(mind == datas[inst]._mind);
      REQUIRE(mstart == datas[inst]._mstart);
    }
  }
}

TEST_CASE("Modification: add columns", "[modif][add-cols]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  // auto inst = GENERATE(MIP_TOY, MULTIKP, UNBD_PRB, INFEAS_PRB);
  auto inst = GENERATE(MIP_TOY);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      // testing add 1 and 2 columns
      for (int newcol = 1; newcol < 3; newcol++) {
        std::filesystem::path instance = datas[inst]._path;

        //========================================================================================
        // solver declaration
        SolverAbstract::Ptr solver = factory.create_solver(solver_name);
        solver->read_prob_mps(instance);

        //========================================================================================
        // Add new variable to problem
        int nnz = 2 * newcol;
        std::vector<double> newobj(newcol, 3.0);
        // Offset of col j in mind and matval
        std::vector<int> nmstart(newcol);
        for (int j = 0; j < newcol; j++) {
          nmstart[j] = 2 * j;
        }
        // Row indices
        std::vector<int> nmind(nnz);
        for (int j = 0; j < newcol; j++) {
          nmind[2 * j] = 0;
          nmind[2 * j + 1] = 1;
        }
        // Values
        std::vector<double> nmatval(nnz);
        for (int j = 0; j < newcol; j++) {
          nmatval[2 * j] = 8.32;
          nmatval[2 * j + 1] = 21.78;
        }
        // LBs & UBs
        std::vector<double> lbs(newcol, 0.0);
        std::vector<double> ubs(newcol, 6.0);

        solver->add_cols(newcol, nnz, newobj.data(), nmstart.data(),
                         nmind.data(), nmatval.data(), lbs.data(), ubs.data());

        //========================================================================================
        // Check objective and rows
        REQUIRE(solver->get_ncols() == datas[inst]._ncols + newcol);

        // test obj
        int n_vars = solver->get_ncols();
        std::vector<double> obj(n_vars);
        solver->get_obj(obj.data(), 0, n_vars - 1);

        std::vector<double> neededObj = datas[inst]._obj;
        for (int j = 0; j < newcol; j++) {
          neededObj.push_back(3.0);
        }
        REQUIRE(obj == neededObj);

        // test matrix
        int n_elems = solver->get_nelems();
        int n_cstr = solver->get_nrows();
        REQUIRE(n_elems == datas[inst]._nelems + nnz);
        REQUIRE(n_cstr == datas[inst]._nrows);

        std::vector<double> matval;
        std::vector<int> mind;
        std::vector<int> mstart;

        matval.resize(n_elems);
        mstart.resize(n_cstr + 1);
        mind.resize(n_elems);

        int n_returned = 0;
        solver->get_rows(mstart.data(), mind.data(), matval.data(), n_elems,
                         &n_returned, 0, n_cstr - 1);

        std::vector<double> neededMatval = datas[inst]._matval;
        for (int j = 0; j < newcol; j++) {
          neededMatval.insert(neededMatval.begin() + datas[inst]._mstart[1] + j,
                              nmatval[0]);
          neededMatval.insert(
              neededMatval.begin() + datas[inst]._mstart[2] + 1 + j,
              nmatval[1]);
        }
        std::vector<int> neededMatind = datas[inst]._mind;
        for (int j = 0; j < newcol; j++) {
          neededMatind.insert(neededMatind.begin() + datas[inst]._mstart[1] + j,
                              datas[inst]._ncols + j);
          neededMatind.insert(
              neededMatind.begin() + datas[inst]._mstart[2] + 1 + 2 * j,
              datas[inst]._ncols + j);
        }

        std::vector<int> neededMstart = datas[inst]._mstart;
        neededMstart[1] += newcol;
        for (int j = 2; j < neededMstart.size(); j++) {
          neededMstart[j] += 2 * newcol;
        }
        REQUIRE(n_returned == datas[inst]._nelems + nnz);
        REQUIRE(matval == neededMatval);
        REQUIRE(mind == neededMatind);
        REQUIRE(mstart == neededMstart);

        // 9. test variables bounds
        n_vars = solver->get_ncols();
        lbs.resize(n_vars);
        ubs.resize(n_vars);

        solver->get_lb(lbs.data(), 0, n_vars - 1);
        solver->get_ub(ubs.data(), 0, n_vars - 1);

        std::vector<double> neededLb = datas[inst]._lb;
        std::vector<double> neededUb = datas[inst]._ub;
        for (int j = 0; j < newcol; j++) {
          neededLb.push_back(0.0);
          neededUb.push_back(6.0);
        }
        // infinite management, setting to 1e20
        for (int i(0); i < solver->get_ncols(); i++) {
          if (neededUb[i] == 1e20 && ubs[i] > 1e20) {
            ubs[i] = 1e20;
          }
        }
        REQUIRE(lbs == neededLb);
        REQUIRE(ubs == neededUb);
      }
    }
  }
}

TEST_CASE("Modification: change name of row and column", "[modif][chg-names]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(MIP_TOY, MULTIKP);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance);

      // Test change col name
      // Modifying name of Column of index 1
      std::string newColName = "testColumn";
      solver->chg_col_name(1, newColName);

      std::vector<std::string> solverColNames = solver->get_col_names(1, 1);
      /*
      WARNING : Xpress Solver adds sometimes spaces at the end of the names.
      In order to perform comparison, we only compare the str.size() first
      characters of the required names.
      */
      REQUIRE(solverColNames[0].compare(0, newColName.size(), newColName) == 0);

      // Test change row name
      // Modifying name of Row of index 1
      std::string newRowName = "testRow";
      solver->chg_row_name(1, newRowName);

      std::vector<std::string> solverRowNames = solver->get_row_names(1, 1);
      REQUIRE(solverRowNames[0].compare(0, newRowName.size(), newRowName) == 0);
    }
  }
}

TEST_CASE("Modification: add cols and a row associated to those columns",
          "[modif][add-cols-rows]") {
  AllDatas datas;
  fill_datas(datas);

  SolverFactory factory;

  auto inst = GENERATE(NET_MASTER);
  SECTION("Loop on instances") {
    for (auto const& solver_name : factory.get_solvers_list()) {
      std::filesystem::path instance = datas[inst]._path;
      SolverAbstract::Ptr solver = factory.create_solver(solver_name);
      solver->read_prob_mps(instance);

      /*--------------------------------------------------------------------------------*/
      // adding 4 columns, the first with obj 1 and the three others wih obj 0
      // first column with obj 1
      std::vector<double> obj(1, 1.0);
      std::vector<int> matstart(1, 0);
      std::vector<int> matind(0);
      std::vector<double> matval(0);
      std::vector<double> lb(1, 0.0);
      std::vector<double> ub(1, 100.0);
      solver->add_cols(1, 0, obj.data(), matstart.data(), matind.data(),
                       matval.data(), lb.data(), ub.data());

      // 3 Colunms with no obj
      matstart.resize(3);
      matstart = std::vector<int>(3, 0);
      obj.resize(3);
      obj = std::vector<double>(3, 0.0);
      lb.resize(3);
      lb = std::vector<double>(3, 0.0);
      ub.resize(3);
      ub = std::vector<double>(3, 100.0);
      solver->add_cols(3, 0, obj.data(), matstart.data(), matind.data(),
                       matval.data(), lb.data(), ub.data());

      REQUIRE(solver->get_ncols() == datas[inst]._ncols + 4);
      REQUIRE(solver->get_nrows() == datas[inst]._nrows);
      REQUIRE(solver->get_nelems() == datas[inst]._nelems);

      /*--------------------------------------------------------------------------------*/
      /* Add a row linking the 4 new variables */
      int newrows = 1;
      int newnz = 4;
      std::vector<char> rowType(newrows, 'E');
      std::vector<double> rhs(newrows, 0.0);
      std::vector<int> rowmatstart(newrows + 1);
      rowmatstart[0] = 0;
      rowmatstart[1] = newnz;

      std::vector<int> rowmatind(newnz);
      for (int i = 0; i < newnz; i++) {
        rowmatind[i] = datas[inst]._ncols + i;
      }

      std::vector<double> rowmatval(newnz, -1.0);
      rowmatval[0] = 1.0;

      solver->add_rows(newrows, newnz, rowType.data(), rhs.data(), NULL,
                       rowmatstart.data(), rowmatind.data(), rowmatval.data());

      REQUIRE(solver->get_nrows() == datas[inst]._nrows + newrows);
      REQUIRE(solver->get_nelems() == datas[inst]._nelems + newnz);

      std::vector<int> solv_matstart(newrows + 1);
      std::vector<int> solv_matind(newnz);
      std::vector<double> solv_matval(newnz);
      int n_returned;
      solver->get_rows(solv_matstart.data(), solv_matind.data(),
                       solv_matval.data(), newnz, &n_returned,
                       solver->get_nrows() - 1, solver->get_nrows() - 1);

      REQUIRE(n_returned == newnz);
      REQUIRE(solv_matstart == rowmatstart);
      REQUIRE(solv_matind == rowmatind);
      REQUIRE(solv_matval == rowmatval);
    }
  }
}