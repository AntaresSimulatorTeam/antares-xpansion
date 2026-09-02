#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "NOOPSolver.h"
#include "antares-xpansion/benders/benders_core/skeleton_coefficient_reader.h"

class SkeletonCoefficientReaderTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        tmpDir_ = std::filesystem::temp_directory_path()
                  / ("skeleton_reader_test_"
                     + std::to_string(::testing::UnitTest::GetInstance()->random_seed()));
        std::filesystem::create_directories(tmpDir_);
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

    std::filesystem::path tmpDir_;
    // NOOPSolverForSkeleton knows: x1->0, x2->1 (cols), row1->0, row2->1 (rows)
    std::shared_ptr<NOOPSolverForSkeleton> solver_ = std::make_shared<NOOPSolverForSkeleton>();
};

// ---------- Column mode (is_col = true) ----------

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_NoThrow_WhenAllColumnsFound)
{
    auto csvPath = tmpDir_ / "cols.csv";
    writeFile(csvPath, "x1,x2");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    ASSERT_NO_THROW(reader.read_indices_csv(csvPath, indices, true, solver_));
    EXPECT_EQ(indices, (std::vector<int>{0, 1}));
}

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_Throws_WhenOneColumnNotFound)
{
    auto csvPath = tmpDir_ / "cols.csv";
    writeFile(csvPath, "x1,unknown_col");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    EXPECT_THROW(reader.read_indices_csv(csvPath, indices, true, solver_),
                 SkeletonCoefficientReader::NamesNotFoundException);
}

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_ColumnException_MessageContainsCount)
{
    auto csvPath = tmpDir_ / "cols.csv";
    writeFile(csvPath, "unknown1,unknown2");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    try
    {
        reader.read_indices_csv(csvPath, indices, true, solver_);
        FAIL() << "Expected NamesNotFoundException";
    }
    catch (const SkeletonCoefficientReader::NamesNotFoundException& e)
    {
        const std::string msg(e.what());
        EXPECT_NE(msg.find("2 column(s) not found"), std::string::npos);
    }
}

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_ColumnException_MessageContainsCsvPath)
{
    auto csvPath = tmpDir_ / "cols.csv";
    writeFile(csvPath, "unknown_col");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    try
    {
        reader.read_indices_csv(csvPath, indices, true, solver_);
        FAIL() << "Expected NamesNotFoundException";
    }
    catch (const SkeletonCoefficientReader::NamesNotFoundException& e)
    {
        const std::string msg(e.what());
        EXPECT_NE(msg.find(csvPath.string()), std::string::npos);
    }
}

// ---------- Row mode (is_col = false) ----------

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_NoThrow_WhenAllRowsFound)
{
    auto csvPath = tmpDir_ / "rows.csv";
    writeFile(csvPath, "row1,row2");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    ASSERT_NO_THROW(reader.read_indices_csv(csvPath, indices, false, solver_));
    EXPECT_EQ(indices, (std::vector<int>{0, 1}));
}

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_Throws_WhenOneRowNotFound)
{
    auto csvPath = tmpDir_ / "rows.csv";
    writeFile(csvPath, "row1,unknown_row");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    EXPECT_THROW(reader.read_indices_csv(csvPath, indices, false, solver_),
                 SkeletonCoefficientReader::NamesNotFoundException);
}

TEST_F(SkeletonCoefficientReaderTest, ReadIndicesCsv_RowException_MessageContainsCount)
{
    auto csvPath = tmpDir_ / "rows.csv";
    writeFile(csvPath, "unknown_row1,unknown_row2");

    SkeletonCoefficientReader reader({});
    std::vector<int> indices;
    try
    {
        reader.read_indices_csv(csvPath, indices, false, solver_);
        FAIL() << "Expected NamesNotFoundException";
    }
    catch (const SkeletonCoefficientReader::NamesNotFoundException& e)
    {
        const std::string msg(e.what());
        EXPECT_NE(msg.find("2 row(s) not found"), std::string::npos);
    }
}
