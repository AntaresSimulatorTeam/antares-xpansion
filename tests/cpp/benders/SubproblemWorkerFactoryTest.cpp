#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "LoggerStub.h"
#include "NOOPSolver.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h"

class SubproblemWorkerFactoryTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        tmpDir_ = std::filesystem::temp_directory_path() / ("skeleton_test_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()));
        subDir_ = tmpDir_ / "sub";
        std::filesystem::create_directories(subDir_);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(tmpDir_);
    }

    void writeFile(const std::filesystem::path& path, const std::string& content)
    {
        std::ofstream ofs(path);
        ofs << content;
    }

    void writeTwoSubFixture()
    {
        writeFile(subDir_ / "coef.csv", "sub1,1.0,2.0,3.0\nsub2,4.0,5.0,6.0\n");
        writeFile(subDir_ / "coef_cols.csv", "x1,x2,x3\n");
        writeFile(subDir_ / "coef_rows.csv", "row1,row2,row3\n");
        writeFile(subDir_ / "obj_coef.csv", "sub1,10.0,20.0\nsub2,30.0,40.0\n");
        writeFile(subDir_ / "obj_cols.csv", "x1,x2\n");
        writeFile(subDir_ / "rhs.csv", "sub1,100.0\nsub2,200.0\n");
        writeFile(subDir_ / "rhs_rows.csv", "row1\n");
    }

    void writeEmptyFixture()
    {
        writeFile(subDir_ / "coef.csv", "");
        writeFile(subDir_ / "coef_cols.csv", "");
        writeFile(subDir_ / "coef_rows.csv", "");
        writeFile(subDir_ / "obj_coef.csv", "");
        writeFile(subDir_ / "obj_cols.csv", "");
        writeFile(subDir_ / "rhs.csv", "");
        writeFile(subDir_ / "rhs_rows.csv", "");
    }

    void writeSingleSubFixture()
    {
        writeFile(subDir_ / "coef.csv", "sub1,1.5,2.5\n");
        writeFile(subDir_ / "coef_cols.csv", "x1,x2\n");
        writeFile(subDir_ / "coef_rows.csv", "row1,row2\n");
        writeFile(subDir_ / "obj_coef.csv", "sub1,10.0\n");
        writeFile(subDir_ / "obj_cols.csv", "x1\n");
        writeFile(subDir_ / "rhs.csv", "sub1,100.0\n");
        writeFile(subDir_ / "rhs_rows.csv", "row1\n");
    }

    std::unique_ptr<SubproblemWorkerFactory> buildDummyFactory()
    {
        std::vector<std::string> sub_names = {"sub1", "sub2"};
        solver_->set_nrows(2);
        solver_->set_ncols(2);
        std::vector<double> obj = {2.0, 4.5};
        solver_->set_obj(obj.data(), 0, 1);
        solver_->set_rhs({2.0, 3.0});
        solver_->set_constraints({{2.0, 0.0}, {0.0, 4.0}});
        return std::make_unique<SubproblemWorkerFactory>(
          tmpDir_, logger_, solver_, std::move(sub_names));
    }

    Logger logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
    std::filesystem::path tmpDir_;
    std::filesystem::path subDir_;
    std::shared_ptr<NOOPSolverForSubProblemFactory> solver_ =
      std::make_shared<NOOPSolverForSubProblemFactory>();
};

// ---------- Group 1: Construction & CSV parsing ----------

TEST_F(SubproblemWorkerFactoryTest, ConstructsWithValidFiles)
{
    writeTwoSubFixture();
    ASSERT_NO_THROW(buildWithNOOP());
}

TEST_F(SubproblemWorkerFactoryTest, GetSubNumberReturnsRhsKeyCount)
{
    writeTwoSubFixture();
    auto builder = buildWithNOOP();
    EXPECT_EQ(builder->GetSubNumber(), 2);
}

TEST_F(SubproblemWorkerFactoryTest, GetSubNumberZeroWithEmptyRhs)
{
    writeEmptyFixture();
    auto builder = buildWithNOOP();
    EXPECT_EQ(builder->GetSubNumber(), 0);
}

TEST_F(SubproblemWorkerFactoryTest, SingleSubproblem)
{
    writeSingleSubFixture();
    auto builder = buildWithNOOP();
    EXPECT_EQ(builder->GetSubNumber(), 1);
}

// ---------- Group 2: CreateSubSolverAbstract ----------

TEST_F(SubproblemWorkerFactoryTest, ReturnsNonNullWorker)
{
    writeTwoSubFixture();
    auto builder = buildWithNOOP();
    VariableMap variable_map;
    auto worker = builder->CreateSubSolverAbstract("sub1", variable_map, 1.0);
    EXPECT_NE(worker, nullptr);
}

TEST_F(SubproblemWorkerFactoryTest, DifferentSubsReturnDistinctWorkers)
{
    writeTwoSubFixture();
    auto builder = buildWithNOOP();
    VariableMap vm1, vm2;
    auto worker1 = builder->CreateSubSolverAbstract("sub1", vm1, 1.0);
    auto worker2 = builder->CreateSubSolverAbstract("sub2", vm2, 1.0);
    EXPECT_NE(worker1, nullptr);
    EXPECT_NE(worker2, nullptr);
    EXPECT_NE(worker1, worker2);
}

TEST_F(SubproblemWorkerFactoryTest, UnknownSubNameReturnsWorker)
{
    writeTwoSubFixture();
    auto builder = buildWithNOOP();
    VariableMap variable_map;
    // operator[] inserts empty vectors for unknown keys; NOOPSolver accepts empty vectors
    auto worker = builder->CreateSubSolverAbstract("nonexistent", variable_map, 1.0);
    EXPECT_NE(worker, nullptr);
}

// ---------- Group 3: Many subproblems ----------

TEST_F(SubproblemWorkerFactoryTest, ManySubproblems)
{
    std::string coef_csv, obj_csv, rhs_csv;
    for (int i = 0; i < 100; ++i)
    {
        std::string name = "sub" + std::to_string(i);
        coef_csv += name + ",1.0,2.0\n";
        obj_csv += name + ",10.0\n";
        rhs_csv += name + ",100.0\n";
    }
    writeFile(subDir_ / "coef.csv", coef_csv);
    writeFile(subDir_ / "coef_cols.csv", "x1,x2\n");
    writeFile(subDir_ / "coef_rows.csv", "row1,row2\n");
    writeFile(subDir_ / "obj_coef.csv", obj_csv);
    writeFile(subDir_ / "obj_cols.csv", "x1\n");
    writeFile(subDir_ / "rhs.csv", rhs_csv);
    writeFile(subDir_ / "rhs_rows.csv", "row1\n");

    std::vector<std::string> names;
    for (int i = 0; i < 100; ++i)
    {
        names.push_back("sub" + std::to_string(i));
    }
    auto builder = buildWithNOOP(std::move(names));
    EXPECT_EQ(builder->GetSubNumber(), 100);
}

// ---------- Group 4: CSV edge cases ----------

TEST_F(SubproblemWorkerFactoryTest, CsvWithKeyOnly)
{
    // A key with no values — should result in an empty coefficient vector
    writeFile(subDir_ / "coef.csv", "sub1\n");
    writeFile(subDir_ / "coef_cols.csv", "\n");
    writeFile(subDir_ / "coef_rows.csv", "\n");
    writeFile(subDir_ / "obj_coef.csv", "sub1\n");
    writeFile(subDir_ / "obj_cols.csv", "\n");
    writeFile(subDir_ / "rhs.csv", "sub1\n");
    writeFile(subDir_ / "rhs_rows.csv", "\n");

    auto builder = buildWithNOOP();
    EXPECT_EQ(builder->GetSubNumber(), 1);

    VariableMap variable_map;
    auto worker = builder->CreateSubSolverAbstract("sub1", variable_map, 1.0);
    EXPECT_NE(worker, nullptr);
}

TEST_F(SubproblemWorkerFactoryTest, CsvWithWhitespaceInValues)
{
    // std::stod handles leading/trailing whitespace
    writeFile(subDir_ / "coef.csv", "sub1, 1.0 , 2.0 \n");
    writeFile(subDir_ / "coef_cols.csv", "x1,x2\n");
    writeFile(subDir_ / "coef_rows.csv", "row1,row2\n");
    writeFile(subDir_ / "obj_coef.csv", "sub1, 10.0 \n");
    writeFile(subDir_ / "obj_cols.csv", "x1\n");
    writeFile(subDir_ / "rhs.csv", "sub1, 100.0 \n");
    writeFile(subDir_ / "rhs_rows.csv", "row1\n");

    ASSERT_NO_THROW(buildWithNOOP());
}

TEST_F(SubproblemWorkerFactoryTest, subForResolutioncreation)
{
    writeFile(subDir_ / "coef.csv", "sub1,1.0,2.0\nsub2,2.0,3.0");
    writeFile(subDir_ / "coef_cols.csv", "x1,x2\n");
    writeFile(subDir_ / "coef_rows.csv", "row1,row2\n");
    
    writeFile(subDir_ / "obj_coef.csv", "sub1,10.0\nsub2,3.0");
    writeFile(subDir_ / "obj_cols.csv", "x1\n");
    
    writeFile(subDir_ / "rhs.csv", "sub1,100.0\n");
    writeFile(subDir_ / "rhs_rows.csv", "row1\n");
    auto dummyFactory = buildDummyFactory();

    auto solver_before = dummyFactory->GetSolver();
    double obj_before[2] = {0.0, 0.0};
    double rhs_before[2] = {0.0, 0.0};
    solver_before->get_obj(obj_before, 0, 1);
    solver_before->get_rhs(rhs_before, 0, 1);

    VariableMap variable_map = {{"x1", 0}, {"x2", 1}};
    auto worker = dummyFactory->CreateSubSolverAbstract("sub1", variable_map, 1.0);
    EXPECT_NE(worker, nullptr);

    auto solver = dummyFactory->GetSolver() ; 

    double obj[2] = {0.0, 0.0};
    double rhs[2] = {0.0, 0.0};
    solver->get_obj(obj, 0, 1);

    
    solver->get_rhs(rhs, 0, 1);


    EXPECT_DOUBLE_EQ(obj[0], 10.0);
    EXPECT_DOUBLE_EQ(obj[1], 4.5);
    EXPECT_DOUBLE_EQ(rhs[0], 100.0);
    EXPECT_DOUBLE_EQ(rhs[1], 3.0);

    int mstart[2];
    int mclind[4];
    double dmatval[4];
    int nels[2];
    solver->get_rows(mstart, mclind, dmatval, 4, nels, 0, 1);

    EXPECT_DOUBLE_EQ(dmatval[0], 1.0);
    EXPECT_DOUBLE_EQ(dmatval[1], 0.0);
    EXPECT_DOUBLE_EQ(dmatval[2], 0.0);
    EXPECT_DOUBLE_EQ(dmatval[3], 2.0);

    VariableMap variable_map2 = {{"x1", 0}, {"x2", 1}};
    auto worker2 = dummyFactory->CreateSubSolverAbstract("sub2", variable_map2, 1.0);
    EXPECT_NE(worker2, nullptr);

    solver->get_obj(obj, 0, 1);
    solver->get_rhs(rhs, 0, 1);

    EXPECT_DOUBLE_EQ(obj[0], 3.0);
    EXPECT_DOUBLE_EQ(obj[1], 4.5);

    solver->get_rows(mstart, mclind, dmatval, 4, nels, 0, 1);

    EXPECT_DOUBLE_EQ(dmatval[0], 2.0);
    EXPECT_DOUBLE_EQ(dmatval[1], 0.0);
    EXPECT_DOUBLE_EQ(dmatval[2], 0.0);
    EXPECT_DOUBLE_EQ(dmatval[3], 3.0);
}


TEST_F(SubproblemWorkerFactoryTest, subForResolutioncreationSlaveWeights)
{
    writeFile(subDir_ / "coef.csv", "sub1,1.0,2.0\nsub2,2.0,3.0");
    writeFile(subDir_ / "coef_cols.csv", "x1,x2\n");
    writeFile(subDir_ / "coef_rows.csv", "row1,row2\n");
    
    writeFile(subDir_ / "obj_coef.csv", "sub1,10.0\nsub2,3.0");
    writeFile(subDir_ / "obj_cols.csv", "x1\n");
    
    writeFile(subDir_ / "rhs.csv", "sub1,100.0\n");
    writeFile(subDir_ / "rhs_rows.csv", "row1\n");
    auto dummyFactory = buildDummyFactory();

    VariableMap variable_map = {{"x1", 0}, {"x2", 1}};
    auto worker = dummyFactory->CreateSubSolverAbstract("sub1", variable_map, 0.5);
    EXPECT_NE(worker, nullptr);

    auto solver = dummyFactory->GetSolver();

    double obj[2] = {0.0, 0.0};
    solver->get_obj(obj, 0, 1);

    EXPECT_DOUBLE_EQ(obj[0], 5.0);
    EXPECT_DOUBLE_EQ(obj[1], 2.25);

    VariableMap variable_map2 = {{"x1", 0}, {"x2", 1}};
    auto worker2 = dummyFactory->CreateSubSolverAbstract("sub2", variable_map2, 0.5);
    EXPECT_NE(worker2, nullptr);

    solver->get_obj(obj, 0, 1);

    EXPECT_DOUBLE_EQ(obj[0], 1.5);
    EXPECT_DOUBLE_EQ(obj[1], 2.25);
}
