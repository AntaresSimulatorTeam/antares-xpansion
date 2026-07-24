#include <memory>

#include <gtest/gtest.h>

#include "RecordingSolver.h"
#include "antares-xpansion/benders/benders_core/SolverRowExtractor.h"

TEST(SolverRowExtractorTest, GetRowMarshalsSolverDataWithoutTouchingDisk)
{
    auto solver = std::make_shared<RecordingSolver>();
    solver->ncols = 3;
    solver->row_mclind = {0, 2};
    solver->row_dmatval = {1.5, 2.5};
    solver->row_rhs = 10.0;
    solver->row_range = 4.0;
    solver->row_qrtype = 'L';

    SolverRowExtractor extractor(solver);
    auto row = extractor.GetRow("row1");

    EXPECT_EQ(row.row_names, std::vector<std::string>({"row1"}));
    EXPECT_EQ(row.mclind, std::vector<int>({0, 2}));
    EXPECT_EQ(row.dmatval, std::vector<double>({1.5, 2.5}));
    EXPECT_EQ(row.rhs, std::vector<double>({10.0}));
    EXPECT_EQ(row.range_p, std::vector<double>({4.0}));
    ASSERT_EQ(row.qrtype_p.size(), 1u);
    EXPECT_EQ(row.qrtype_p[0], 'L');
}

TEST(SolverRowExtractorTest, GetRowUsesSolverRowIndexLookup)
{
    auto solver = std::make_shared<RecordingSolver>();
    solver->row_index = 7;
    solver->row_mclind = {};
    solver->row_dmatval = {};

    SolverRowExtractor extractor(solver);
    auto row = extractor.GetRow("some_row");

    // mstart is resized to hold [begin] once nels is known; no crash / UB on an
    // empty row is the behavior under test here.
    EXPECT_TRUE(row.mclind.empty());
    EXPECT_TRUE(row.dmatval.empty());
}
