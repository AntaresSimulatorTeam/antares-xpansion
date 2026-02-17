#pragma once

#include "antares-xpansion/benders/strategy/IBendersCore.h"
#include <string>

// forward declaration to avoid heavy include dependencies
class BendersBase;

class BendersBaseAdapter : public IBendersCore
{
public:
    // accepte pointeur non-owning (peut être nullptr)
    explicit BendersBaseAdapter(BendersBase* base = nullptr) : base_(base) {}

    void launch() override { if (base_) base_->launch(); }
    void set_input_map(const CouplingMap& coupling_map) override { if (base_) base_->set_input_map(coupling_map); }
    [[nodiscard]] int MasterRowIndex(const std::string& row_name) const override { return base_ ? base_->MasterRowIndex(row_name) : -1; }
    void MasterChangeRhs(int id_row, double val) const override { if (base_) base_->MasterChangeRhs(id_row, val); }
    [[nodiscard]] LogData GetBestIterationData() const override { return base_ ? base_->GetBestIterationData() : LogData{}; }
    [[nodiscard]] WorkerMasterDataVect AllCuts() const override { return base_ ? base_->AllCuts() : WorkerMasterDataVect{}; }
    void SaveOuterLoopSolutionInOutputFile() const override { if (base_) base_->SaveOuterLoopSolutionInOutputFile(); }
    void free() override { if (base_) base_->free(); }
    void DoFreeProblems(bool v) override { if (base_) base_->DoFreeProblems(v); }
    void InitializeProblems() override { if (base_) base_->InitializeProblems(); }
    [[nodiscard]] std::string BendersName() const override { return base_ ? base_->BendersName() : std::string("BendersBaseAdapter"); }
    [[nodiscard]] double execution_time() const override { return base_ ? base_->execution_time() : 0.0; }

private:
    BendersBase* base_; // non-owning
};
