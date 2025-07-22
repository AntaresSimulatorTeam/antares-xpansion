#include "antares-xpansion/benders/benders_core/Worker.h"

#include <utility>

#include "antares-xpansion/helpers/solver_utils.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

/*!
 *  \brief Free the problem
 */
void Worker::free()
{
    if (_solver)
    {
        _solver.reset();
        _solver = nullptr;
    }
}

/*!
 *  \brief Return the optimal value of a problem
 *
 *  \param lb : double which receives the optimal value
 */
void Worker::get_value(double& lb) const
{
    if (_is_master && _solver->get_n_integer_vars() > 0)
    {
        lb = _solver->get_mip_value();
    }
    else
    {
        lb = _solver->get_lp_value();
    }
}

/*!
 *  \brief Initialization of a problem
 *
 *  \param variable_map : map linking each problem name to its variables and
 * their ids
 *
 *  \param problem_name : name of the problem
 */
void Worker::init(const std::string& solver_name,
                  int log_level,
                  const SolverLogManager& solver_log_manager,
                  ProblemsFormat format)
{
    solver_io_.configure(solver_name, format);
    SolverFactory factory(logger_);

    if (_is_master)
    {
        _solver = factory.create_solver(solver_name, SOLVER_TYPE::INTEGER, solver_log_manager);
    }
    else
    {
        _solver = factory.create_solver(solver_name, SOLVER_TYPE::CONTINUOUS, solver_log_manager);
    }

    read_prob(_solver.get(), _path_to_mps);
    // Always set solver parameters after reading problems, as restore (used by Xpress writing .svf
    // files) also comes with parameters of the solver and we do not want them to override our
    // preferences
    _solver->set_threads(1);
    _solver->set_output_log_level(log_level);

    for (const auto& kvp: _name_to_id)
    {
        _id_to_name[kvp.second] = kvp.first;
    }
}

/*!
 *  \brief Method to solve a problem
 *
 *  \param lp_status : problem status after optimization
 */
void Worker::solve(int& lp_status,
                   const std::string& outputroot,
                   const std::string& output_master_mps_file_name,
                   std::shared_ptr<Output::OutputWriter> writer) const
{
    if (_is_master && _solver->get_n_integer_vars() > 0)
    {
        lp_status = _solver->solve_mip();
    }
    else
    {
        lp_status = _solver->solve_lp();
    }

    if (lp_status != SOLVER_STATUS::OPTIMAL)
    {
        std::filesystem::path error_file_path;
        auto problem_status = _solver->SOLVER_STRING_STATUS[lp_status];
        error_file_path = std::filesystem::path(outputroot)
                          / (_path_to_mps.filename().string() + "_lp_status_" + problem_status
                             + MPS_SUFFIX);
        std::ostringstream msg;
        msg << "lp_status is : " << problem_status << std::endl;

        msg << "written in " << error_file_path.string() << std::endl;
        logger_->display_message(msg.str());
        writeProb(error_file_path);
        Output::ProblemData data;
        data.name = _path_to_mps.filename().string();
        data.path = error_file_path;
        data.status = problem_status;
        writer->WriteProblem(data);
        writer->dump();
        auto log_location = LOGLOCATION;
        msg.str("");
        msg << "Invalid solver status " + problem_status + " optimality expected";
        logger_->display_message(log_location + msg.str());
        throw InvalidSolverStatusException(msg.str(), log_location);
    }

    if (_is_master)
    {
        writeProb(std::filesystem::path(outputroot) / output_master_mps_file_name);
    }
}

/*!
 *  \brief Get the number of iteration needed to solve a problem
 *
 *  \param result : result
 */
void Worker::get_splex_num_of_ite_last(int& result) const
{
    result = _solver->get_splex_num_of_ite_last();
}

void Worker::write_basis(const std::filesystem::path& filename) const
{
    _solver->write_basis(filename);
}

int Worker::RowIndex(const std::string& row_name) const
{
    return _solver->get_row_index(row_name);
}

void Worker::ChangeRhs(int id_row, double val) const
{
    _solver->chg_rhs(id_row, val);
}

void Worker::GetRhs(double* val, int id_row) const
{
    _solver->get_rhs(val, id_row, id_row);
}

void Worker::AddRows(const std::vector<char>& qrtype_p,
                     const std::vector<double>& rhs_p,
                     const std::vector<double>& range_p,
                     const std::vector<int>& mstart_p,
                     const std::vector<int>& mclind_p,
                     const std::vector<double>& dmatval_p,
                     const std::vector<std::string>& row_names) const
{
    solver_addrows(*_solver, qrtype_p, rhs_p, {}, mstart_p, mclind_p, dmatval_p, row_names);
}

int Worker::Getnrows() const
{
    return _solver->get_nrows();
}

int Worker::Getncols() const
{
    return _solver->get_ncols();
}

/**
 * Have `problem` read the problem problem data from `path`
 *
 * Used to hold logic to select between mps/save/etc.
 * @param problem
 * @param path
 */
void Worker::read_prob(SolverAbstract* problem, const std::filesystem::path& path) const
{
    solver_io_.read(_solver.get(), path);
}

std::shared_ptr<SolverAbstract> Worker::solver() const
{
    return _solver;
}

Worker::Worker(VariableMap variable_map,
               std::filesystem::path path_to_mps,
               Logger logger,
               double cut_coefficient_tolerance):
    _path_to_mps{std::move(path_to_mps)},
    _name_to_id{std::move(variable_map)},
    logger_{std::move(logger)},
    cut_coefficient_tolerance_{cut_coefficient_tolerance}
{
}

void Worker::writeProb(const std::filesystem::path& out) const
{
    solver_io_.write(_solver.get(), out);
}

void Worker::roundIfWithinTolerance(std::vector<double>& values) const
{
    std::transform(values.begin(),
                   values.end(),
                   values.begin(),
                   [this](double value) -> double
                   { return std::abs(value) < cut_coefficient_tolerance_ ? 0 : value; });
}
