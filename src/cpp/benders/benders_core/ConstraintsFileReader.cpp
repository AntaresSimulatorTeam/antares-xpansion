#include "antares-xpansion/benders/benders_core/ConstraintsFileReader.h"

ConstraintsFileReader::ConstraintsFileReader(const std::filesystem::path constraint_file_path,
                                             const std::string& solver_name,
                                             const SolverLogManager& solver_log_manager,
                                             Logger& logger,
                                             int log_level,
                                             ProblemsFormat format):
    logger_(logger)
{
    SolverFactory solver_factory(logger_);
    solver_ = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);

    solver_->set_threads(1);
    solver_->set_output_log_level(log_level);


    std::cout<<"constraint_file_path "<<std::endl ; 
    std::cout<<"number of rows "<<solver_->get_nrows()<<std::endl ; 

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(constraint_file_path);
    solver_IO_.configure(solver_name, format);
    benders_problem_provider_->provide_problem(solver_IO_, solver_);
}

ConstraintsFileReader::ConstraintsFileReader(std::shared_ptr<SolverAbstract> solver):
    solver_(std::move(solver))
{
}

int ConstraintsFileReader::get_row_index(const std::string& name)
{
    return solver_->get_row_index(name);
}

SolverRepresentedRows ConstraintsFileReader::get_row(const std::string& name)
{
    SolverRepresentedRows result;
    result.range_p = {};
    result.row_names = {name};
    int constraint_pos = get_row_index(name);

    int ncols = solver_->get_ncols();
    result.mstart.resize(2);
    result.mclind.resize(ncols);
    result.dmatval.resize(ncols);

    int nels(0);
    solver_->get_rows(result.mstart.data(),
                      result.mclind.data(),
                      result.dmatval.data(),
                      ncols,
                      &nels,
                      constraint_pos,
                      constraint_pos);

    result.mclind.resize(nels);
    result.dmatval.resize(nels);
    result.mstart.resize(1);

    double rhs(0.);
    solver_->get_rhs(&rhs, constraint_pos, constraint_pos);
    result.rhs = {rhs};

    double range_p(0.);
    solver_->get_rhs_range(&range_p, constraint_pos, constraint_pos);
    result.range_p = {range_p};

    const int MAX_LEN = 10;
    char buffer[MAX_LEN];
    solver_->get_row_type(buffer, constraint_pos, constraint_pos);
    int len = 0;
    while (len < MAX_LEN && buffer[len] >= 'A' && buffer[len] <= 'Z')
    {
        ++len;
    }
    std::string qrtype(buffer, len);
    result.qrtype_p = {qrtype[0]};

    return result;
}
