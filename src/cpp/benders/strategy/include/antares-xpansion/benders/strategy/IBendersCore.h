#pragma once

#include <string>

#include "antares-xpansion/benders/benders_core/BendersBase.h"

class IBendersCore
{
public:
    virtual ~IBendersCore() = default;
    virtual void launch() = 0;
    virtual void set_input_map(const CouplingMap& coupling_map) = 0;
    virtual int MasterRowIndex(const std::string& row_name) const = 0;
    virtual void MasterChangeRhs(int id_row, double val) const = 0;
    virtual LogData GetBestIterationData() const = 0;
    virtual WorkerMasterDataVect AllCuts() const = 0;
    virtual void SaveOuterLoopSolutionInOutputFile() const = 0;
    virtual void free() = 0;
    virtual void DoFreeProblems(bool) = 0;
    virtual void InitializeProblems() = 0;
    virtual std::string BendersName() const = 0;
    virtual double execution_time() const = 0;
};
