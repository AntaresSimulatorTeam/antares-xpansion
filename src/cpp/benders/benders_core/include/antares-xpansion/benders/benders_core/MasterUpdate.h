#pragma once
#include <memory>
#include <string>

#include "antares-xpansion/benders/outer_loop/IMasterUpdate.h"
#include "common.h"

class BendersMasterManager;

namespace Outerloop
{

class MasterUpdateBase: public IMasterUpdate
{
public:
    explicit MasterUpdateBase(std::shared_ptr<BendersMasterManager> master_manager,
                              double tau,
                              double outer_loop_stopping_threshold);
    explicit MasterUpdateBase(std::shared_ptr<BendersMasterManager> master_manager,
                              double tau,
                              double outer_loop_stopping_threshold,
                              const std::string& name);
    bool Update(double lambda_min, double lambda_max) override;
    [[nodiscard]] double Rhs() const override;

private:
    void CheckTau(double tau);
    void UpdateConstraints();
    void AddMinInvestConstraint();
    int additional_constraint_index_ = -1;
    std::shared_ptr<BendersMasterManager> master_manager_;
    double lambda_ = 0;
    // tau
    double dichotomy_weight_coeff_ = 0.5;
    double outer_loop_stopping_threshold_ = 1e-1;
    // rename min invest constraint
    std::string min_invest_constraint_name_ = "Min_Investment_Constraint";
    bool stop_update_ = true;
};
} // namespace Outerloop
