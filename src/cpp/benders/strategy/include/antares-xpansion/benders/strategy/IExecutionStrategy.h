#pragma once

#include <string>
#include "antares-xpansion/benders/benders_core/BendersBase.h"

/**
 * @interface IExecutionStrategy
 * @brief Interface for Benders execution strategies
 * 
 * Defines the contract for execution strategies (Sequential, ParallelMPI, etc.)
 * Extended to expose necessary BendersBase methods for proper delegation from BendersCore.
 */
class IExecutionStrategy
{
public:
    virtual ~IExecutionStrategy() = default;
    
    // Core execution methods
    virtual void launch() = 0;
    virtual void InitializeProblems() = 0;
    virtual void Run() = 0;
    
    // Naming and timing
    virtual std::string BendersName() const = 0;
    virtual double execution_time() const = 0;
    
    // Master problem interaction
    virtual void set_input_map(const CouplingMap& coupling_map) = 0;
    virtual int MasterRowIndex(const std::string& row_name) const = 0;
    virtual void MasterChangeRhs(int id_row, double val) const = 0;
    
    // Results and data access
    virtual LogData GetBestIterationData() const = 0;
    virtual WorkerMasterDataVect AllCuts() const = 0;
    
    // Resource management
    virtual void free() = 0;
    virtual void DoFreeProblems(bool v) = 0;
};

