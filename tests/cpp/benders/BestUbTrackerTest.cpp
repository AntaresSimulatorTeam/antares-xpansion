#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/mpi.hpp>

#include "LoggerStub.h"
#include "antares-xpansion/benders/benders_core/BestUbTracker.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "gtest/gtest.h"

namespace mpi = boost::mpi;

boost::mpi::environment* penv = nullptr;
boost::mpi::communicator* pworld = nullptr;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    mpi::environment env(argc, argv);
    mpi::communicator world;
    penv = &env;
    pworld = &world;

    return RUN_ALL_TESTS();
}

using namespace Xpansion::Test;

class SubproblemWorkerTest : public SubproblemWorker
{
public:
    using SubproblemWorker::SubproblemWorker;

    void set_solution(std::vector<double> new_sol)
    {
        solution_ = new_sol ;
    }

    std::vector<double> get_solution() const override
    {
        return solution_ ;
    }

    int get_variable_index(const std::string& variable_name) override
    {
        return variable_indices_map_.at(variable_name) ;
    }

private:
    std::map<std::string, int> variable_indices_map_ = {
        {"variable_1", 0},
        {"variable_2", 1},
        {"variable_3", 2},
        {"variable_4", 3}
    };
    std::vector<double> solution_ ;
};

std::map<std::string, std::vector<double>> read_output_csv(const std::filesystem::path& file)
{
    std::map<std::string, std::vector<double>> result;
    std::ifstream in(file);
    std::string line;
    std::getline(in, line); // skip header
    while (std::getline(in, line))
    {
        std::istringstream ss(line);
        std::string token;
        std::string sub_name;
        std::vector<double> values;
        bool first = true;
        while (std::getline(ss, token, ','))
        {
            if (first) { sub_name = token; first = false; }
            else { values.push_back(std::stod(token)); }
        }
        result[sub_name] = values;
    }
    return result;
}

class BestUbTrackerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        tmpDir_ = std::filesystem::temp_directory_path()
                  / ("best_ub_tracker_test_"
                     + std::to_string(::testing::UnitTest::GetInstance()->random_seed()));
        std::filesystem::create_directories(tmpDir_);

        csvFile_ = tmpDir_ / "variable_to_follow.csv";
        {
            std::ofstream out(csvFile_);
            out << "variable_1,variable_2,variable_3,variable_4\n";
        }

        logger_ = std::make_shared<LoggerNOOPStub>();
        worker_1_ = std::make_shared<SubproblemWorkerTest>() ; 
        worker_2_ = std::make_shared<SubproblemWorkerTest>() ;

        best_ub_tracker_ = std::make_shared<BestUbTracker>(pworld,csvFile_,tmpDir_,logger_) ;

    }

    void TearDown() override
    {
        std::filesystem::remove_all(tmpDir_);
    }

    std::filesystem::path tmpDir_;
    std::filesystem::path csvFile_;
    Logger logger_;
    std::shared_ptr<SubproblemWorkerTest> worker_1_ ; 
    std::shared_ptr<SubproblemWorkerTest> worker_2_ ;
    std::shared_ptr<BestUbTracker>  best_ub_tracker_ ; 
};

TEST_F(BestUbTrackerTest, ConstructorWithValidFile_DoesNotThrow)
{
    EXPECT_NO_THROW(BestUbTracker(pworld, csvFile_, tmpDir_, logger_));
}

TEST_F(BestUbTrackerTest, ConstructorWithMissingFile_DoesNotThrow)
{
    auto missingFile = tmpDir_ / "nonexistent.csv";
    EXPECT_NO_THROW(BestUbTracker(pworld, missingFile, tmpDir_, logger_));
}

TEST_F(BestUbTrackerTest, SetAndDumpBehaviour) 
{
    //We simulate a firest resolution by setting the solution on the sub_1 and sub_2
    worker_1_->set_solution({1., 3., 5.,5.}) ; 
    best_ub_tracker_->set_variables_values("sub_1",worker_1_,1,10);
    
    worker_2_->set_solution({3., 3., 5.,5.}) ; 
    best_ub_tracker_->set_variables_values("sub_2",worker_2_,1,10);

    best_ub_tracker_->dump_values();

    auto result = read_output_csv(tmpDir_ / "sub_best_ub_variables.csv");

    EXPECT_EQ(result["sub_1"], (std::vector<double>{1., 3., 5., 5.}));
    EXPECT_EQ(result["sub_2"], (std::vector<double>{3., 3., 5., 5.}));

    // Second iteration with a bigger UB — values should not be updated
    worker_1_->set_solution({9., 8., 7., 6.}) ;
    best_ub_tracker_->set_variables_values("sub_1", worker_1_, 2, 20);

    worker_2_->set_solution({9., 8., 7., 6.}) ;
    best_ub_tracker_->set_variables_values("sub_2", worker_2_, 2, 20);

    best_ub_tracker_->dump_values();

    auto result2 = read_output_csv(tmpDir_ / "sub_best_ub_variables.csv");
    std::cout<<"resul2 size after the csv read "<<result2.size()<<std::endl ; 

    EXPECT_EQ(result2["sub_1"], (std::vector<double>{1., 3., 5., 5.}));
    EXPECT_EQ(result2["sub_2"], (std::vector<double>{3., 3., 5., 5.}));

    // Third iteration with a smaller UB — values should be updated
    worker_1_->set_solution({2., 4., 6., 8.}) ;
    best_ub_tracker_->set_variables_values("sub_1", worker_1_, 3, 5);

    worker_2_->set_solution({1., 2., 3., 4.}) ;
    best_ub_tracker_->set_variables_values("sub_2", worker_2_, 3, 5);

    best_ub_tracker_->dump_values();

    auto result3 = read_output_csv(tmpDir_ / "sub_best_ub_variables.csv");

    EXPECT_EQ(result3["sub_1"], (std::vector<double>{2., 4., 6., 8.}));
    EXPECT_EQ(result3["sub_2"], (std::vector<double>{1., 2., 3., 4.}));
}

