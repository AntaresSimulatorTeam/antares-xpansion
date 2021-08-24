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

TEST_F(MasterProblemBuilderTest, test_one_candidate_continue)
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
    ASSERT_EQ(master_problem->get_col_names(0, master_problem->get_ncols()-1)[0], cand1.name);

    std::vector<char> colTypeArray(master_problem->get_ncols());
    master_problem->get_col_type(colTypeArray.data(), 0, master_problem->get_ncols() - 1);

    ASSERT_EQ(colTypeArray[0], 'C');

    std::vector<double> varLbArray(master_problem->get_ncols());
    master_problem->get_lb(varLbArray.data(), 0, master_problem->get_ncols() - 1);

    ASSERT_EQ(varLbArray[0], 0);

    std::vector<double> varUbArray(master_problem->get_ncols());
    master_problem->get_ub(varUbArray.data(), 0, master_problem->get_ncols() - 1);

    ASSERT_EQ(varUbArray[0], 0);

}

TEST_F(MasterProblemBuilderTest, test_one_candidate_integer)
{
    std::string solver_name = "CBC";

    CandidateData cand1;
    cand1.link_id = 1;
    cand1.name = "transmission_line_1";
    cand1.link_name = "area1 - area2";
    cand1.already_installed_capacity = 0;
    cand1.unit_size = 4;
    cand1.max_units = 17;

    std::vector<CandidateData> cand_data_list = { cand1 };

    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    auto master_problem = MasterProblemBuilder().build(solver_name, links);
    ASSERT_EQ(master_problem->get_ncols(), 2);

    std::vector<char> colTypeArray(master_problem->get_ncols());
    master_problem->get_col_type(colTypeArray.data(), 0, master_problem->get_ncols() - 1);

    ASSERT_EQ(colTypeArray[1], 'I');

    std::vector<double> varLbArray(master_problem->get_ncols());
    master_problem->get_lb(varLbArray.data(), 0, master_problem->get_ncols() - 1);

    ASSERT_EQ(varLbArray[1], 0);

    std::vector<double> varUbArray(master_problem->get_ncols());
    master_problem->get_ub(varUbArray.data(), 0, master_problem->get_ncols() - 1);

    ASSERT_EQ(varUbArray[1], 17);

}