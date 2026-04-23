#include "antares-xpansion/benders/benders_core/MemOptim_subproblem_builder.h"
#include <antares-xpansion/benders/benders_core/SolverIO.h>


#include <fstream>
#include <iostream>

#include <boost/mpi.hpp>
#include <boost/tokenizer.hpp>

MemOptimSubProblemBuilder::MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot,Logger& logger) 
    : inputRoot_(inputRoot)
{
    logger_ = logger ; 
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
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_csv_path = inputRoot_ / "sub" / "coef.csv";
    std::ifstream coef_csv_stream(coef_csv_path);
    if (coef_csv_stream.is_open())
    {
        int j = 0;
        std::string line;
        std::cout<<"from read coef "<<std::endl;  
        while (std::getline(coef_csv_stream, line))
        {
            Tokenizer tok(line);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            std::string key = tokens[0];
            std::vector<std::string> values;
            if (tokens.size() > 1)
            values.assign(tokens.begin() + 1, tokens.end());
            coeffs_[key] = values;
        }

    }
}

void MemOptimSubProblemBuilder::read_coef_cols()
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_cols_path = inputRoot_ / "sub" / "coef_cols.csv";
    std::ifstream coef_cols_stream(coef_cols_path);
    if (coef_cols_stream.is_open())
    {
        std::string line;

        while (std::getline(coef_cols_stream, line))
        {
            Tokenizer tok(line);
            cof_cols_.assign(tok.begin(), tok.end());
        }
    }
}

void MemOptimSubProblemBuilder::read_coef_rows()
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_rows_path = inputRoot_ / "sub" / "coef_rows.csv";
    std::ifstream coef_rows_stream(coef_rows_path);
    if (coef_rows_stream.is_open())
    {
        std::string line;
        while (std::getline(coef_rows_stream, line))
        {
            Tokenizer tok(line);
            coef_rows_.assign(tok.begin(), tok.end());
        }
    }
}

void MemOptimSubProblemBuilder::read_obj_coef()
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto obj_coef_path = inputRoot_ / "sub" / "obj_coef.csv";
    std::ifstream obj_coef_stream(obj_coef_path);
    if (obj_coef_stream.is_open())
    {
        std::cout << "obj_coef not null" << std::endl;
        std::string line;
        int j = 0;
        while (std::getline(obj_coef_stream, line))
        {
            Tokenizer tok(line);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            std::string key = tokens[0];
            std::vector<std::string> values;
            if (tokens.size() > 1)
                values.assign(tokens.begin() + 1, tokens.end());
            if (j < 2)

            ++j;
            obj_coefs_[key] = values;
        }
    }
}

void MemOptimSubProblemBuilder::read_obj_cols()
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto obj_cols_path = inputRoot_ / "sub" / "obj_cols.csv";
    std::ifstream obj_cols_stream(obj_cols_path);

    if (obj_cols_stream.is_open())
    {
        std::string line;
        while (std::getline(obj_cols_stream, line))
        {
            Tokenizer tok(line);
            obj_cols_.assign(tok.begin(), tok.end());
        }
    }
}

void MemOptimSubProblemBuilder::read_rhs()
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto rhs_path = inputRoot_ / "sub" / "rhs.csv";
    std::ifstream rhs_stream(rhs_path);
    if (rhs_stream.is_open())
    {
        std::string line;
        int j = 0;

        while (std::getline(rhs_stream, line))
        {
            Tokenizer tok(line);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            std::string key = tokens[0];
            std::vector<std::string> values;
            if (tokens.size() > 1)
                values.assign(tokens.begin() + 1, tokens.end());
            rhs_[key] = values;
        }

    }
}

void MemOptimSubProblemBuilder::read_rhs_rows()
{
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto rhs_rows_path = inputRoot_ / "sub" / "rhs_rows.csv";
    std::ifstream rhs_rows_stream(rhs_rows_path);
    if (rhs_rows_stream.is_open())
    {
        std::string line;
        while (std::getline(rhs_rows_stream, line))
        {
            Tokenizer tok(line);
            rhs_rows_.assign(tok.begin(), tok.end());
        }
    }
}


void  MemOptimSubProblemBuilder::build_sub_skeleton()
{
  
} 
