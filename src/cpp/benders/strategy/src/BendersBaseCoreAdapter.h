#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/strategy/IBendersCore.h"

class BendersBaseCoreAdapter: public IBendersCore
{
public:
    explicit BendersBaseCoreAdapter(pBendersBase benders):
        benders_(std::move(benders))
    {
    }

    void launch() override
    {
        benders_->launch();
    }

    void set_input_map(const CouplingMap& coupling_map) override
    {
        benders_->set_input_map(coupling_map);
    }

    int MasterRowIndex(const std::string& row_name) const override
    {
        return benders_->MasterRowIndex(row_name);
    }

    void MasterChangeRhs(int id_row, double val) const override
    {
        benders_->MasterChangeRhs(id_row, val);
    }

    LogData GetBestIterationData() const override
    {
        return benders_->GetBestIterationData();
    }

    WorkerMasterDataVect AllCuts() const override
    {
        return benders_->AllCuts();
    }

    void SaveOuterLoopSolutionInOutputFile() const override
    {
        benders_->SaveOuterLoopSolutionInOutputFile();
    }

    void free() override
    {
        benders_->free();
    }

    void DoFreeProblems(bool val) override
    {
        benders_->DoFreeProblems(val);
    }

    void InitializeProblems() override
    {
        benders_->InitializeProblems();
    }

    std::string BendersName() const override
    {
        return benders_->BendersName();
    }

    double execution_time() const override
    {
        return benders_->execution_time();
    }

private:
    pBendersBase benders_;
};
