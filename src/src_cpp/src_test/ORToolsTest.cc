#include "gtest/gtest.h"

#include <sstream>
#include <fstream>

#include "ortools_utils.h"


#include "ortools/linear_solver/linear_solver.pb.h"

class ORToolsTest : public ::testing::Test
{
protected:
	static void SetUpTestCase()
	{
		// called before 1st test
		std::string content_l = "\
NAME          EXAMPLE\n\
ROWS\n\
 N  OBJ\n\
 G  ROW01\n\
 L  ROW02\n\
 E  ROW03\n\
 G  ROW04\n\
 L  ROW05\n\
COLUMNS\n\
    COL01     OBJ                1.0\n\
    COL01     ROW01              3.0   ROW05              5.6\n\
    COL02     ROW01              1.0   ROW02              2.0\n\
    COL03     ROW02              1.1   ROW03              1.0\n\
    COL04     ROW01             -2.0   ROW04              2.8\n\
    COL05     OBJ                2.0\n\
    COL05     ROW01             -1.0   ROW05              1.0\n\
    COL06     ROW03              1.0\n\
    COL07     ROW04             -1.2\n\
    COL08     OBJ               -1.0\n\
    COL08     ROW01             -1.0   ROW05              1.9\n\
RHS\n\
    RHS1      ROW01              2.5\n\
    RHS1      ROW02              2.1\n\
    RHS1      ROW03              4.0\n\
    RHS1      ROW04              1.8\n\
    RHS1      ROW05             15.0\n\
RANGES\n\
    RNG1      ROW04              3.2\n\
    RNG1      ROW05             12.0\n\
BOUNDS\n\
 LO BND1      COL01              2.5\n\
 UP BND1      COL02              4.1\n\
 LO BND1      COL05              0.5\n\
 UP BND1      COL05              4.0\n\
 UP BND1      COL08              4.3\n\
ENDATA\
";

	// dummy mps tmp file name
	std::ofstream file_l("temp_mps_problem.mps");
	file_l << content_l;
	file_l.close();
	}

	static void TearDownTestCase()
	{
		// called after last test

		//delete the created tmp file
		std::remove("temp_mps_problem.mps");
	}

	void SetUp()
	{
		// called before each test
		_solver = new operations_research::MPSolver("mainSolver", ORTOOLS_LP_SOLVER_TYPE);
		ORTreadmps(*_solver, "temp_mps_problem.mps");
		infinity_ = _solver->infinity();
		nbConstarints_ = 5;
		nbVariables_ = 8;
	}

   void TearDown()
	{
		// called after each test
		delete _solver;
		_solver = nullptr;
	}

	operations_research::MPSolver * _solver;
	double doubleTolerance_ = 1e-15;
	double infinity_ = 1e16;
	double nbConstarints_ = 0;
	double nbVariables_ = 0;
};

TEST_F(ORToolsTest, testReadMPS)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	// std::ostringstream oss;
	// ORTdescribe(*_solver, oss);
	// std::cout << "\n" << oss.str() ;

	// ORTwritemps(*_solver, "exported.mps");
}


TEST_F(ORToolsTest, testDeleteRows)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	ORTdeactivaterows(*_solver, {0,3});

	// std::ostringstream oss;
	// ORTdescribe(*_solver, oss);
	// std::cout << "\n" << oss.str();

	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);
}


TEST_F(ORToolsTest, testGetRows)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	std::vector<int> mstart;
	std::vector<int> mclind;
	std::vector<double> dmatval;
	ORTgetrows(*_solver, mstart , mclind, dmatval, 0, 4);

	// std::cout << "\nmstart : ";
	// for(auto el : mstart)
	// {
	// 	std::cout << el << " ";
	// }
	// std::cout << "\nmclind : ";
	// for(auto el : mclind)
	// {
	// 	std::cout << el << " ";
	// }
	// std::cout << "\ndmatval : ";
	// for(auto el : dmatval)
	// {
	// 	std::cout << el << " ";
	// }

	std::vector<int> expected_mstart = {0, 5, 7, 9, 11};
	ASSERT_EQ(expected_mstart.size(), mstart.size()) << "mstart size different";
	for ( size_t i = 0 ; i < expected_mstart.size() ; ++i )
	{
		EXPECT_EQ(expected_mstart[i], mstart[i]) << "(i) = (" << i << ")";
	}

	int row_l(0);
	for(auto constraint : _solver->constraints())
	{
		int endIndex_l = (row_l == mstart.size()-1) ? mclind.size() : mstart[row_l+1];
		// std::cout << "\nconstraint " << constraint->name() << " ends before " << endIndex_l << ": \n";
		for(int cnt_l(mstart[row_l]); cnt_l < endIndex_l; ++cnt_l)
		{
			int varInd(mclind[cnt_l]);
			double varCoeff(dmatval[cnt_l]);
			// std::cout << "index " << cnt_l << ": coeff of variable " << _solver->variables()[varInd]->name()
			// 			<< " is " << constraint->GetCoefficient(_solver->variables()[varInd]) << "\n";
			ASSERT_EQ(varCoeff, constraint->GetCoefficient(_solver->variables()[varInd]));
		}
		++row_l;
	}
}


TEST_F(ORToolsTest, testORTchgobj)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	ORTchgobj(*_solver, {0, 1, 3, 7}, {5, 2.2, 1.5, 0});

	std::map<int, double> expected_mapVarCoeff = {
		{0, 5},
		{1, 2.2},
		{2, 0},
		{3, 1.5},
		{4, 2},
		{5, 0},
		{6, 0},
		{7, 0},
	};

	operations_research::MPObjective const & objective_l(_solver->Objective());
	for(auto pairVarCoeff : objective_l.terms())
	{
		ASSERT_EQ( expected_mapVarCoeff[pairVarCoeff.first->index()], pairVarCoeff.second)
				<< "var " << pairVarCoeff.first->index() << " has wrong coeff " <<  pairVarCoeff.second;
	}
}


TEST_F(ORToolsTest, testORTgetobj)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

 	std::vector<double> obj;

	ORTgetobj(*_solver, obj, 0, nbVariables_-1);
	std::vector<int> expected_fullObj = {1, 0, 0, 0, 2, 0, 0, -1};
	ASSERT_EQ(expected_fullObj.size(), obj.size()) << "obj size different";
	for ( size_t i = 0 ; i < expected_fullObj.size() ; ++i )
	{
		EXPECT_EQ(expected_fullObj[i], obj[i]) << "(i) = (" << i << ")";
	}

	ORTgetobj(*_solver, obj, 0, 5);
	std::vector<int> expected_truncObj = {1, 0, 0, 0, 2, 0};
	ASSERT_EQ(expected_truncObj.size(), obj.size()) << "obj size different";
	for ( size_t i = 0 ; i < expected_truncObj.size() ; ++i )
	{
		EXPECT_EQ(expected_truncObj[i], obj[i]) << "(i) = (" << i << ")";
	}
}


TEST_F(ORToolsTest, testORTaddcols)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);


	//WARN: the created solver is an LP solver so integer values will be solved as Continuous
	ORTaddcols(*_solver, {1, 2, -5}, {0, 2, 4}, {0, 2, 1, 3, 2, 4}, {1, 5.2, 1, 5, 1, -1}, {0, -8, 15}, {1, 5, 28}, {'B', 'I', 'C'});

	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), 11);

	std::map<int, double> expected_mapVarCoeff = {
		{0, 1},
		{1, 0},
		{2, 0},
		{3, 0},
		{4, 2},
		{5, 0},
		{6, 0},
		{7, -1},
		{8, 1},
		{9, 2},
		{10, -5},
	};
	operations_research::MPObjective const & objective_l(_solver->Objective());
	for(auto pairVarCoeff : objective_l.terms())
	{
		ASSERT_EQ( expected_mapVarCoeff[pairVarCoeff.first->index()], pairVarCoeff.second)
				<< "var " << pairVarCoeff.first->index() << " has wrong coeff " <<  pairVarCoeff.second;
	}

	std::vector<int> mstart;
	std::vector<int> mclind;
	std::vector<double> dmatval;
	ORTgetrows(*_solver, mstart , mclind, dmatval, 0, 4);
	ASSERT_EQ(dmatval.size(), 20);
	ASSERT_EQ(mclind.size(), 20);
	std::vector<int> expected_mstart = {0, 6, 9, 13, 16};
	ASSERT_EQ(expected_mstart.size(), mstart.size()) << "mstart size different";
	for ( size_t i = 0 ; i < expected_mstart.size() ; ++i )
	{
		EXPECT_EQ(expected_mstart[i], mstart[i]) << "(i) = (" << i << ")";
	}
	int row_l(0);
	for(auto constraint : _solver->constraints())
	{
		int endIndex_l = (row_l == mstart.size()-1) ? mclind.size() : mstart[row_l+1];
		// std::cout << "\nconstraint " << constraint->name() << " ends before " << endIndex_l << ": \n";
		for(int cnt_l(mstart[row_l]); cnt_l < endIndex_l; ++cnt_l)
		{
			int varInd(mclind[cnt_l]);
			double varCoeff(dmatval[cnt_l]);
			// std::cout << "index " << cnt_l << ": coeff of variable " << _solver->variables()[varInd]->name()
			// 			<< " is " << constraint->GetCoefficient(_solver->variables()[varInd]) << "\n";
			ASSERT_EQ(varCoeff, constraint->GetCoefficient(_solver->variables()[varInd]));
		}
		++row_l;
	}


	std::vector<char> coltype_l;
	std::vector<double> bdl_l;
	std::vector<double> bdu_l;
	ORTgetcolinfo(*_solver, coltype_l, bdl_l, bdu_l, 0, _solver->NumVariables()-1);

	//WARN: the created solver is an LP solver so integer values will be solved as Continuous
	std::vector<char> expected_coltype =  {'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'B', 'I', 'C'};
	ASSERT_EQ(expected_coltype.size(), coltype_l.size()) << "column type vector size different";
	for ( size_t i = 0 ; i < expected_coltype.size() ; ++i )
	{
		EXPECT_EQ(expected_coltype[i], coltype_l[i]) << "(i) = (" << i << ")";
	}
}


TEST_F(ORToolsTest, testORTaddrows)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	ORTaddrows(*_solver, {'E','L', 'G', 'R'}, {9, 17, 1, 21}, {0, 0, 0, 23}, {0,3,5,7}, {0,1,2,5,7,0,4,0,6}, {-1, 5, 3, 16, 4, 11, -3.2, 1, 1});

	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_+4);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	std::vector<int> mstart;
	std::vector<int> mclind;
	std::vector<double> dmatval;

	//partial matrix
	ORTgetrows(*_solver, mstart , mclind, dmatval, 0, 4);
	ASSERT_EQ(dmatval.size(), 14);
	std::vector<int> expected_mstart =  {0, 5, 7, 9, 11};
	ASSERT_EQ(expected_mstart.size(), mstart.size()) << "mstart size different";
	for ( size_t i = 0 ; i < expected_mstart.size() ; ++i )
	{
		EXPECT_EQ(expected_mstart[i], mstart[i]) << "(i) = (" << i << ")";
	}
	auto vectConstraints_l = _solver->constraints();
	for(int rowInd_l(0); rowInd_l < 5; ++rowInd_l)
	{
		operations_research::MPConstraint* constraint = vectConstraints_l[rowInd_l];

		int endIndex_l = (rowInd_l == mstart.size()-1) ? mclind.size() : mstart[rowInd_l+1];
		for(int cnt_l(mstart[rowInd_l]); cnt_l < endIndex_l; ++cnt_l)
		{
			int varInd(mclind[cnt_l]);
			double varCoeff(dmatval[cnt_l]);
			ASSERT_EQ(varCoeff, constraint->GetCoefficient(_solver->variables()[varInd]));
		}
		++rowInd_l;
	}

	//full
	ORTgetrows(*_solver, mstart , mclind, dmatval, 0, _solver->constraints().size()-1);
	ASSERT_EQ(dmatval.size(), 23);
	expected_mstart =  {0, 5, 7, 9, 11, 14, 17, 19, 21};
	ASSERT_EQ(expected_mstart.size(), mstart.size()) << "mstart size different";
	for ( size_t i = 0 ; i < expected_mstart.size() ; ++i )
	{
		EXPECT_EQ(expected_mstart[i], mstart[i]) << "(i) = (" << i << ")";
	}
	int row_l = 0;
	for(auto constraint : _solver->constraints())
	{
		int endIndex_l = (row_l == mstart.size()-1) ? mclind.size() : mstart[row_l+1];
		for(int cnt_l(mstart[row_l]); cnt_l < endIndex_l; ++cnt_l)
		{
			int varInd(mclind[cnt_l]);
			double varCoeff(dmatval[cnt_l]);
			ASSERT_EQ(varCoeff, constraint->GetCoefficient(_solver->variables()[varInd]));
		}
		++row_l;
	}

}


TEST_F(ORToolsTest, testORTgetlpsol)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	std::vector<double> x;
	std::vector<double> dual;
	std::vector<double> dj;

	_solver->Solve();
	ORTgetlpsolution(*_solver, x);
	ORTgetlpdual(*_solver, dual);
	ORTgetlpreducedcost(*_solver, dj);

	std::vector<double> expected_solution =  {2.5, 1.05, 0, 0.6428571428571429, 0.5, 4, 0, 0.26315789473684209};//cplex solution
	std::vector<double> expected_dual =  {0, 0, 0, 0, -0.52631578947368418};//cplex solution
	std::vector<double> expected_rc =  {3.9473684210526314, 0, 0, 0, 2.5263157894736841, 0, 0, 0};//cplex solution

	ASSERT_EQ(expected_solution.size(), x.size()) << "solution size different";
	for ( size_t i = 0 ; i < expected_solution.size() ; ++i )
	{
		EXPECT_NEAR(expected_solution[i], x[i], doubleTolerance_) << "(i) = (" << i << ")";
	}
	ASSERT_EQ(expected_dual.size(), dual.size()) << "dual size different";
	for ( size_t i = 0 ; i < expected_dual.size() ; ++i )
	{
		EXPECT_NEAR(expected_dual[i], dual[i], doubleTolerance_) << "(i) = (" << i << ")";
	}
	ASSERT_EQ(expected_rc.size(), dj.size()) << "reduced cost size different";
	for ( size_t i = 0 ; i < expected_rc.size() ; ++i )
	{
		EXPECT_NEAR(expected_rc[i], dj[i], doubleTolerance_) << "(i) = (" << i << ")";
	}
}


TEST_F(ORToolsTest, testORTgetRowtypeAndRhs)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	std::vector<char> rowtypes;
	ORTgetrowtype(*_solver, rowtypes, 0, _solver->NumConstraints()-1);

	std::vector<char> expected_rowtypes =  {'G', 'L', 'E', 'R', 'R'};
	ASSERT_EQ(expected_rowtypes.size(), rowtypes.size()) << "rowtype vector size different";
	for ( size_t i = 0 ; i < expected_rowtypes.size() ; ++i )
	{
		EXPECT_EQ(expected_rowtypes[i], rowtypes[i]) << "(i) = (" << i << ")";
	}

	operations_research::MPConstraint * rangeConstraint_l( _solver->MakeRowConstraint(1, 12) );
	rangeConstraint_l->SetCoefficient(_solver->LookupVariableOrNull("COL02"), 2);
	rangeConstraint_l->SetCoefficient(_solver->LookupVariableOrNull("COL06"), 1.5);
	ORTgetrowtype(*_solver, rowtypes, 0, _solver->NumConstraints()-1);

	expected_rowtypes =  {'G', 'L', 'E', 'R', 'R', 'R'};
	ASSERT_EQ(expected_rowtypes.size(), rowtypes.size()) << "updated row type vector size different";
	for ( size_t i = 0 ; i < expected_rowtypes.size() ; ++i )
	{
		EXPECT_EQ(expected_rowtypes[i], rowtypes[i]) << "(i) = (" << i << ")";
	}

	std::vector<double> rhs;
	ORTgetrhs(*_solver, rhs, 0,  _solver->NumConstraints()-1);
	std::vector<double> expected_rhs =  {2.5, 2.1, 4, 5, 15, 12};
	ASSERT_EQ(expected_rhs.size(), rhs.size()) << "rowtype vector size different";
	for ( size_t i = 0 ; i < expected_rhs.size() ; ++i )
	{
		EXPECT_EQ(expected_rhs[i], rhs[i]) << "(i) = (" << i << ")";
	}

	std::vector<double> range;
	ORTgetrhsrange(*_solver, range, 0,  _solver->NumConstraints()-1);
	std::vector<double> expected_range =  {infinity_, infinity_, 0, 3.2, 12, 11};
	ASSERT_EQ(expected_range.size(), range.size()) << "rowtype vector size different";
	for ( size_t i = 0 ; i < expected_range.size() ; ++i )
	{
		EXPECT_EQ(expected_range[i], range[i]) << "(i) = (" << i << ")";
	}
}

TEST_F(ORToolsTest, testORTdeactivaterows)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	_solver->Solve();
	double solValue_l = _solver->Objective().Value();
	EXPECT_NEAR(solValue_l, 3.236842105263158, doubleTolerance_);

	//deactivate Row04 and ROW05
	ORTdeactivaterows(*_solver, {3,4});

	//number of constraints does not change : contraints still exist but are empty
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(0, _solver->constraints()[3]->terms().size()) << "row04 has terms";
	ASSERT_EQ(0, _solver->constraints()[4]->terms().size()) << "row05 has terms";

	_solver->Solve();
	double newSolValue_l = _solver->Objective().Value();
	EXPECT_NEAR(newSolValue_l, -0.8, doubleTolerance_);
}

TEST_F(ORToolsTest, testproblem)
{
	operations_research::MPSolver solver("simple_mip_program", ORTOOLS_LP_SOLVER_TYPE);

	const double infinity = solver.infinity();
	// x and y are integer non-negative variables.
	operations_research::MPVariable* const x = solver.MakeIntVar(0.0, 3.5, "x");
	operations_research::MPVariable* const y = solver.MakeIntVar(0.0, infinity, "y");

	// x + 7 * y <= 17.5.
	operations_research::MPConstraint* const c0 = solver.MakeRowConstraint(-infinity, 17.5, "c0");
	c0->SetCoefficient(x, 1);
	c0->SetCoefficient(y, 7);

	// 10 * x + y >= 2.5.
	operations_research::MPConstraint* const c1 = solver.MakeRowConstraint(3, infinity, "c1");
	c1->SetCoefficient(x, 10);
	c1->SetCoefficient(y, 1);

	// Maximize x + 10 * y.
	operations_research::MPObjective* const objective = solver.MutableObjective();
	objective->SetCoefficient(x, 1);
	objective->SetCoefficient(y, 10);
	objective->SetMaximization();

	const int result_status = solver.Solve();

	EXPECT_EQ(result_status, 0);

	std::vector<int> rstatus_l;
	std::vector<int> cstatus_l;
	ORTgetbasis(solver, rstatus_l, cstatus_l);

	ASSERT_EQ(2, rstatus_l.size());
	ASSERT_EQ(2, cstatus_l.size());

	std::vector<int> expected_rowstatus =  {2, 0};
	ASSERT_EQ(expected_rowstatus.size(), rstatus_l.size()) << "rstatus_l vector size different";
	for ( size_t i = 0 ; i < expected_rowstatus.size() ; ++i )
	{
		EXPECT_EQ(expected_rowstatus[i], rstatus_l[i]) << "(i) = (" << i << ")";
	}

	std::vector<int> expected_colstatus =  {1, 1};
	ASSERT_EQ(expected_colstatus.size(), cstatus_l.size()) << "cstatus_l vector size different";
	for ( size_t i = 0 ; i < expected_colstatus.size() ; ++i )
	{
		EXPECT_EQ(expected_colstatus[i], cstatus_l[i]) << "(i) = (" << i << ")";
	}
}


TEST_F(ORToolsTest, testORTgetbasis)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	_solver->Solve();
	double solValue_l = _solver->Objective().Value();
	EXPECT_NEAR(solValue_l, 3.236842105263158, doubleTolerance_);

	std::vector<int> rstatus_l;
	std::vector<int> cstatus_l;
	ORTgetbasis(*_solver, rstatus_l, cstatus_l);

	//ROW03 is actually fixed
	std::vector<int> expected_rowstatus =  {1, 2, 0, 0, 2};
	ASSERT_EQ(expected_rowstatus.size(), rstatus_l.size()) << "rstatus_l vector size different";
	for ( size_t i = 0 ; i < expected_rowstatus.size() ; ++i )
	{
		EXPECT_EQ(expected_rowstatus[i], rstatus_l[i]) << "(i) = (" << i << ")";
	}

	std::vector<int> expected_colstatus =  {0, 1, 0, 1, 0, 1, 0, 1};
	ASSERT_EQ(expected_colstatus.size(), cstatus_l.size()) << "cstatus_l vector size different";
	for ( size_t i = 0 ; i < expected_colstatus.size() ; ++i )
	{
		EXPECT_EQ(expected_colstatus[i], cstatus_l[i]) << "(i) = (" << i << ")";
	}
}

TEST_F(ORToolsTest, testORTchgbounds)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	std::vector<int> const mindex_l = {1,0,4};
	std::vector<char> const qbtype_l = {'B','L','U'};
	std::vector<double> const bnd_l = {0,1.5,2};
	ORTchgbounds(*_solver, mindex_l, qbtype_l, bnd_l);

	EXPECT_EQ(_solver->variables()[0]->lb(), 1.5);
	EXPECT_EQ(_solver->variables()[1]->lb(), 0);
	EXPECT_EQ(_solver->variables()[1]->ub(), 0);
	EXPECT_EQ(_solver->variables()[4]->ub(), 2);
}

TEST_F(ORToolsTest, testORTcopy)
{
	ASSERT_EQ(_solver->NumConstraints(), nbConstarints_);
	ASSERT_EQ(_solver->NumVariables(), nbVariables_);

	// std::ostringstream oss_l;
	// ORTdescribe(*_solver, oss_l);

	operations_research::MPSolver outSolver_l("copy", ORTOOLS_LP_SOLVER_TYPE);
	ORTcopyandrenamevars(outSolver_l, *_solver, {"y0","y1","y2","y3","y4","y5","y6","y7"});

	// ORTdescribe(outSolver_l, oss_l);
	// std::cout << "\n" << oss_l.str();

	std::vector<double> x;
	std::vector<double> dual;
	std::vector<double> dj;
	outSolver_l.Solve();
	ORTgetlpsolution(outSolver_l, x);
	ORTgetlpdual(outSolver_l, dual);
	ORTgetlpreducedcost(outSolver_l, dj);
	std::vector<double> expected_solution =  {2.5, 1.05, 0, 0.6428571428571429, 0.5, 4, 0, 0.26315789473684209};//cplex solution
	std::vector<double> expected_dual =  {0, 0, 0, 0, -0.52631578947368418};//cplex solution
	std::vector<double> expected_rc =  {3.9473684210526314, 0, 0, 0, 2.5263157894736841, 0, 0, 0};//cplex solution
	ASSERT_EQ(expected_solution.size(), x.size()) << "solution size different";
	for ( size_t i = 0 ; i < expected_solution.size() ; ++i )
	{
		EXPECT_NEAR(expected_solution[i], x[i], doubleTolerance_) << "(i) = (" << i << ")";
	}
	ASSERT_EQ(expected_dual.size(), dual.size()) << "dual size different";
	for ( size_t i = 0 ; i < expected_dual.size() ; ++i )
	{
		EXPECT_NEAR(expected_dual[i], dual[i], doubleTolerance_) << "(i) = (" << i << ")";
	}
	ASSERT_EQ(expected_rc.size(), dj.size()) << "reduced cost size different";
	for ( size_t i = 0 ; i < expected_rc.size() ; ++i )
	{
		EXPECT_NEAR(expected_rc[i], dj[i], doubleTolerance_) << "(i) = (" << i << ")";
	}

}