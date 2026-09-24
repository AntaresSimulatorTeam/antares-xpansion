#include <utility>

#include "antares-xpansion/benders/benders_core/BendersMasterManager.h"
#include "antares-xpansion/benders/benders_core/MasterUpdate.h"

using namespace Outerloop;

MasterUpdateBase::MasterUpdateBase(std::shared_ptr<BendersMasterManager> master_manager,
                                   double tau,
                                   double outer_loop_stopping_threshold):
    MasterUpdateBase(std::move(master_manager),
                     tau,
                     outer_loop_stopping_threshold,
                     "Min_Investment_Constraint")
{
}

MasterUpdateBase::MasterUpdateBase(std::shared_ptr<BendersMasterManager> master_manager,
                                   double tau,
                                   double outer_loop_stopping_threshold,
                                   const std::string& name):
    master_manager_(std::move(master_manager)),
    outer_loop_stopping_threshold_(outer_loop_stopping_threshold),
    min_invest_constraint_name_(name)
{
    CheckTau(tau);
}

void MasterUpdateBase::CheckTau(double tau)
{
    if (tau >= 0 && tau <= 1)
    {
        dichotomy_weight_coeff_ = tau;
    }
}

bool MasterUpdateBase::Update(double lambda_min, double lambda_max)
{
    stop_update_ = std::abs(lambda_max - lambda_min) < outer_loop_stopping_threshold_;
    if (!stop_update_)
    {
        lambda_ = dichotomy_weight_coeff_ * lambda_max + (1 - dichotomy_weight_coeff_) * lambda_min;
        UpdateConstraints();
    }
    return stop_update_;
}

void MasterUpdateBase::UpdateConstraints()
{
    if (!master_manager_->IsEmpty() && additional_constraint_index_ > -1)
    {
        master_manager_->ChangeRhs(additional_constraint_index_, lambda_);
    }
    else
    {
        AddMinInvestConstraint();
    }
}

/**
 * Add the new constraint in benders main problem
 * /!\ to be called once
 */
void MasterUpdateBase::AddMinInvestConstraint()
{
    auto master_variables = master_manager_->GetVariableMap();
    const auto obj_coeff = master_manager_->GetObjectiveFunctionCoeffs();
    auto newnz = master_variables.size();
    int newrows = 1;
    std::vector<char> rtype(newrows, 'G');
    std::vector<double> rhs(newrows, lambda_);
    std::vector<int> mclind(newnz);

    size_t mclindCnt_l(0);
    std::vector<double> matval(newnz);
    for (const auto& [name, var_id]: master_variables)
    {
        mclind[mclindCnt_l] = var_id;
        matval[mclindCnt_l] = obj_coeff.at(var_id);
        ++mclindCnt_l;
    }
    std::vector<int> matstart(newrows + 1);
    matstart[0] = 0;
    matstart[1] = newnz;
    if (!min_invest_constraint_name_.empty())
    {
        std::vector<std::string> row_names(newrows, min_invest_constraint_name_);

        master_manager_->AddRows(rtype, rhs, {}, matstart, mclind, matval, row_names);
    }
    else
    {
        master_manager_->AddRows(rtype, rhs, {}, matstart, mclind, matval);
    }
    additional_constraint_index_ = master_manager_->GetNrows() - 1;
}

double MasterUpdateBase::Rhs() const
{
    return lambda_;
}
