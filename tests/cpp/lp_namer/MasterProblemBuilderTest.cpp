//
// Created by s90365 on 23/08/2021.
//
#include "MasterProblemBuilder.h"
#include <solver_utils.h>
#include "gtest/gtest.h"

class MasterProblemBuilderTest : public ::testing::Test
{
public:
    SolverAbstract::Ptr math_problem;

protected:
    void SetUp()
    {
        
    }
};

TEST_F(MasterProblemBuilderTest, test_init)
{
    std::string solver_name = "CBC";

    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link_name = "area1 - area2";
    cand1.already_installed_capacity = 0;

    std::vector<CandidateData> cand_data_list = { cand1 };

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    auto master_problem = MasterProblemBuilder().build(solver_name, links);
    ASSERT_EQ(master_problem->get_ncols(), 1);
}