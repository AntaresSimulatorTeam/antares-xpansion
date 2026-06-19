#include "antares-xpansion/benders/benders_core/FixedSkeletonSubProblemBuilder.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <fstream>
#include <iostream>

#include <boost/mpi.hpp>
#include <boost/tokenizer.hpp>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

FixedSkeletonSubProblemBuilder::FixedSkeletonSubProblemBuilder(const std::filesystem::path& inputRoot,
                                                               Logger& logger,
                                                               std::string solver_name,
                                                               int log_level,
                                                               ProblemsFormat format):
    inputRoot_(inputRoot)
{
    logger_ = logger;
    build_sub_skeleton(solver_name, solver_log_manager_, log_level, format);
    read_coeffs_and_indices(CoeffType::constraints);
    read_coeffs_and_indices(CoeffType::objective);
    read_coeffs_and_indices(CoeffType::rhs);
    micro_iters_ = false;
    warm_start_ = false;
}

FixedSkeletonSubProblemBuilder::FixedSkeletonSubProblemBuilder(const std::filesystem::path& inputRoot,
                                                               Logger& logger,
                                                               std::shared_ptr<SolverAbstract> solver):
    inputRoot_(inputRoot),
    solver_(std::move(solver))
{
    logger_ = logger;
    read_coeffs_and_indices(CoeffType::constraints);
    read_coeffs_and_indices(CoeffType::objective);
    read_coeffs_and_indices(CoeffType::rhs);
    micro_iters_ = false;
    warm_start_ = false;
}

void FixedSkeletonSubProblemBuilder::read_keyed_coeffs_csv(
  const std::filesystem::path& csv_path,
  std::map<std::string, std::vector<double>>& dest)
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    std::ifstream stream(csv_path);
    if (!stream.is_open())
    {
        std::cerr << "Error: unable to open file " << csv_path << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    while (std::getline(stream, line))
    {
        Tokenizer tok(line, sep);
        std::vector<std::string> tokens(tok.begin(), tok.end());
        std::string key = tokens[0];
        std::vector<double> values_double;
        if (tokens.size() > 1)
        {
            values_double.resize(tokens.size() - 1);
            std::transform(tokens.begin() + 1,
                           tokens.end(),
                           values_double.begin(),
                           [](const std::string& s) { return std::stod(s); });
        }
        dest[key] = values_double;
    }
}

void FixedSkeletonSubProblemBuilder::read_indices_csv(const std::filesystem::path& csv_path,
                                                      std::vector<int>& dest_indices,
                                                      bool is_col)
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    std::ifstream stream(csv_path);
    if (!stream.is_open())
    {
        std::cerr << "Error: unable to open file " << csv_path << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::string line;
    std::vector<std::string> names;
    while (std::getline(stream, line))
    {
        Tokenizer tok(line, sep);
        names.assign(tok.begin(), tok.end());
    }
    for (auto& name: names)
    {
        if (is_col)
        {
            dest_indices.push_back(solver_->get_col_index(name));
        }
        else
        {
            dest_indices.push_back(solver_->get_row_index(name));
        }
    }
}

void FixedSkeletonSubProblemBuilder::read_coeffs_and_indices(CoeffType coeff_type)
{
    auto sub_dir = inputRoot_ / "sub";
    switch (coeff_type)
    {
    case CoeffType::constraints:
        read_keyed_coeffs_csv(sub_dir / "coef.csv", coeffs_);
        read_indices_csv(sub_dir / "coef_cols.csv", constraints_col_indices_, true);
        read_indices_csv(sub_dir / "coef_rows.csv", constraints_row_indices_, false);
        break;
    case CoeffType::objective:
        read_keyed_coeffs_csv(sub_dir / "obj_coef.csv", obj_coeffs_);
        read_indices_csv(sub_dir / "obj_cols.csv", obj_col_indices_, true);
        break;
    case CoeffType::rhs:
        read_keyed_coeffs_csv(sub_dir / "rhs.csv", rhs_);
        read_indices_csv(sub_dir / "rhs_rows.csv", rhs_row_indices_, false);
        break;
    }
}

void FixedSkeletonSubProblemBuilder::build_sub_skeleton(std::string solver_name,
                                                        const SolverLogManager& solver_log_manager,
                                                        int log_level,
                                                        ProblemsFormat format)
{
    SolverFactory solver_factory(logger_);
    solver_ = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);

    solver_->set_threads(1);
    solver_->set_output_log_level(log_level);
    std::filesystem::path skeleton_sub = inputRoot_ / "sub" / "sub.mps";

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(skeleton_sub);
    solver_IO_.configure(solver_name, format);

    benders_problem_provider_->provide_problem(solver_IO_, solver_);
}

int FixedSkeletonSubProblemBuilder::get_sub_number()
{
    return rhs_.size();
}

std::shared_ptr<SubproblemWorker> FixedSkeletonSubProblemBuilder::create_sub_solver_abstract(
  std::string sub_name,
  VariableMap& variable_map,
  double cut_coefficient_tolerance,
  double slave_weight)
{
    auto& coeffs_sub = coeffs_[sub_name];
    auto& coeffs_obj = obj_coeffs_[sub_name];
    auto& rhs_values = rhs_[sub_name];

    solver_->chg_coefs(constraints_row_indices_, constraints_col_indices_, coeffs_sub);
    solver_->chg_obj(obj_col_indices_, coeffs_obj);
    solver_->chg_rhs_values(rhs_row_indices_, rhs_values);

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                solver_,
                                                                logger_,
                                                                slave_weight);

    return subproblem_worker;
}
