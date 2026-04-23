#pragma once 

#include "antares-xpansion/multisolver_interface/Solver.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include <filesystem> 
#include <boost/tokenizer.hpp>

class MemOptimSubProblemBuilder
{
   public:
    MemOptimSubProblemBuilder(const std::filesystem::path& inputRoot);

  private:
    void read_coef();
    void read_coef_cols();
    void read_coef_rows();
    void read_obj_coef();
    void read_obj_cols();
    void read_rhs();
    void read_rhs_rows();

    std::filesystem::path inputRoot_;
    std::map<std::string, std::vector<std::string>> coeffs_;
    std::vector<std::string> cof_cols_;
    std::vector<std::string> coef_rows_;
    std::map<std::string, std::vector<std::string>> obj_coefs_;
    std::vector<std::string> obj_cols_;
    std::vector<std::string> rhs_rows_;
    std::map<std::string, std::vector<std::string>> rhs_;
};  