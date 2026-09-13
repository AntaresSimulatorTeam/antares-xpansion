#include "SolverMathOpt.h"

#include <atomic>
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>

#include "ortools/math_opt/io/lp_converter.h"
#include "ortools/math_opt/io/lp_parser.h"
#include "ortools/math_opt/io/mps_converter.h"

using namespace std::literals;

namespace
{
std::atomic<int>& number_of_problems_counter()
{
    static std::atomic<int> counter{0};
    return counter;
}

constexpr double kInf = std::numeric_limits<double>::infinity();
} // namespace

/*************************************************************************************************
-----------------------------------    Constructor/Destructor
--------------------------------
*************************************************************************************************/

SolverMathOpt::SolverMathOpt():
    _model(std::make_unique<math_opt::Model>("mathopt"))
{
    number_of_problems_counter() += 1;
}

SolverMathOpt::SolverMathOpt(const SolverLogManager& log_manager):
    SolverMathOpt()
{
    if (log_manager.log_file_path != "")
    {
        _log_file = log_manager.log_file_path;
        _log_stream.open(_log_file, std::ofstream::out | std::ofstream::app);
        add_stream(_log_stream);
    }
}

SolverMathOpt::SolverMathOpt(const SolverMathOpt& other):
    SolverMathOpt()
{
    _log_file = other._log_file;
    if (_log_file != "")
    {
        _log_stream.open(_log_file, std::ofstream::out | std::ofstream::app);
        add_stream(_log_stream);
    }
    _minimize = other._minimize;
    _log_level = other._log_level;
    _threads = other._threads;
    _iteration_limit = other._iteration_limit;

    // Rebuild model from scratch by copying structure
    _model = std::make_unique<math_opt::Model>("mathopt");

    // Copy variables
    for (const auto& var: other._variables)
    {
        auto new_var = _model->AddContinuousVariable(other._model->lower_bound(var),
                                                     other._model->upper_bound(var),
                                                     other._model->name(var));
        if (other._model->is_integer(var))
        {
            _model->set_integer(new_var);
        }
        _variables.push_back(new_var);
    }

    // Copy objective
    if (other._minimize)
    {
        _model->set_minimize();
    }
    else
    {
        _model->set_maximize();
    }
    for (size_t i = 0; i < other._variables.size(); ++i)
    {
        _model->set_objective_coefficient(_variables[i],
                                          other._model->objective_coefficient(other._variables[i]));
    }

    // Copy constraints
    for (size_t r = 0; r < other._constraints.size(); ++r)
    {
        const auto& src_ct = other._constraints[r];
        double lb = other._model->lower_bound(src_ct);
        double ub = other._model->upper_bound(src_ct);

        auto new_ct = _model->AddLinearConstraint(lb, ub, other._model->name(src_ct));

        // Copy coefficients
        for (const auto& var: other._model->RowNonzeros(src_ct))
        {
            double coeff = other._model->coefficient(src_ct, var);
            // Find the index of this variable in other._variables
            for (size_t j = 0; j < other._variables.size(); ++j)
            {
                if (var.id() == other._variables[j].id())
                {
                    _model->set_coefficient(new_ct, _variables[j], coeff);
                    break;
                }
            }
        }
        _constraints.push_back(new_ct);
    }

    _row_types = other._row_types;
    rebuild_index_maps();
}

SolverMathOpt::~SolverMathOpt()
{
    number_of_problems_counter() -= 1;
    if (_log_stream.is_open())
    {
        _log_stream.close();
    }
}

SolverMathOpt* SolverMathOpt::clone() const
{
    return new SolverMathOpt(*this);
}

int SolverMathOpt::get_number_of_instances()
{
    return number_of_problems_counter();
}

/*************************************************************************************************
-----------------------------------    Init / Free
--------------------------------
*************************************************************************************************/

void SolverMathOpt::init()
{
    _model = std::make_unique<math_opt::Model>("mathopt");
    _variables.clear();
    _constraints.clear();
    _row_types.clear();
    _last_result.reset();
    _initial_basis.reset();
    _col_name_to_index.clear();
    _row_name_to_index.clear();
    _minimize = true;
}

void SolverMathOpt::free()
{
    // RAII — nothing to do
}

/*************************************************************************************************
-------------------------------    Reading & Writing problems
-------------------------------
*************************************************************************************************/

void SolverMathOpt::write_prob_mps(const std::filesystem::path& filename)
{
    auto proto = _model->ExportModel();
    auto mps_or = operations_research::math_opt::ModelProtoToMps(proto);
    if (!mps_or.ok())
    {
        throw GenericSolverException("Failed to export MPS: "
                                     + std::string(mps_or.status().message()));
    }

    auto path = filename;
    if (path.extension() != ".mps")
    {
        path.replace_extension(".mps");
    }
    std::ofstream ofs(path);
    ofs << *mps_or;
}

void SolverMathOpt::write_prob_lp(const std::filesystem::path& filename)
{
    auto proto = _model->ExportModel();
    auto lp_or = operations_research::math_opt::ModelProtoToLp(proto);
    if (!lp_or.ok())
    {
        throw GenericSolverException("Failed to export LP: "
                                     + std::string(lp_or.status().message()));
    }

    auto path = filename;
    if (path.extension() != ".lp")
    {
        path.replace_extension(".lp");
    }
    std::ofstream ofs(path);
    ofs << *lp_or;
}

void SolverMathOpt::save_prob(const std::filesystem::path& filename)
{
    write_prob_mps(filename);
}

void SolverMathOpt::write_basis(const std::filesystem::path& filename)
{
    if (!_last_result.has_value() || _last_result->solutions.empty()
        || !_last_result->solutions[0].basis.has_value())
    {
        throw GenericSolverException(
          "write_basis: no basis available (solve not called or no basis)");
    }

    const auto& basis = *_last_result->solutions[0].basis;
    using BS = math_opt::BasisStatus;

    auto status_to_int = [](BS s) -> int
    {
        switch (s)
        {
        case BS::kFree:
            return 0;
        case BS::kBasic:
            return 1;
        case BS::kAtUpperBound:
            return 2;
        case BS::kAtLowerBound:
            return 3;
        case BS::kFixedValue:
            return 4;
        default:
            return 0;
        }
    };

    std::ofstream ofs(filename);
    if (!ofs)
    {
        throw GenericSolverException("write_basis: cannot open file " + filename.string());
    }

    ofs << "MATHOPT_BASIS v1\n";
    ofs << "COLUMNS " << get_ncols() << "\n";
    for (int i = 0; i < get_ncols(); ++i)
    {
        auto it = basis.variable_status.find(_variables[i]);
        int s = (it != basis.variable_status.end()) ? status_to_int(it->second) : 0;
        ofs << std::string(_model->name(_variables[i])) << " " << s << "\n";
    }

    ofs << "ROWS " << get_nrows() << "\n";
    for (int i = 0; i < get_nrows(); ++i)
    {
        auto it = basis.constraint_status.find(_constraints[i]);
        int s = (it != basis.constraint_status.end()) ? status_to_int(it->second) : 0;
        ofs << std::string(_model->name(_constraints[i])) << " " << s << "\n";
    }

    ofs << "ENDATA\n";
}

void SolverMathOpt::read_prob_mps(const std::filesystem::path& filename)
{
    auto path = filename;
    if (path.extension() != ".mps")
    {
        path.replace_extension(".mps");
    }

    auto proto_or = operations_research::math_opt::ReadMpsFile(path.string());
    if (!proto_or.ok())
    {
        throw GenericSolverException("Failed to read MPS file: "
                                     + std::string(proto_or.status().message()));
    }

    auto model_or = math_opt::Model::FromModelProto(*proto_or);
    if (!model_or.ok())
    {
        throw GenericSolverException("Failed to build model from MPS: "
                                     + std::string(model_or.status().message()));
    }

    _model = std::move(*model_or);
    rebuild_from_model();
}

void SolverMathOpt::read_prob_lp(const std::filesystem::path& filename)
{
    auto path = filename;
    if (path.extension() != ".lp")
    {
        path.replace_extension(".lp");
    }

    std::ifstream ifs(path);
    if (!ifs)
    {
        throw GenericSolverException("Failed to open LP file: " + path.string());
    }
    std::string lp_data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    auto proto_or = operations_research::math_opt::ModelProtoFromLp(lp_data);
    if (!proto_or.ok())
    {
        throw GenericSolverException("Failed to parse LP file: "
                                     + std::string(proto_or.status().message()));
    }

    auto model_or = math_opt::Model::FromModelProto(*proto_or);
    if (!model_or.ok())
    {
        throw GenericSolverException("Failed to build model from LP: "
                                     + std::string(model_or.status().message()));
    }

    _model = std::move(*model_or);
    rebuild_from_model();
}

void SolverMathOpt::restore_prob(const std::filesystem::path& filename)
{
    read_prob_mps(filename);
}

void SolverMathOpt::read_basis(const std::filesystem::path& filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        throw GenericSolverException("read_basis: cannot open file " + filename.string());
    }

    auto int_to_status = [](int s) -> math_opt::BasisStatus
    {
        switch (s)
        {
        case 1:
            return math_opt::BasisStatus::kBasic;
        case 2:
            return math_opt::BasisStatus::kAtUpperBound;
        case 3:
            return math_opt::BasisStatus::kAtLowerBound;
        case 4:
            return math_opt::BasisStatus::kFixedValue;
        default:
            return math_opt::BasisStatus::kFree;
        }
    };

    std::string line;
    std::getline(ifs, line); // "MATHOPT_BASIS v1"

    math_opt::Basis basis;

    // Read column statuses
    int ncols = 0;
    ifs >> line >> ncols;    // "COLUMNS <n>"
    std::getline(ifs, line); // consume newline
    for (int i = 0; i < ncols && i < get_ncols(); ++i)
    {
        std::string name;
        int status;
        ifs >> name >> status;
        basis.variable_status[_variables[i]] = int_to_status(status);
    }

    // Read row statuses
    int nrows = 0;
    ifs >> line >> nrows;    // "ROWS <n>"
    std::getline(ifs, line); // consume newline
    for (int i = 0; i < nrows && i < get_nrows(); ++i)
    {
        std::string name;
        int status;
        ifs >> name >> status;
        basis.constraint_status[_constraints[i]] = int_to_status(status);
    }

    _initial_basis = std::move(basis);
}

void SolverMathOpt::set_basis(std::span<int> rstatus, std::span<int> cstatus)
{
    auto int_to_status = [](int s) -> math_opt::BasisStatus
    {
        switch (s)
        {
        case 1:
            return math_opt::BasisStatus::kBasic;
        case 2:
            return math_opt::BasisStatus::kAtUpperBound;
        case 3:
            return math_opt::BasisStatus::kAtLowerBound;
        case 4:
            return math_opt::BasisStatus::kFixedValue;
        default:
            return math_opt::BasisStatus::kFree;
        }
    };

    math_opt::Basis basis;
    for (size_t i = 0; i < cstatus.size() && i < _variables.size(); ++i)
    {
        basis.variable_status[_variables[i]] = int_to_status(cstatus[i]);
    }
    for (size_t i = 0; i < rstatus.size() && i < _constraints.size(); ++i)
    {
        basis.constraint_status[_constraints[i]] = int_to_status(rstatus[i]);
    }

    _initial_basis = std::move(basis);
}

/*************************************************************************************************
-----------------------    Get general informations about problem
----------------------------
*************************************************************************************************/

int SolverMathOpt::get_ncols() const
{
    return static_cast<int>(_variables.size());
}

int SolverMathOpt::get_nrows() const
{
    return static_cast<int>(_constraints.size());
}

int SolverMathOpt::get_nelems() const
{
    int count = 0;
    for (const auto& ct: _constraints)
    {
        count += static_cast<int>(_model->RowNonzeros(ct).size());
    }
    return count;
}

int SolverMathOpt::get_n_integer_vars() const
{
    int count = 0;
    for (const auto& var: _variables)
    {
        if (_model->is_integer(var))
        {
            ++count;
        }
    }
    return count;
}

void SolverMathOpt::get_obj(double* obj, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        obj[i - first] = _model->objective_coefficient(_variables[i]);
    }
}

void SolverMathOpt::set_obj_to_zero()
{
    for (const auto& var: _variables)
    {
        _model->set_objective_coefficient(var, 0.0);
    }
}

void SolverMathOpt::set_obj(const double* obj, int first, int last)
{
    for (int i = first; i <= last; ++i)
    {
        _model->set_objective_coefficient(_variables[i], obj[i - first]);
    }
}

void SolverMathOpt::get_rows(int* mstart,
                             int* mclind,
                             double* dmatval,
                             int size,
                             int* nels,
                             int first,
                             int last) const
{
    int offset = 0;
    int total_nels = 0;
    for (int r = first; r <= last; ++r)
    {
        mstart[r - first] = offset;
        const auto& vars = _model->RowNonzeros(_constraints[r]);
        for (const auto& var: vars)
        {
            if (offset < size)
            {
                double coeff = _model->coefficient(_constraints[r], var);
                // Find variable index
                for (int j = 0; j < static_cast<int>(_variables.size()); ++j)
                {
                    if (var.id() == _variables[j].id())
                    {
                        mclind[offset] = j;
                        dmatval[offset] = coeff;
                        break;
                    }
                }
                ++offset;
            }
        }
        total_nels += static_cast<int>(vars.size());
    }
    mstart[last - first + 1] = offset;
    *nels = total_nels;
}

void SolverMathOpt::get_row_type(char* qrtype, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        qrtype[i - first] = _row_types[i];
    }
}

void SolverMathOpt::get_rhs(double* rhs, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        double lb = _model->lower_bound(_constraints[i]);
        double ub = _model->upper_bound(_constraints[i]);
        char type = _row_types[i];

        if (type == 'E')
        {
            rhs[i - first] = lb;
        }
        else if (type == 'L')
        {
            rhs[i - first] = ub;
        }
        else if (type == 'G')
        {
            rhs[i - first] = lb;
        }
        else if (type == 'R')
        {
            rhs[i - first] = ub;
        }
        else
        {
            rhs[i - first] = 0.0;
        }
    }
}

void SolverMathOpt::get_rhs_range(double* range, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        if (_row_types[i] == 'R')
        {
            double lb = _model->lower_bound(_constraints[i]);
            double ub = _model->upper_bound(_constraints[i]);
            range[i - first] = ub - lb;
        }
        else
        {
            range[i - first] = 0.0;
        }
    }
}

void SolverMathOpt::get_cols(int* mstart,
                             int* mrwind,
                             double* dmatval,
                             int size,
                             int* nels,
                             int first,
                             int last) const
{
    int offset = 0;
    int total_nels = 0;
    for (int c = first; c <= last; ++c)
    {
        mstart[c - first] = offset;
        const auto& cts = _model->ColumnNonzeros(_variables[c]);
        for (const auto& ct: cts)
        {
            if (offset < size)
            {
                double coeff = _model->coefficient(ct, _variables[c]);
                // Find constraint index
                for (int r = 0; r < static_cast<int>(_constraints.size()); ++r)
                {
                    if (ct.id() == _constraints[r].id())
                    {
                        mrwind[offset] = r;
                        dmatval[offset] = coeff;
                        break;
                    }
                }
                ++offset;
            }
        }
        total_nels += static_cast<int>(cts.size());
    }
    mstart[last - first + 1] = offset;
    *nels = total_nels;
}

void SolverMathOpt::get_col_type(char* coltype, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        if (_model->is_integer(_variables[i]))
        {
            double lb = _model->lower_bound(_variables[i]);
            double ub = _model->upper_bound(_variables[i]);
            if (lb == 0.0 && ub == 1.0)
            {
                coltype[i - first] = 'B';
            }
            else
            {
                coltype[i - first] = 'I';
            }
        }
        else
        {
            coltype[i - first] = 'C';
        }
    }
}

void SolverMathOpt::get_lb(double* lb, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        lb[i - first] = _model->lower_bound(_variables[i]);
    }
}

void SolverMathOpt::get_ub(double* ub, int first, int last) const
{
    for (int i = first; i <= last; ++i)
    {
        ub[i - first] = _model->upper_bound(_variables[i]);
    }
}

int SolverMathOpt::get_row_index(const std::string& name)
{
    auto it = _row_name_to_index.find(name);
    return (it != _row_name_to_index.end()) ? it->second : -1;
}

int SolverMathOpt::get_col_index(const std::string& name)
{
    auto it = _col_name_to_index.find(name);
    return (it != _col_name_to_index.end()) ? it->second : -1;
}

std::vector<std::string> SolverMathOpt::get_row_names(int first, int last) const
{
    std::vector<std::string> names;
    names.reserve(last - first + 1);
    for (int i = first; i <= last; ++i)
    {
        names.emplace_back(_model->name(_constraints[i]));
    }
    return names;
}

std::vector<std::string> SolverMathOpt::get_row_names()
{
    return get_row_names(0, get_nrows() - 1);
}

std::vector<std::string> SolverMathOpt::get_col_names(int first, int last) const
{
    std::vector<std::string> names;
    names.reserve(last - first + 1);
    for (int i = first; i <= last; ++i)
    {
        names.emplace_back(_model->name(_variables[i]));
    }
    return names;
}

std::vector<std::string> SolverMathOpt::get_col_names()
{
    return get_col_names(0, get_ncols() - 1);
}

/*************************************************************************************************
------------------------------    Methods to modify problem
----------------------------------
*************************************************************************************************/

void SolverMathOpt::del_rows(int first, int last)
{
    for (int i = last; i >= first; --i)
    {
        _model->DeleteLinearConstraint(_constraints[i]);
    }
    _constraints.erase(_constraints.begin() + first, _constraints.begin() + last + 1);
    _row_types.erase(_row_types.begin() + first, _row_types.begin() + last + 1);
    rebuild_index_maps();
}

void SolverMathOpt::del_cols(int first, int last)
{
    for (int i = last; i >= first; --i)
    {
        _model->DeleteVariable(_variables[i]);
    }
    _variables.erase(_variables.begin() + first, _variables.begin() + last + 1);
    rebuild_index_maps();
}

void SolverMathOpt::add_rows(int newrows,
                             int newnz,
                             const char* qrtype,
                             const double* rhs,
                             const double* range,
                             const int* mstart,
                             const int* mclind,
                             const double* dmatval,
                             const std::vector<std::string>& row_names)
{
    for (int i = 0; i < newrows; ++i)
    {
        int start = mstart[i];
        int end = (i + 1 < newrows) ? mstart[i + 1] : newnz;

        // Determine bounds from row type
        double lb, ub;
        switch (qrtype[i])
        {
        case 'L':
            lb = -kInf;
            ub = rhs[i];
            break;
        case 'G':
            lb = rhs[i];
            ub = kInf;
            break;
        case 'E':
            lb = rhs[i];
            ub = rhs[i];
            break;
        case 'R':
            ub = rhs[i];
            lb = rhs[i] - (range ? range[i] : 0.0);
            break;
        default:
        {
            std::stringstream buffer;
            buffer << "add_rows: unknown row type '" << qrtype[i] << "' for row " << i;
            throw GenericSolverException(buffer.str());
        }
        }

        std::string name = (i < static_cast<int>(row_names.size())) ? row_names[i] : "";
        auto ct = _model->AddLinearConstraint(lb, ub, name);

        // Set coefficients
        for (int k = start; k < end; ++k)
        {
            _model->set_coefficient(ct, _variables[mclind[k]], dmatval[k]);
        }

        _constraints.push_back(ct);
        _row_types.push_back(qrtype[i]);

        if (!name.empty())
        {
            _row_name_to_index[name] = static_cast<int>(_constraints.size()) - 1;
        }
    }
}

void SolverMathOpt::add_cols(int newcol,
                             int newnz,
                             const double* objx,
                             const int* mstart,
                             const int* mrwind,
                             const double* dmatval,
                             const double* bdl,
                             const double* bdu,
                             const std::vector<std::string>& col_names)
{
    for (int i = 0; i < newcol; ++i)
    {
        std::string name = (i < static_cast<int>(col_names.size())) ? col_names[i] : "";
        auto var = _model->AddContinuousVariable(bdl[i], bdu[i], name);
        _model->set_objective_coefficient(var, objx[i]);

        // Set column coefficients in existing constraints
        int start = mstart[i];
        int end = (i + 1 < newcol) ? mstart[i + 1] : newnz;
        for (int k = start; k < end; ++k)
        {
            _model->set_coefficient(_constraints[mrwind[k]], var, dmatval[k]);
        }

        _variables.push_back(var);
        if (!name.empty())
        {
            _col_name_to_index[name] = static_cast<int>(_variables.size()) - 1;
        }
    }
}

void SolverMathOpt::add_name(int type, const char* cnames, int indice)
{
    throw NotImplementedFeatureSolverException("add_name is not supported for MATHOPT solver");
}

void SolverMathOpt::add_names(int type, const std::vector<std::string>& cnames, int first, int end)
{
    throw NotImplementedFeatureSolverException("add_names is not supported for MATHOPT solver");
}

void SolverMathOpt::chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj)
{
    assert(obj.size() == mindex.size());
    for (size_t i = 0; i < mindex.size(); ++i)
    {
        _model->set_objective_coefficient(_variables[mindex[i]], obj[i]);
    }
}

void SolverMathOpt::chg_obj_direction(bool minimize)
{
    _minimize = minimize;
    if (minimize)
    {
        _model->set_minimize();
    }
    else
    {
        _model->set_maximize();
    }
}

void SolverMathOpt::chg_bounds(const std::vector<int>& mindex,
                               const std::vector<char>& qbtype,
                               const std::vector<double>& bnd)
{
    assert(qbtype.size() == mindex.size());
    assert(bnd.size() == mindex.size());
    for (size_t i = 0; i < mindex.size(); ++i)
    {
        auto& var = _variables[mindex[i]];
        switch (qbtype[i])
        {
        case 'L':
            _model->set_lower_bound(var, bnd[i]);
            break;
        case 'U':
            _model->set_upper_bound(var, bnd[i]);
            break;
        case 'B':
            _model->set_lower_bound(var, bnd[i]);
            _model->set_upper_bound(var, bnd[i]);
            break;
        default:
            throw InvalidBoundTypeException(qbtype[i], LOGLOCATION);
        }
    }
}

void SolverMathOpt::chg_col_type(const std::vector<int>& mindex, const std::vector<char>& qctype)
{
    assert(qctype.size() == mindex.size());
    for (size_t i = 0; i < mindex.size(); ++i)
    {
        auto& var = _variables[mindex[i]];
        switch (qctype[i])
        {
        case 'C':
            _model->set_continuous(var);
            break;
        case 'I':
            _model->set_integer(var);
            break;
        case 'B':
            _model->set_integer(var);
            _model->set_lower_bound(var, 0.0);
            _model->set_upper_bound(var, 1.0);
            break;
        default:
            throw InvalidColTypeException(qctype[i], LOGLOCATION);
        }
    }
}

void SolverMathOpt::chg_rhs(int id_row, double val)
{
    char type = _row_types[id_row];
    auto& ct = _constraints[id_row];

    switch (type)
    {
    case 'E':
        _model->set_lower_bound(ct, val);
        _model->set_upper_bound(ct, val);
        break;
    case 'L':
        _model->set_upper_bound(ct, val);
        break;
    case 'G':
        _model->set_lower_bound(ct, val);
        break;
    case 'R':
    {
        double old_ub = _model->upper_bound(ct);
        double old_lb = _model->lower_bound(ct);
        double range = old_ub - old_lb;
        _model->set_upper_bound(ct, val);
        _model->set_lower_bound(ct, val - range);
        break;
    }
    default:
    {
        std::stringstream buffer;
        buffer << "chg_rhs: unconstrained constraint " << id_row;
        throw GenericSolverException(buffer.str());
    }
    }
}

void SolverMathOpt::chg_coef(int id_row, int id_col, double val)
{
    _model->set_coefficient(_constraints[id_row], _variables[id_col], val);
}

void SolverMathOpt::chg_coefs(const std::vector<int>& id_rows,
                              const std::vector<int>& id_cols,
                              const std::vector<double>& vals)
{
    for (size_t i = 0; i < id_rows.size(); ++i)
    {
        chg_coef(id_rows[i], id_cols[i], vals[i]);
    }
}

void SolverMathOpt::chg_rhs_values(std::vector<int>& rhs_indices, std::vector<double>& rhs_values)
{
    for (size_t i = 0; i < rhs_indices.size(); ++i)
    {
        chg_rhs(rhs_indices[i], rhs_values[i]);
    }
}

void SolverMathOpt::chg_row_name(int id_row, const std::string& name)
{
    // Remove old name from map
    std::string old_name(_model->name(_constraints[id_row]));
    if (!old_name.empty())
    {
        _row_name_to_index.erase(old_name);
    }

    // MathOpt doesn't support renaming after creation.
    // We delete and re-create the constraint with the new name.
    auto& ct = _constraints[id_row];
    double lb = _model->lower_bound(ct);
    double ub = _model->upper_bound(ct);

    // Save coefficients
    std::vector<std::pair<math_opt::Variable, double>> coeffs;
    for (const auto& var: _model->RowNonzeros(ct))
    {
        coeffs.emplace_back(var, _model->coefficient(ct, var));
    }

    _model->DeleteLinearConstraint(ct);
    auto new_ct = _model->AddLinearConstraint(lb, ub, name);
    for (const auto& [var, coeff]: coeffs)
    {
        _model->set_coefficient(new_ct, var, coeff);
    }
    _constraints[id_row] = new_ct;

    if (!name.empty())
    {
        _row_name_to_index[name] = id_row;
    }
}

void SolverMathOpt::chg_col_name(int id_col, const std::string& name)
{
    // Remove old name from map
    std::string old_name(_model->name(_variables[id_col]));
    if (!old_name.empty())
    {
        _col_name_to_index.erase(old_name);
    }

    // MathOpt doesn't support renaming after creation.
    // We delete and re-create the variable with the new name.
    auto& var = _variables[id_col];
    double lb = _model->lower_bound(var);
    double ub = _model->upper_bound(var);
    bool is_int = _model->is_integer(var);
    double obj_coeff = _model->objective_coefficient(var);

    // Save column coefficients
    std::vector<std::pair<math_opt::LinearConstraint, double>> coeffs;
    for (const auto& ct: _model->ColumnNonzeros(var))
    {
        coeffs.emplace_back(ct, _model->coefficient(ct, var));
    }

    _model->DeleteVariable(var);
    math_opt::Variable new_var = is_int ? _model->AddIntegerVariable(lb, ub, name)
                                        : _model->AddContinuousVariable(lb, ub, name);
    _model->set_objective_coefficient(new_var, obj_coeff);

    for (const auto& [ct, coeff]: coeffs)
    {
        _model->set_coefficient(ct, new_var, coeff);
    }
    _variables[id_col] = new_var;

    if (!name.empty())
    {
        _col_name_to_index[name] = id_col;
    }
}

/*************************************************************************************************
-----------------------------    Methods to solve the problem
---------------------------------
*************************************************************************************************/

int SolverMathOpt::solve_lp()
{
    math_opt::SolveArguments args;
    args.parameters.threads = _threads;
    if (_iteration_limit.has_value())
    {
        args.parameters.iteration_limit = *_iteration_limit;
    }
    if (_log_level > 0 && _log_stream.is_open())
    {
        args.message_callback = math_opt::PrinterMessageCallback(_log_stream);
    }
    else
    {
        args.parameters.enable_output = (_log_level > 0);
    }

    // Pass warm-start basis if one was set via set_basis() or read_basis().
    // Consumed on use so subsequent solves start fresh unless re-set.
    if (_initial_basis.has_value())
    {
        args.model_parameters.initial_basis = _initial_basis;
        _initial_basis.reset();
    }

    auto result = math_opt::Solve(*_model, math_opt::SolverType::kGlop, args);
    if (!result.ok())
    {
        std::stringstream buffer;
        buffer << "MathOpt solve failed: " << result.status();
        for (auto* stream: get_stream())
        {
            *stream << buffer.str() << std::endl;
        }
        return UNKNOWN;
    }

    _last_result = std::move(*result);

    auto reason = _last_result->termination.reason;
    using TR = math_opt::TerminationReason;

    if (reason == TR::kOptimal)
    {
        return OPTIMAL;
    }
    else if (reason == TR::kInfeasible)
    {
        return INFEASIBLE;
    }
    else if (reason == TR::kUnbounded || reason == TR::kInfeasibleOrUnbounded)
    {
        return UNBOUNDED;
    }
    else
    {
        return UNKNOWN;
    }
}

int SolverMathOpt::solve_mip()
{
    math_opt::SolveArguments args;
    args.parameters.threads = _threads;
    if (_iteration_limit.has_value())
    {
        args.parameters.iteration_limit = *_iteration_limit;
    }
    if (_log_level > 0 && _log_stream.is_open())
    {
        args.message_callback = math_opt::PrinterMessageCallback(_log_stream);
    }
    else
    {
        args.parameters.enable_output = (_log_level > 0);
    }

    auto result = math_opt::Solve(*_model, math_opt::SolverType::kGscip, args);
    if (!result.ok())
    {
        std::stringstream buffer;
        buffer << "MathOpt MIP solve failed: " << result.status();
        for (auto* stream: get_stream())
        {
            *stream << buffer.str() << std::endl;
        }
        return UNKNOWN;
    }

    _last_result = std::move(*result);

    auto reason = _last_result->termination.reason;
    using TR = math_opt::TerminationReason;

    if (reason == TR::kOptimal)
    {
        return OPTIMAL;
    }
    else if (reason == TR::kInfeasible)
    {
        return INFEASIBLE;
    }
    else if (reason == TR::kUnbounded || reason == TR::kInfeasibleOrUnbounded)
    {
        return UNBOUNDED;
    }
    else
    {
        return UNKNOWN;
    }
}

/*************************************************************************************************
-------------------------    Methods to get solutions information
-----------------------------
*************************************************************************************************/

void SolverMathOpt::get_basis(int* rstatus, int* cstatus) const
{
    if (!_last_result.has_value() || _last_result->solutions.empty()
        || !_last_result->solutions[0].basis.has_value())
    {
        // No basis available — fill with zeros
        if (rstatus)
        {
            std::fill_n(rstatus, get_nrows(), 0);
        }
        if (cstatus)
        {
            std::fill_n(cstatus, get_ncols(), 0);
        }
        return;
    }

    const auto& basis = *_last_result->solutions[0].basis;
    using BS = math_opt::BasisStatus;

    auto map_status = [](BS s) -> int
    {
        switch (s)
        {
        case BS::kFree:
            return 0;
        case BS::kBasic:
            return 1;
        case BS::kAtUpperBound:
            return 2;
        case BS::kAtLowerBound:
            return 3;
        case BS::kFixedValue:
            return 4;
        default:
            return 0;
        }
    };

    if (cstatus)
    {
        for (int i = 0; i < get_ncols(); ++i)
        {
            auto it = basis.variable_status.find(_variables[i]);
            cstatus[i] = (it != basis.variable_status.end()) ? map_status(it->second) : 0;
        }
    }

    if (rstatus)
    {
        for (int i = 0; i < get_nrows(); ++i)
        {
            auto it = basis.constraint_status.find(_constraints[i]);
            rstatus[i] = (it != basis.constraint_status.end()) ? map_status(it->second) : 0;
        }
    }
}

double SolverMathOpt::get_mip_value() const
{
    if (!_last_result.has_value())
    {
        throw GenericSolverException("get_mip_value: no solution available (solve not called)");
    }
    // Use primal_bound instead of objective_value() because objective_value()
    // CHECK-fails when no primal feasible solution exists (e.g. INFEASIBLE).
    // primal_bound is always safe and equals objective_value() on OPTIMAL.
    return _last_result->termination.objective_bounds.primal_bound;
}

double SolverMathOpt::get_lp_value() const
{
    if (!_last_result.has_value())
    {
        throw GenericSolverException("get_lp_value: no solution available (solve not called)");
    }
    // Use primal_bound instead of objective_value() because objective_value()
    // CHECK-fails when no primal feasible solution exists (e.g. INFEASIBLE).
    // primal_bound is always safe and equals objective_value() on OPTIMAL.
    return _last_result->termination.objective_bounds.primal_bound;
}

int SolverMathOpt::get_splex_num_of_ite_last() const
{
    if (!_last_result.has_value())
    {
        return 0;
    }
    return static_cast<int>(_last_result->solve_stats.simplex_iterations);
}

void SolverMathOpt::get_lp_sol(double* primals, double* duals, double* reduced_costs) const
{
    if (!_last_result.has_value())
    {
        throw GenericSolverException("get_lp_sol: no solution available");
    }

    if (primals)
    {
        const auto& vals = _last_result->variable_values();
        for (int i = 0; i < get_ncols(); ++i)
        {
            auto it = vals.find(_variables[i]);
            primals[i] = (it != vals.end()) ? it->second : 0.0;
        }
    }

    if (duals)
    {
        if (_last_result->has_dual_feasible_solution())
        {
            const auto& dvals = _last_result->dual_values();
            for (int i = 0; i < get_nrows(); ++i)
            {
                auto it = dvals.find(_constraints[i]);
                duals[i] = (it != dvals.end()) ? it->second : 0.0;
            }
        }
        else
        {
            // When we don't have a dual feasible solution do we return vector of 01
            std::fill_n(duals, get_nrows(), 0.0);
        }
    }

    if (reduced_costs)
    {
        if (_last_result->has_dual_feasible_solution())
        {
            const auto& rc = _last_result->reduced_costs();
            for (int i = 0; i < get_ncols(); ++i)
            {
                auto it = rc.find(_variables[i]);
                reduced_costs[i] = (it != rc.end()) ? it->second : 0.0;
            }
        }
        else
        {
            std::fill_n(reduced_costs, get_ncols(), 0.0);
        }
    }
}

void SolverMathOpt::get_mip_sol(double* primals)
{
    if (!_last_result.has_value())
    {
        throw GenericSolverException("get_mip_sol: no solution available");
    }

    const auto& vals = _last_result->variable_values();
    for (int i = 0; i < get_ncols(); ++i)
    {
        auto it = vals.find(_variables[i]);
        primals[i] = (it != vals.end()) ? it->second : 0.0;
    }
}

void SolverMathOpt::get_presolve_map(int* rowmap, int* colmap) const
{
    throw NotImplementedFeatureSolverException(
      "get_presolve_map is not supported for MATHOPT solver");
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels
---------------------------
*************************************************************************************************/

void SolverMathOpt::set_output_log_level(int loglevel)
{
    _log_level = loglevel;
}

void SolverMathOpt::set_algorithm(const std::string& algo)
{
    // Glop only supports simplex variants — accept DUAL silently, reject others
    if (algo != "DUAL" && algo != "dual")
    {
        throw InvalidSolverOptionException("set_algorithm: " + algo, LOGLOCATION);
    }
}

void SolverMathOpt::set_threads(int n_threads)
{
    _threads = n_threads;
}

void SolverMathOpt::set_optimality_gap(double gap)
{
    // LP solver — gap not applicable
}

void SolverMathOpt::set_simplex_iter(int iter)
{
    _iteration_limit = iter;
}

void SolverMathOpt::mark_indices_to_keep_presolve(int, int, int*, int*)
{
    throw NotImplementedFeatureSolverException(
      "mark_indices_to_keep_presolve is not supported for MATHOPT solver");
}

void SolverMathOpt::presolve_only()
{
    throw NotImplementedFeatureSolverException("presolve_only is not supported for MATHOPT solver");
}

/*************************************************************************************************
-----------------------------------    Internal helpers
--------------------------------
*************************************************************************************************/

void SolverMathOpt::rebuild_from_model()
{
    _last_result.reset();
    _initial_basis.reset();
    _variables.clear();
    _constraints.clear();
    _row_types.clear();

    for (const auto& var: _model->SortedVariables())
    {
        _variables.push_back(var);
    }

    // MathOpt stores constraints as lb <= expr <= ub with no row type.
    // SolverAbstract uses row type chars (L/G/E/R) that get_rhs(), chg_rhs(),
    // and get_row_type() depend on, so we derive and cache them from bounds.
    for (const auto& ct: _model->SortedLinearConstraints())
    {
        _constraints.push_back(ct);

        double lb = _model->lower_bound(ct);
        double ub = _model->upper_bound(ct);
        if (lb == ub)
        {
            _row_types.push_back('E');
        }
        else if (lb <= -kInf)
        {
            _row_types.push_back('L');
        }
        else if (ub >= kInf)
        {
            _row_types.push_back('G');
        }
        else
        {
            _row_types.push_back('R');
        }
    }

    _minimize = !_model->is_maximize();

    rebuild_index_maps();
}

void SolverMathOpt::rebuild_index_maps()
{
    _col_name_to_index.clear();
    for (int i = 0; i < static_cast<int>(_variables.size()); ++i)
    {
        std::string n(_model->name(_variables[i]));
        if (!n.empty())
        {
            _col_name_to_index[n] = i;
        }
    }

    _row_name_to_index.clear();
    for (int i = 0; i < static_cast<int>(_constraints.size()); ++i)
    {
        std::string n(_model->name(_constraints[i]));
        if (!n.empty())
        {
            _row_name_to_index[n] = i;
        }
    }
}
