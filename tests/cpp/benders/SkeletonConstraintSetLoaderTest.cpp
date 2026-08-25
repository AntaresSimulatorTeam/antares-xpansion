#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "LoggerStub.h"
#include "RecordingSolver.h"
#include "antares-xpansion/benders/benders_core/SkeletonConstraintSetLoader.h"

class SkeletonConstraintSetLoaderTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        tmp_dir_ = std::filesystem::temp_directory_path() / ("constraints_builder_test_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()));
        constraints_dir_ = tmp_dir_ / "constraints";
        std::filesystem::create_directories(constraints_dir_);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(tmp_dir_);
    }

    void write_file(const std::filesystem::path& path, const std::string& content)
    {
        std::ofstream ofs(path);
        ofs << content;
    }

    void write_constraints_fixture()
    {
        write_file(constraints_dir_ / "coef.csv", "myconstraints,1.0,2.0\n");
        write_file(constraints_dir_ / "coef_cols.csv", "x1,x2\n");
        write_file(constraints_dir_ / "coef_rows.csv", "row1,row2\n");
        write_file(constraints_dir_ / "rhs.csv", "myconstraints,100.0\n");
        write_file(constraints_dir_ / "rhs_rows.csv", "row1\n");
    }

    std::shared_ptr<RecordingSolver> solver_ = std::make_shared<RecordingSolver>();
    Logger logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
    std::filesystem::path tmp_dir_;
    std::filesystem::path constraints_dir_;
};

TEST_F(SkeletonConstraintSetLoaderTest, ConstructsWithValidFiles)
{
    write_constraints_fixture();
    ASSERT_NO_THROW((SkeletonConstraintSetLoader(tmp_dir_, logger_, solver_)));
}

// SkeletonConstraintSetLoader currently constructs its MemoptimUtils with an
// empty subproblem-name filter set (both constructors), and
// MemoptimUtils::read_keyed_coeffs_csv skips any CSV row whose key isn't in
// that set. This test documents the resulting current behavior: even with a
// non-empty coef.csv/rhs.csv, no coefficients end up loaded. See the "Flagged,
// not fixed" section of the refactor plan.
TEST_F(SkeletonConstraintSetLoaderTest, GetConstraintsNumberIsZeroDueToEmptyMemoptimFilter)
{
    write_constraints_fixture();
    SkeletonConstraintSetLoader builder(tmp_dir_, logger_, solver_);
    EXPECT_EQ(builder.GetConstraintsNumber(), 0);
}

TEST_F(SkeletonConstraintSetLoaderTest, LoadConstraintSetAppliesEmptyVectorsGivenCurrentFilter)
{
    write_constraints_fixture();
    SkeletonConstraintSetLoader builder(tmp_dir_, logger_, solver_);

    auto returned_solver = builder.LoadConstraintSet("myconstraints");

    EXPECT_EQ(returned_solver, solver_);
    EXPECT_EQ(solver_->chg_coefs_calls, 1);
    EXPECT_EQ(solver_->chg_rhs_values_calls, 1);
    // operator[] on the unpopulated coeffs_/rhs_ maps inserts empty vectors,
    // consistent with GetConstraintsNumberIsZeroDueToEmptyMemoptimFilter above.
    EXPECT_TRUE(solver_->chg_coefs_vals.empty());
    EXPECT_TRUE(solver_->chg_rhs_vals.empty());
}
