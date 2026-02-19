#pragma once

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "InMemoryWriter.h"
#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/merge_mps/MergeMPS.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"

using namespace std::string_literals;

class MergeMPSTest: public ::testing::Test
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

    void createMasterProblem()
    {
        std::ofstream master_problem(tmp_dir_ / "master.mps"s);
        master_problem << R"(NAME          MASTER  FREE
ROWS
 N  OBJROW
 L  C1
 G  C2
COLUMNS
    X1        OBJROW    3.0
    X1        C1        2.0
    X1        C2        1.0
    X2        OBJROW    4.0
    X2        C1        1.0
    X2        C2        2.0
RHS
    RHS      C1        8.0
    RHS      C2        3.0
BOUNDS
 UP BOUND      X1        4.0
 UP BOUND      X2        4.0
ENDATA)";
        master_problem.close();
        options_.MASTER_NAME = "master.mps";
    }

    void createSatelliteProblem()
    {
        std::ofstream satellite_problem(tmp_dir_ / "satellite.mps"s);
        satellite_problem << R"(NAME       SATELLITE  FREE
ROWS
 N  OBJROW
 L  C1
 G  C2
COLUMNS
    Y1        OBJROW    1.0
    Y1        C1        3.0
    Y1        C2        9.0
    X1        C1        2.0
    X1        C2        3.0
RHS
    RHS      C1        7.0
    RHS      C2        5.0
BOUNDS
 UP BOUND      Y1        10.0
 UP BOUND      X1        10.0
ENDATA)";
        satellite_problem.close();
        // Required to avoid missing contribution to Obj
        options_.weights["satellite.mps"] = 1;
    }

    void createStructureFile(const std::vector<std::tuple<std::string, std::string, int>>& entries)
    {
        std::ofstream structure_file(options_.STRUCTURE_FILE);
        for (const auto& [mps, var, idx]: entries)
        {
            structure_file << fmt::format("{0} {1} {2}\n", mps, var, idx);
        }
        structure_file.close();
    }

    std::filesystem::path previous_path;
    std::filesystem::path tmp_dir_;
    MergeMPSOptions options_;
    std::shared_ptr<Xpansion::Test::LoggerNOOPStub> logger_;
    std::shared_ptr<Xpansion::Test::InMemoryWriter> writer_;
};

static inline auto get_rows(const SolverAbstract* solver)
{
    std::vector<int> mstart_merged(solver->get_nrows() + 1);
    std::vector<int> mclind_merged(solver->get_nelems());
    std::vector<double> dmatval_merged(solver->get_nelems());
    int merge_ret = 0;
    solver->get_rows(mstart_merged.data(),
                     mclind_merged.data(),
                     dmatval_merged.data(),
                     solver->get_nelems(),
                     &merge_ret,
                     0,
                     solver->get_nrows() - 1);
    return std::tuple{mstart_merged, mclind_merged, dmatval_merged, merge_ret};
}

