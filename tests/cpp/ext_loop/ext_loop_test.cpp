
#include "OuterLoopCriterion.h"
#include "gtest/gtest.h"

class OuterLoopCriterionTest : public ::testing::Test {};

TEST_F(OuterLoopCriterionTest, IsCriterionHigh) {
  double threshold = 1.4;
  double epsilon = 1e-1;
  double max_unsup_energy = 0.1;
  const ExternalLoopOptions options = {threshold, epsilon, max_unsup_energy};
  PlainData::Variables variables = {
      {"PositiveUnsuppliedEnergy::1", "PositiveUnsuppliedEnergy::2", "var3"},
      {0.2, 0.3, 68}};
  double criterion_value = 2.0;  // two vars named PositiveUnsuppliedEnergy with
                                 // value > max_unsup_energy

  PlainData::SubProblemData subProblemData;
  subProblemData.variables = variables;
  SubProblemDataMap cut_trace = {
      std::make_pair(std::string("P1"), subProblemData)};

  WorkerMasterData worker_master_data;
  worker_master_data._cut_trace = cut_trace;

  OuterloopCriterionLossOfLoad criterion(options);

  EXPECT_EQ(criterion.IsCriterionSatisfied(worker_master_data),
            CRITERION::HIGH);
  EXPECT_EQ(criterion.CriterionValue(), criterion_value);
}

TEST_F(OuterLoopCriterionTest, IsCriterionLow) {
  double threshold = 4;
  double epsilon = 1e-1;
  double max_unsup_energy = 0.1;
  const ExternalLoopOptions options = {threshold, epsilon, max_unsup_energy};
  PlainData::Variables variables = {
      {"PositiveUnsuppliedEnergy::1", "PositiveUnsuppliedEnergy::2", "var3"},
      {0.2, 0.3, 68}};
  double criterion_value = 2.0;  // two vars named PositiveUnsuppliedEnergy with
                                 // value > max_unsup_energy

  PlainData::SubProblemData subProblemData;
  subProblemData.variables = variables;
  SubProblemDataMap cut_trace = {
      std::make_pair(std::string("P1"), subProblemData)};

  WorkerMasterData worker_master_data;
  worker_master_data._cut_trace = cut_trace;

  OuterloopCriterionLossOfLoad criterion(options);

  EXPECT_EQ(criterion.IsCriterionSatisfied(worker_master_data), CRITERION::LOW);
  EXPECT_EQ(criterion.CriterionValue(), criterion_value);
}

TEST_F(OuterLoopCriterionTest, IsMet) {
  double threshold = 2.0;
  double epsilon = 1e-1;
  double max_unsup_energy = 0.1;
  const ExternalLoopOptions options = {threshold, epsilon, max_unsup_energy};
  PlainData::Variables variables = {
      {"PositiveUnsuppliedEnergy::1", "PositiveUnsuppliedEnergy::2", "var3"},
      {0.2, 0.3, 68}};
  double criterion_value = 2.0;  // two vars named PositiveUnsuppliedEnergy with
                                 // value > max_unsup_energy

  PlainData::SubProblemData subProblemData;
  subProblemData.variables = variables;
  SubProblemDataMap cut_trace = {
      std::make_pair(std::string("P1"), subProblemData)};

  WorkerMasterData worker_master_data;
  worker_master_data._cut_trace = cut_trace;

  OuterloopCriterionLossOfLoad criterion(options);

  EXPECT_EQ(criterion.IsCriterionSatisfied(worker_master_data),
            CRITERION::IS_MET);
  EXPECT_EQ(criterion.CriterionValue(), criterion_value);
}