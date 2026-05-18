#include "antares-xpansion/benders/benders_core/ConstraintsFileReaderMemOptim.h" 


ConstraintsFileReaderMemOptim::ConstraintsFileReaderMemOptim(const std::filesystem::path& inputRoot,
                                          const std::string& solver_name,
                                          const SolverLogManager& solver_log_manager,
                                          Logger& logger,
                                          int log_level,
                                          ProblemsFormat format)
{
    build_skeleton(solver_name,
                   solver_log_manager,
                    log_level,
                    format) ;


    read_coef(); 
    read_coef_cols();
    read_coef_rows();
    read_obj_coef();
    read_obj_cols();
    read_rhs();
    read_rhs_rows();
    build_skeleton(solver_name,solver_log_manager,log_level,format) ; 
} 



void ConstraintsFileReaderMemOptim::read_coef()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_csv_path = inputRoot_ / "constraints" / "coef.csv";
    std::ifstream coef_csv_stream(coef_csv_path);
    if (coef_csv_stream.is_open())
    {
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

void ConstraintsFileReaderMemOptim::read_coef_cols()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_cols_path = inputRoot_ / "constraints" / "coef_cols.csv";
    std::ifstream coef_cols_stream(coef_cols_path);
    if (coef_cols_stream.is_open())
    {
        std::string line;
        while (std::getline(coef_cols_stream, line))
        {
            Tokenizer tok(line, sep);
            coef_cols_.assign(tok.begin(), tok.end());
        }
    }
}

void ConstraintsFileReaderMemOptim::read_coef_rows()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto coef_rows_path = inputRoot_ / "constraints" / "coef_rows.csv";
    std::ifstream coef_rows_stream(coef_rows_path);
    if (coef_rows_stream.is_open())
    {
        std::string line;
        while (std::getline(coef_rows_stream, line))
        {
            Tokenizer tok(line, sep);
            coef_rows_.assign(tok.begin(), tok.end());
        }
    }
}

void ConstraintsFileReaderMemOptim::read_obj_coef()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto obj_coef_path = inputRoot_ / "constraints" / "obj_coef.csv";
    std::ifstream obj_coef_stream(obj_coef_path);
    if (obj_coef_stream.is_open())
    {
        std::string line;
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
            obj_coefs_[key] = values_double;
        }
    }
}

void ConstraintsFileReaderMemOptim::read_obj_cols()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto obj_cols_path = inputRoot_ / "constraints" / "obj_cols.csv";
    std::ifstream obj_cols_stream(obj_cols_path);
    if (obj_cols_stream.is_open())
    {
        std::string line;
        while (std::getline(obj_cols_stream, line))
        {
            Tokenizer tok(line, sep);
            obj_cols_.assign(tok.begin(), tok.end());
        }
    }
}

void ConstraintsFileReaderMemOptim::read_rhs()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto rhs_path = inputRoot_ / "constraints" / "rhs.csv";
    std::ifstream rhs_stream(rhs_path);
    if (rhs_stream.is_open())
    {
        std::string line;
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

void ConstraintsFileReaderMemOptim::read_rhs_rows()
{
    boost::char_separator<char> sep(",");
    using Tokenizer = boost::tokenizer<boost::char_separator<char>>;

    auto rhs_rows_path = inputRoot_ / "constraints" / "rhs_rows.csv";
    std::ifstream rhs_rows_stream(rhs_rows_path);
    if (rhs_rows_stream.is_open())
    {
        std::string line;
        while (std::getline(rhs_rows_stream, line))
        {
            Tokenizer tok(line, sep);
            rhs_rows_.assign(tok.begin(), tok.end());
        }
    }
}

void ConstraintsFileReaderMemOptim::read_constraints_for_mem_optim() 
{
 
} 


void ConstraintsFileReaderMemOptim::build_skeleton(std::string solver_name,
                                                   const SolverLogManager& solver_log_manager,
                                                   int log_level,
                                                   ProblemsFormat format)
{
    SolverFactory solver_factory(logger_);
    solver_skeleton = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);

    solver_skeleton->set_threads(1);
    solver_skeleton->set_output_log_level(log_level);
    std::filesystem::path skeleton_sub = inputRoot_ / "sub" / "sub.mps";

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(skeleton_sub);
    solver_IO_.configure(solver_name, format);

    benders_problem_provider_->provide_problem(solver_IO_, solver_skeleton);

    int number_of_rows = solver_skeleton->get_nrows();
}




