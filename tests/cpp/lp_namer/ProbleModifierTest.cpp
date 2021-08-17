
#include <ProblemModifier.h>
#include <ActiveLinks.h>
#include <multisolver_interface/SolverFactory.h>
#include <fstream>
#include <solver_utils.h>
#include "gtest/gtest.h"


class ProblemModifierTest : public ::testing::Test
{
public:
    const std::string mps_name = "dummy.mps";
    SolverAbstract::Ptr math_problem;
    int n_cols=-1;
    int n_rows=-1;
    int n_elems=-1;
    std::vector<char> coltypes;
    std::vector<char> rowtypes;
    std::vector<double> objectives;
    std::vector<double> upper_bounds;
    std::vector<double> lower_bounds;
    std::vector<double> rhs;
    std::vector<double> mat_val;
    std::vector<std::basic_string<char>> col_names;


protected:
    void SetUp()
    {

        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        std::vector<double> objectives(1, 1);
        std::vector<double> lb(1, -1000);
        std::vector<double> ub(1, 1000);
        std::vector<char> coltypes(1, 'C');
        std::vector<int> mstart(1, 0);
        std::vector<std::string> candidates_colnames(1, "C0000000");
        solver_addcols(math_problem, objectives, mstart, {}, {}, lb, ub, coltypes, candidates_colnames);
    }

    void TearDown()
    {
        n_cols=-1;
        n_rows=-1;
        n_elems=-1;

    }

    void update_n_cols(){
        if(n_cols==-1){
            n_cols = math_problem->get_ncols();
        }
    }
    void update_n_rows(){
        if(n_rows==-1){
            n_rows = math_problem->get_nrows();
        }
    }
    void update_n_elems(){
        if(n_elems==-1){
            n_elems = math_problem->get_nelems();
        }
    }
    void update_col_names(){
        update_n_cols();
        col_names = math_problem->get_col_names(0, n_cols-1);
    }
    void update_col_type(){
        update_n_cols();
        if(coltypes.size()!= n_cols){
            std::vector<char> buffer(n_cols, '0');
            coltypes.insert(coltypes.begin(), buffer.begin(), buffer.end());
            math_problem->get_col_type(coltypes.data(), 0, n_cols-1);
        }
    }
    void update_row_type(){
        update_n_rows();
        if(rowtypes.size()!= n_rows){
            std::vector<char> buffer(n_rows, '0');
            rowtypes.insert(rowtypes.begin(), buffer.begin(), buffer.end());
            math_problem->get_row_type(rowtypes.data(), 0, n_rows-1);
        }
    }
    void update_objectives(){
        update_n_cols();
        if(objectives.size()!=n_cols){
            std::vector<double> buffer(n_cols, -777);
            objectives.insert(objectives.begin(), buffer.begin(), buffer.end());
            math_problem->get_obj(objectives.data(), 0, n_cols-1);
        }
    }
    void update_lower_bounds(){
        update_n_cols();
        if(lower_bounds.size()!=n_cols){
            std::vector<double> buffer(n_cols, -777);
            lower_bounds.insert(lower_bounds.begin(), buffer.begin(), buffer.end());
            math_problem->get_lb(lower_bounds.data(), 0, n_cols-1);
        }
    }
    void update_upper_bounds(){
        update_n_cols();
        if(upper_bounds.size()!=n_cols){
            std::vector<double> buffer(n_cols, -777);
            upper_bounds.insert(upper_bounds.begin(), buffer.begin(), buffer.end());
            math_problem->get_ub(upper_bounds.data(), 0, n_cols-1);
        }
    }
    void update_mat_val(){
        update_n_rows();
        update_n_elems();

        if (mat_val.size() != n_elems){
            std::vector<double> buffer(n_elems, -777);
            mat_val.insert(mat_val.begin(), buffer.begin(), buffer.end());

            std::vector<int>	mstart(n_rows + 1);
            std::vector<int>	mind(n_elems);
            int n_returned(0);
            math_problem->get_rows(mstart.data(), mind.data(), mat_val.data(),
                                   n_elems, &n_returned, 0, n_rows - 1);
        }
    }
    void update_rhs_val(){
        update_n_rows();
        if (rhs.size() != n_rows){
            std::vector<double> buffer(n_rows, -777);
            rhs.insert(rhs.begin(), buffer.begin(), buffer.end());
            math_problem->get_rhs(rhs.data(),0, n_rows - 1);
        }
    }
    void verify_columns_are(const int expected_n_cols){
        update_n_cols();
        ASSERT_EQ(n_cols, expected_n_cols);
    }
    void verify_rows_are(const int expected_n_rows){
        update_n_rows();
        ASSERT_EQ(n_rows, expected_n_rows);
    }
    void verify_column_name_is(const int col_id, std::basic_string<char> name){
        update_col_names();
        ASSERT_EQ(col_names.at(col_id), name);
    }
    void verify_column_is_of_type(const int col_id, char type){
        update_col_type();
        ASSERT_EQ(coltypes.at(col_id), type);
    }
    void verify_column_objective_is(const int col_id, double obj_value){
        update_objectives();
        ASSERT_DOUBLE_EQ(objectives.at(col_id), obj_value);
    }
    void verify_column_lower_bound_is(const int col_id, double lower_value){
        update_lower_bounds();
        ASSERT_DOUBLE_EQ(lower_bounds.at(col_id), lower_value);
    }
    void verify_column_upper_bound_is(const int col_id, double upper_value){
        update_upper_bounds();
        ASSERT_DOUBLE_EQ(upper_bounds.at(col_id), upper_value);
    }
    void verify_elems_are(const int expected_n_elems){
        update_n_elems();
        ASSERT_EQ(n_elems, expected_n_elems);
    }
    void verify_row_is_of_type(const int row_id, char type){
        update_row_type();
        ASSERT_EQ(rowtypes.at(row_id), type);
    }
    void verify_mat_val_is(const int mat_id, double mat_value){
        update_mat_val();
        ASSERT_DOUBLE_EQ(mat_val.at(mat_id), mat_value);
    }
    void verify_rhs_is(const int rhs_id, double rhs_value){
        update_rhs_val();
        ASSERT_DOUBLE_EQ(rhs.at(rhs_id), rhs_value);
    }
};

TEST_F(ProblemModifierTest, empty_test_the_multisolver_interface)
{
    verify_columns_are(1);
    verify_rows_are(0);

    verify_column_name_is(0, "C0000000");
    verify_column_is_of_type(0, 'C');
    verify_column_objective_is(0, 1);
    verify_column_lower_bound_is(0, -1000);
    verify_column_upper_bound_is(0, 1000);
}


TEST_F(ProblemModifierTest, One_link_no_candidates_link_boundaries_are_removed) {
    const int link_id = 0;
    const ColumnToChange column = {0, 0};
    const std::map<linkId , ColumnsToChange> p_var_columns = {{link_id,{column}}};
    const std::vector<ActiveLink> active_links= {ActiveLink(link_id, "dummy_link")};

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), active_links, p_var_columns);

    verify_columns_are(1);

    verify_column_name_is(0, "C0000000");
    verify_column_is_of_type(0, 'C');
    verify_column_objective_is(0, 1);
    verify_column_lower_bound_is(0, -1e20);
    verify_column_upper_bound_is(0, 1e20);
}


TEST_F(ProblemModifierTest, One_link_two_candidates) {
    const int link_id = 0;
    const ColumnsToChange columns = {{0, 0}};
    const std::map<linkId , ColumnsToChange> p_var_columns = {{link_id,columns}};

    CandidateData cand1;
    cand1.link_id = link_id;
    cand1.name = "candy1";
    cand1.link_name = "dummy_link";
    cand1.already_installed_capacity = 1000.0;
    CandidateData cand2;
    cand2.link_id = link_id;
    cand2.name = "candy2";
    cand2.link_name = "dummy_link";
    cand2.already_installed_capacity = 1000.0;

    std::vector<CandidateData> cand_data_list = { cand1, cand2 };
    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), links, p_var_columns);

    verify_columns_are(3);

    verify_column_name_is(0, "C0000000");
    verify_column_is_of_type(0, 'C');
    verify_column_objective_is(0, 1);
    verify_column_lower_bound_is(0, -1e20);
    verify_column_upper_bound_is(0, 1e20);

    verify_column_name_is(1, "candy1");
    verify_column_is_of_type(1, 'C');
    verify_column_objective_is(1, 0);
    verify_column_lower_bound_is(1, -1e20);
    verify_column_upper_bound_is(1, 1e20);

    verify_column_name_is(2, "candy2");
    verify_column_is_of_type(2, 'C');
    verify_column_objective_is(2, 0);
    verify_column_lower_bound_is(2, -1e20);
    verify_column_upper_bound_is(2, 1e20);

    verify_rows_are(2);
    verify_row_is_of_type(0, 'L');
    verify_row_is_of_type(1, 'G');

    verify_elems_are(6);
    verify_mat_val_is(0, 1);
    verify_mat_val_is(1, -links.at(0).getCandidates().at(0).direct_profile(0));
    verify_mat_val_is(2, -links.at(0).getCandidates().at(1).direct_profile(0));
    verify_mat_val_is(3, 1);
    verify_mat_val_is(4, links.at(0).getCandidates().at(0).indirect_profile(0));
    verify_mat_val_is(5, links.at(0).getCandidates().at(1).indirect_profile(0));

    verify_rhs_is(0, links.at(0)._already_installed_capacity * links.at(0).already_installed_direct_profile(0));
    verify_rhs_is(1, -links.at(0)._already_installed_capacity* links.at(0).already_installed_indirect_profile(0));
}

TEST_F(ProblemModifierTest, One_link_two_candidates_two_timestep) {
    const int link_id = 0;
    const ColumnsToChange columns = {{0, 0}, {0, 1}};
    const std::map<linkId , ColumnsToChange> p_var_columns = {{link_id,columns}};

    CandidateData cand1;
    cand1.link_id = link_id;
    cand1.name = "candy1";
    cand1.link_name = "dummy_link";
    cand1.already_installed_capacity = 1000.0;
    CandidateData cand2;
    cand2.link_id = link_id;
    cand2.name = "candy2";
    cand2.link_name = "dummy_link";
    cand2.already_installed_capacity = 1000.0;

    std::vector<CandidateData> cand_data_list = { cand1, cand2 };
    std::map<std::string, LinkProfile> profile_map;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), links, p_var_columns);

    verify_columns_are(3);
    verify_rows_are(4);
    verify_row_is_of_type(0, 'L');
    verify_row_is_of_type(1, 'G');
    verify_row_is_of_type(2, 'L');
    verify_row_is_of_type(3, 'G');

    verify_elems_are(12);

    verify_mat_val_is(0, 1);
    verify_mat_val_is(1, -links.at(0).getCandidates().at(0).direct_profile(0));
    verify_mat_val_is(2, -links.at(0).getCandidates().at(1).direct_profile(0));
    verify_mat_val_is(3, 1);
    verify_mat_val_is(4, links.at(0).getCandidates().at(0).indirect_profile(0));
    verify_mat_val_is(5, links.at(0).getCandidates().at(1).indirect_profile(0));
    verify_mat_val_is(6, 1);
    verify_mat_val_is(7, -links.at(0).getCandidates().at(0).direct_profile(1));
    verify_mat_val_is(8, -links.at(0).getCandidates().at(1).direct_profile(1));
    verify_mat_val_is(9, 1);
    verify_mat_val_is(10, links.at(0).getCandidates().at(0).indirect_profile(1));
    verify_mat_val_is(11, links.at(0).getCandidates().at(1).indirect_profile(1));

    verify_rhs_is(0, links.at(0)._already_installed_capacity * links.at(0).already_installed_direct_profile(0));
    verify_rhs_is(1, -links.at(0)._already_installed_capacity* links.at(0).already_installed_indirect_profile(0));
    verify_rhs_is(2, links.at(0)._already_installed_capacity * links.at(0).already_installed_direct_profile(1));
    verify_rhs_is(3, -links.at(0)._already_installed_capacity* links.at(0).already_installed_indirect_profile(1));
}

TEST_F(ProblemModifierTest, One_link_two_candidates_two_timestep_profile) {
    const int link_id = 0;
    const ColumnsToChange columns = {{0, 0}, {0, 1}};
    const std::map<linkId , ColumnsToChange> p_var_columns = {{link_id,columns}};

    CandidateData cand1;
    cand1.link_id = link_id;
    cand1.name = "candy1";
    cand1.link_name = "dummy_link";
    cand1.already_installed_capacity = 2000.0;
    cand1.installed_link_profile_name = "install_link_profile";
    cand1.link_profile = "profile1";
    CandidateData cand2;
    cand2.link_id = link_id;
    cand2.name = "candy2";
    cand2.link_name = "dummy_link";
    cand2.already_installed_capacity = 2000.0;
    cand2.installed_link_profile_name = "install_link_profile";
    cand2.link_profile = "profile2";

    std::vector<CandidateData> cand_data_list = { cand1, cand2 };
    std::map<std::string, LinkProfile> profile_map;
    LinkProfile install_link_profile;
    install_link_profile._directLinkProfile = {1,2};
    install_link_profile._indirectLinkProfile = {3,4};
    profile_map["install_link_profile"]=install_link_profile;
    LinkProfile profile1;
    profile1._directLinkProfile = {0.5,1};
    profile1._indirectLinkProfile = {0.8,1.2};
    profile_map["profile1"]=profile1;
    LinkProfile profile2;
    profile2._directLinkProfile = {1.5,1.7};
    profile2._indirectLinkProfile = {2.6,2.8};
    profile_map["profile2"]=profile2;

    ActiveLinksBuilder linkBuilder{ cand_data_list, profile_map };
    const std::vector<ActiveLink>& links = linkBuilder.getLinks();

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), links, p_var_columns);

    verify_columns_are(3);
    verify_rows_are(4);
    verify_row_is_of_type(0, 'L');
    verify_row_is_of_type(1, 'G');
    verify_row_is_of_type(2, 'L');
    verify_row_is_of_type(3, 'G');

    verify_elems_are(12);

    verify_mat_val_is(0, 1);
    verify_mat_val_is(1, -links.at(0).getCandidates().at(0).direct_profile(0));
    verify_mat_val_is(2, -links.at(0).getCandidates().at(1).direct_profile(0));
    verify_mat_val_is(3, 1);
    verify_mat_val_is(4, links.at(0).getCandidates().at(0).indirect_profile(0));
    verify_mat_val_is(5, links.at(0).getCandidates().at(1).indirect_profile(0));
    verify_mat_val_is(6, 1);
    verify_mat_val_is(7, -links.at(0).getCandidates().at(0).direct_profile(1));
    verify_mat_val_is(8, -links.at(0).getCandidates().at(1).direct_profile(1));
    verify_mat_val_is(9, 1);
    verify_mat_val_is(10, links.at(0).getCandidates().at(0).indirect_profile(1));
    verify_mat_val_is(11, links.at(0).getCandidates().at(1).indirect_profile(1));

    verify_rhs_is(0, links.at(0)._already_installed_capacity * links.at(0).already_installed_direct_profile(0));
    verify_rhs_is(1, -links.at(0)._already_installed_capacity* links.at(0).already_installed_indirect_profile(0));
    verify_rhs_is(2, links.at(0)._already_installed_capacity * links.at(0).already_installed_direct_profile(1));
    verify_rhs_is(3, -links.at(0)._already_installed_capacity* links.at(0).already_installed_indirect_profile(1));
}


