#include <gtest/gtest.h>

#include "antares-xpansion/benders/benders_core/CutsManagement.h"
#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"

using namespace Outerloop;

class CutsManagerRunTimeTest : public ::testing::Test
{
protected:
    CutsManagerRunTime cuts_manager_;

    WorkerMasterDataVect CreateSampleCuts()
    {
        WorkerMasterDataVect cuts;

        // Create first cut
        WorkerMasterData cut1;
        cut1._valid = true;
        cut1._lb = 1400.0;
        cut1._ub = 2000.0;
        cut1._best_ub = 2000.0;
        cut1._master_duration = 1.5;
        cut1._subproblem_duration = 2.5;
        cut1._invest_cost = 1000.0;
        cut1._operational_cost = 500.0;
        cut1._x_cut = std::make_shared<Point>();
        (*cut1._x_cut)["var1"] = 1.0;
        (*cut1._x_cut)["var2"] = 2.0;
        (*cut1._x_cut)["var3"] = 3.0;

        // Create second cut
        WorkerMasterData cut2;
        cut2._valid = true;
        cut2._lb = 1300.0;
        cut2._ub = 1800.0;
        cut2._best_ub = 1800.0;
        cut2._master_duration = 1.2;
        cut2._subproblem_duration = 2.0;
        cut2._invest_cost = 900.0;
        cut2._operational_cost = 450.0;
        cut2._x_cut = std::make_shared<Point>();
        (*cut2._x_cut)["var1"] = 1.5;
        (*cut2._x_cut)["var2"] = 2.5;
        (*cut2._x_cut)["var3"] = 3.5;

        cuts.push_back(cut1);
        cuts.push_back(cut2);

        return cuts;
    }
};

TEST_F(CutsManagerRunTimeTest, SaveAndLoadEmpty)
{
    // Save empty cuts
    WorkerMasterDataVect empty_cuts;
    cuts_manager_.Save(empty_cuts);

    // Load and verify
    WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
    EXPECT_TRUE(loaded_cuts.empty());
}

TEST_F(CutsManagerRunTimeTest, SaveAndLoadSingleCut)
{
    // Create and save a single cut
    WorkerMasterDataVect cuts = CreateSampleCuts();
    WorkerMasterDataVect single_cut = {cuts[0]};

    cuts_manager_.Save(single_cut);

    // Load and verify
    WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
    ASSERT_EQ(loaded_cuts.size(), 1);

    const auto& original = single_cut[0];
    const auto& loaded = loaded_cuts[0];

    EXPECT_EQ(loaded._valid, original._valid);
    EXPECT_DOUBLE_EQ(loaded._lb, original._lb);
    EXPECT_DOUBLE_EQ(loaded._ub, original._ub);
    EXPECT_DOUBLE_EQ(loaded._best_ub, original._best_ub);
    EXPECT_DOUBLE_EQ(loaded._master_duration, original._master_duration);
    EXPECT_DOUBLE_EQ(loaded._subproblem_duration, original._subproblem_duration);
    EXPECT_DOUBLE_EQ(loaded._invest_cost, original._invest_cost);
    EXPECT_DOUBLE_EQ(loaded._operational_cost, original._operational_cost);

    // Check x_cut values
    auto original_x_cut = original.get_x_cut();
    auto loaded_x_cut = loaded.get_x_cut();
    EXPECT_EQ(loaded_x_cut, original_x_cut);
}

TEST_F(CutsManagerRunTimeTest, SaveAndLoadMultipleCuts)
{
    // Create and save multiple cuts
    WorkerMasterDataVect cuts = CreateSampleCuts();
    cuts_manager_.Save(cuts);

    // Load and verify
    WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
    ASSERT_EQ(loaded_cuts.size(), cuts.size());

    for (size_t i = 0; i < cuts.size(); ++i)
    {
        const auto& original = cuts[i];
        const auto& loaded = loaded_cuts[i];

        EXPECT_DOUBLE_EQ(loaded._invest_cost, original._invest_cost);
        EXPECT_DOUBLE_EQ(loaded._operational_cost, original._operational_cost);
        EXPECT_EQ(loaded.get_x_cut(), original.get_x_cut());
    }
}

TEST_F(CutsManagerRunTimeTest, OverwritePreviousSave)
{
    // Save first set of cuts
    WorkerMasterDataVect cuts1 = CreateSampleCuts();
    cuts_manager_.Save(cuts1);

    // Save second set of cuts (should overwrite)
    WorkerMasterDataVect cuts2;
    WorkerMasterData cut3;
    cut3._valid = true;
    cut3._invest_cost = 1200.0;
    cut3._operational_cost = 600.0;
    cut3._x_cut = std::make_shared<Point>();
    (*cut3._x_cut)["var1"] = 4.0;
    (*cut3._x_cut)["var2"] = 5.0;
    (*cut3._x_cut)["var3"] = 6.0;
    cuts2.push_back(cut3);

    cuts_manager_.Save(cuts2);

    // Load and verify we get the second set
    WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
    ASSERT_EQ(loaded_cuts.size(), 1);
    EXPECT_DOUBLE_EQ(loaded_cuts[0]._invest_cost, 1200.0);
    EXPECT_DOUBLE_EQ(loaded_cuts[0]._operational_cost, 600.0);
    auto x_cut = loaded_cuts[0].get_x_cut();
    EXPECT_DOUBLE_EQ(x_cut["var1"], 4.0);
    EXPECT_DOUBLE_EQ(x_cut["var2"], 5.0);
    EXPECT_DOUBLE_EQ(x_cut["var3"], 6.0);
}

TEST_F(CutsManagerRunTimeTest, LoadWithoutSave)
{
    // Load without saving first
    WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
    EXPECT_TRUE(loaded_cuts.empty());
}

TEST_F(CutsManagerRunTimeTest, SaveAndLoadWithLargeXCut)
{
    // Create cut with large x_cut map
    WorkerMasterDataVect cuts;
    WorkerMasterData cut;
    cut._valid = true;
    cut._invest_cost = 5000.0;
    cut._x_cut = std::make_shared<Point>();
    for (int i = 0; i < 100; ++i)
    {
        (*cut._x_cut)["var_" + std::to_string(i)] = static_cast<double>(i);
    }
    cuts.push_back(cut);

    cuts_manager_.Save(cuts);

    // Load and verify
    WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
    ASSERT_EQ(loaded_cuts.size(), 1);
    auto x_cut = loaded_cuts[0].get_x_cut();
    EXPECT_EQ(x_cut.size(), 100);
    EXPECT_DOUBLE_EQ(loaded_cuts[0]._invest_cost, 5000.0);

    for (int i = 0; i < 100; ++i)
    {
        std::string key = "var_" + std::to_string(i);
        EXPECT_DOUBLE_EQ(x_cut[key], static_cast<double>(i));
    }
}

TEST_F(CutsManagerRunTimeTest, MultipleSaveAndLoadCycles)
{
    // Test multiple save/load cycles
    for (int cycle = 0; cycle < 5; ++cycle)
    {
        WorkerMasterDataVect cuts = CreateSampleCuts();
        cuts_manager_.Save(cuts);

        WorkerMasterDataVect loaded_cuts = cuts_manager_.Load();
        ASSERT_EQ(loaded_cuts.size(), 2);
    }
}
