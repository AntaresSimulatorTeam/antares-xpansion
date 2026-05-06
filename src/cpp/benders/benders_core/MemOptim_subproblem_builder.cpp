#include "antares-xpansion/benders/benders_core/MemOptim_subproblem_builder.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <chrono>
#include <fstream>
#include <iostream>

#include <boost/mpi.hpp>
#include <boost/tokenizer.hpp>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

MemOptimSubProblemBuilder::MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot,
                                                     Logger& logger,
                                                     std::string solver_name,
                                                     int log_level,
                                                     ProblemsFormat format):
    inputRoot_(inputRoot)
{
    logger_ = logger;
    build_sub_skeleton(solver_name, solver_log_manager_, log_level, format);
    read_coef();
    read_coef_cols();
    read_coef_rows();
    read_obj_coef();
    read_obj_cols();
    read_rhs();
    read_rhs_rows();
}

void MemOptimSubProblemBuilder::read_coef()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_csv_path = inputRoot_ / "sub" / "coef.csv";
    std::ifstream coef_csv_stream(coef_csv_path);
    if (coef_csv_stream.is_open())
    {
        int j = 0;
        std::string line;
        while (std::getline(coef_csv_stream, line))
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
            coeffs_[key] = values_double;
        }
    }
}

void MemOptimSubProblemBuilder::read_coef_cols()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_cols_path = inputRoot_ / "sub" / "coef_cols.csv";
    std::ifstream coef_cols_stream(coef_cols_path);
    if (coef_cols_stream.is_open())
    {
        std::string line;

        while (std::getline(coef_cols_stream, line))
        {
            Tokenizer tok(line, sep);
            coef_cols_.assign(tok.begin(), tok.end());
        }
        for (auto& col_name: coef_cols_)
        {
            constraints_col_indices_.push_back(solver_->get_col_index(col_name));
        }
    }
}

void MemOptimSubProblemBuilder::read_coef_rows()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_rows_path = inputRoot_ / "sub" / "coef_rows.csv";
    std::ifstream coef_rows_stream(coef_rows_path);
    if (coef_rows_stream.is_open())
    {
        std::string line;
        while (std::getline(coef_rows_stream, line))
        {
            Tokenizer tok(line, sep);
            coef_rows_.assign(tok.begin(), tok.end());
        }

        for (auto& row_name: coef_rows_)
        {
            constraints_row_indices_.push_back(solver_->get_row_index(row_name));
        }
    }
}

void MemOptimSubProblemBuilder::read_obj_coef()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto obj_coef_path = inputRoot_ / "sub" / "obj_coef.csv";
    std::ifstream obj_coef_stream(obj_coef_path);
    if (obj_coef_stream.is_open())
    {
        std::string line;
        int j = 0;
        while (std::getline(obj_coef_stream, line))
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
            if (j < 2)
            {
                ++j;
            }
            obj_coefs_[key] = values_double;
        }
    }
}

void MemOptimSubProblemBuilder::read_obj_cols()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto obj_cols_path = inputRoot_ / "sub" / "obj_cols.csv";
    std::ifstream obj_cols_stream(obj_cols_path);

    if (obj_cols_stream.is_open())
    {
        std::string line;
        while (std::getline(obj_cols_stream, line))
        {
            Tokenizer tok(line, sep);
            obj_cols_.assign(tok.begin(), tok.end());
        }
        for (auto& obj_col: obj_cols_)
        {
            obj_col_indices_.push_back(solver_->get_col_index(obj_col));
        }
    }
}

void MemOptimSubProblemBuilder::read_rhs()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto rhs_path = inputRoot_ / "sub" / "rhs.csv";
    std::ifstream rhs_stream(rhs_path);
    if (rhs_stream.is_open())
    {
        std::string line;
        int j = 0;

        while (std::getline(rhs_stream, line))
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
            rhs_[key] = values_double;
        }
    }
}

void MemOptimSubProblemBuilder::read_rhs_rows()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto rhs_rows_path = inputRoot_ / "sub" / "rhs_rows.csv";
    std::ifstream rhs_rows_stream(rhs_rows_path);
    if (rhs_rows_stream.is_open())
    {
        std::string line;
        while (std::getline(rhs_rows_stream, line))
        {
            Tokenizer tok(line, sep);
            rhs_rows_.assign(tok.begin(), tok.end());
        }
        for (auto& rhs_row: rhs_rows_)
        {
            rhs_row_indices_.push_back(solver_->get_row_index(rhs_row));
        }
    }
}

void MemOptimSubProblemBuilder::build_sub_skeleton(std::string solver_name,
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

    int number_of_rows = solver_->get_nrows();
}

int MemOptimSubProblemBuilder::get_sub_number()
{
    return rhs_.size();
}

std::shared_ptr<SubproblemWorker> MemOptimSubProblemBuilder::create_sub_solver_abstract(
  std::string sub_name,
  VariableMap& variable_map,
  double cut_coefficient_tolerance,
  double slave_weight)
{
    auto& coeffs_sub = coeffs_[sub_name];
    auto& coeffs_obj = obj_coefs_[sub_name];
    auto& rhs_values = rhs_[sub_name];
    int n_coefs = coeffs_sub.size();
    auto sub_solver = solver_;

    auto start = std::chrono::high_resolution_clock::now();
    sub_solver->chg_coefs(n_coefs,
                          constraints_row_indices_.data(),
                          constraints_col_indices_.data(),
                          coeffs_sub.data());
    sub_solver->chg_obj(obj_col_indices_, coeffs_obj);
    sub_solver->chg_rhs_values(rhs_row_indices_, rhs_values);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                sub_solver,
                                                                logger_,
                                                                slave_weight);

    return subproblem_worker;
}
