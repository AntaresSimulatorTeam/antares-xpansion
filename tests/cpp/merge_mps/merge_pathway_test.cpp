#include <fmt/format.h>
#include <gtest/gtest.h>

#include "InMemoryWriter.h"
#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/merge_mps/MergeMPS.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"

using namespace std::string_literals;

class MergePathwayTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        tmp_dir_ = CreateRandomSubDir(std::filesystem::temp_directory_path());
        previous_path = std::filesystem::current_path();
        std::filesystem::current_path(tmp_dir_);
        logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer_ = std::make_shared<Xpansion::Test::InMemoryWriter>();

        options_.SOLVER_NAME = "COIN";
        options_.STRUCTURE_FILE = (tmp_dir_ / "structure_file.txt").string();
        options_.OUTPUTROOT = tmp_dir_.string();
    }

    void TearDown() override
    {
        std::filesystem::current_path(previous_path);
    }

    std::filesystem::path previous_path;
    std::filesystem::path tmp_dir_;
    MergeMPSOptions options_;
    std::shared_ptr<Xpansion::Test::LoggerNOOPStub> logger_;
    std::shared_ptr<Xpansion::Test::InMemoryWriter> writer_;
};

TEST_F(MergePathwayTest, empty_tree_ko)
{
    std::ofstream structure_file(tmp_dir_ / "structure_file.txt"s);
    structure_file.close();

    std::ofstream tree(tmp_dir_ / "empty_tree.mps"s);
    tree << "{}";
    tree.close();

    EXPECT_DEATH(
      { MergePathwayMPS mergeMPS(options_, logger_, writer_, tmp_dir_ / "empty_tree.mps"s); },
      "Tree is empty");
}
