#pragma once

#include <filesystem>

#include "IBendersProblemProvider.h"
#include "SolverIO.h"
#include "antares-xpansion/benders/output/OutputWriter.h"
#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common.h"

/*!
 * \class Worker
 * \brief Mother-class Worker
 *
 *  This class opens and sets a problem using a BendersProblemProvider and a mapping variable map
 */
class Worker
{
public:
    Worker(VariableMap variable_map, Logger logger, double cut_coefficient_tolerance);
    virtual void init(const std::string& solver_name,
                      int log_level,
                      const SolverLogManager& solver_log_manager,
                      ProblemsFormat format,
                      IBendersProblemProvider* benders_problem_provider);
    virtual ~Worker() = default;

    void get_value(double& lb) const;

    void get_splex_num_of_ite_last(int& result) const;

    void free();
    void write_basis(const std::filesystem::path& filename) const;
    [[nodiscard]] virtual std::shared_ptr<SolverAbstract> solver() const;

public:
    std::filesystem::path _base_filename;
    VariableMap _name_to_id; /*!< Link between the variable name and its identifier */
    Int2Str _id_to_name;     /*!< Link between the identifier of a variable and its name*/

public:
    void solve(int& lp_status,
               const std::string& outputroot,
               const std::string& output_master_mps_file_name,
               std::shared_ptr<Output::OutputWriter> writer) const;
    int RowIndex(const std::string& row_name) const;
    void ChangeRhs(int id_row, double val) const;
    void GetRhs(double* val, int id_row) const;
    void AddRows(const std::vector<char>& qrtype_p,
                 const std::vector<double>& rhs_p,
                 const std::vector<double>& range_p,
                 const std::vector<int>& mstart_p,
                 const std::vector<int>& mclind_p,
                 const std::vector<double>& dmatval_p,
                 const std::vector<std::string>& row_names) const;

    /**
     * @brief Returns the number of rows (constraints)
     *
     * @param solver_p  : solver containing the model to consider.
     */
    int Getnrows() const;

    int Getncols() const;

public:
    std::shared_ptr<SolverAbstract> _solver = nullptr; /*!< Problem stocked in the instance Worker*/
    bool _is_master = false;

    Logger logger_;

private:
    SolverIO solver_io_;
    void writeProb(const std::filesystem::path& out) const;
    double cut_coefficient_tolerance_;

protected:
    void roundIfWithinTolerance(std::vector<double>& values, int first, int last) const;
};
