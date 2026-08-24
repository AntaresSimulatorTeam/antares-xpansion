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

    std::unique_ptr<SubproblemWorkerFactory> buildWithNOOP(
      std::vector<std::string> sub_names = {"sub1", "sub2"})
    {
        auto solver = std::make_shared<NOOPSolver>();
        return std::make_unique<SubproblemWorkerFactory>(
          tmpDir_, logger_, solver, std::move(sub_names));
    }

    Logger logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
    std::filesystem::path tmpDir_;
    std::filesystem::path subDir_;
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
