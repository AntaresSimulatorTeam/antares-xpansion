#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/strategy/IBendersCore.h"

class BendersBaseAdapter: public IBendersCore
{
public:
    explicit BendersBaseAdapter(BendersBase& base):
        base_(base)
    {
    }

    void launch() override
    {
        base_.launch();
    }

    void set_input_map(const CouplingMap& coupling_map) override
    {
        base_.set_input_map(coupling_map);
    }

    int MasterRowIndex(const std::string& row_name) const override
    {
        return base_.MasterRowIndex(row_name);
    }

    void MasterChangeRhs(int id_row, double val) const override
    {
        base_.MasterChangeRhs(id_row, val);
    }

    LogData GetBestIterationData() const override
    {
        return base_.GetBestIterationData();
    }

    WorkerMasterDataVect AllCuts() const override
    {
        return base_.AllCuts();
    }

    void SaveOuterLoopSolutionInOutputFile() const override
    {
        base_.SaveOuterLoopSolutionInOutputFile();
    }

    void free() override
    {
        base_.free();
    }

    void DoFreeProblems(bool v) override
    {
        base_.DoFreeProblems(v);
    }

    void InitializeProblems() override
    {
        base_.InitializeProblems();
    }

    std::string BendersName() const override
    {
        return base_.BendersName();
    }

    double execution_time() const override
    {
        return base_.execution_time();
    }

private:
    BendersBase& base_;
};
