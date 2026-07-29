#pragma once

#include <memory>
#include <string>
#include <vector>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

struct SolverRepresentedRows
{
    std::vector<int> mstart;
    std::vector<int> mclind;
    std::vector<double> dmatval;
    std::vector<double> range_p;
    std::vector<char> qrtype_p;
    std::vector<double> rhs;
    std::vector<std::string> row_names;
};

class SolverRowExtractor
{
public:
    explicit SolverRowExtractor(std::shared_ptr<SolverAbstract> solver);

    SolverRepresentedRows GetRow(const std::string& name);
    static SolverRepresentedRows GetRow(std::shared_ptr<SolverAbstract> solver, int row_pos);

private:
    int get_row_index(const std::string& name);

    std::shared_ptr<SolverAbstract> solver_;
};
