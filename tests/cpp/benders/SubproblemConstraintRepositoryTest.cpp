#include <memory>

#include <gtest/gtest.h>

#include "LoggerStub.h"
#include "RecordingSolver.h"
#include "antares-xpansion/benders/benders_core/SubproblemConstraintRepository.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

class SubproblemConstraintRepositoryTest: public ::testing::Test
{
protected:
    std::shared_ptr<SubproblemWorker> make_worker(int initial_nrows)
    {
        worker_solver_->nrows = initial_nrows;
        VariableMap variable_map;
        return std::make_shared<SubproblemWorker>(variable_map, 1.0, worker_solver_, logger_);
    }

    std::shared_ptr<RecordingSolver> worker_solver_ = std::make_shared<RecordingSolver>();
    std::shared_ptr<RecordingSolver> reader_solver_ = std::make_shared<RecordingSolver>();
    Logger logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
};

TEST_F(SubproblemConstraintRepositoryTest, AppendConstraintReturnsRowFetchedFromReader)
{
    reader_solver_->row_mclind = {0, 1};
    reader_solver_->row_dmatval = {3.0, 4.0};
    reader_solver_->row_rhs = 7.0;

    auto worker = make_worker(/*initial_nrows=*/2);
    auto manager = SubproblemConstraintRepository::FromSharedSolver(reader_solver_, worker);

    std::string row_name = "extra_constraint";
    auto row = manager->AppendConstraint(row_name);

    EXPECT_EQ(row.row_names, std::vector<std::string>({"extra_constraint"}));
    EXPECT_EQ(row.mclind, std::vector<int>({0, 1}));
    EXPECT_EQ(row.dmatval, std::vector<double>({3.0, 4.0}));
    EXPECT_EQ(row.rhs, std::vector<double>({7.0}));
    EXPECT_EQ(worker_solver_->add_rows_calls, 1);
}

TEST_F(SubproblemConstraintRepositoryTest, RemoveAppendedConstraintsRollsBackToSizeCapturedAtConstruction)
{
    auto worker = make_worker(/*initial_nrows=*/5);
    auto manager = SubproblemConstraintRepository::FromSharedSolver(reader_solver_, worker);

    manager->RemoveAppendedConstraints();

    EXPECT_EQ(worker_solver_->del_rows_calls, 1);
    EXPECT_EQ(worker_solver_->del_rows_first, 5);
}
