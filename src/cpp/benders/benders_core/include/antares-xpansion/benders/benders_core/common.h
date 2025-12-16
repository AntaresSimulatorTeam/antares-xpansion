#pragma once

#ifdef _MSC_VER
#pragma warning(disable : 4267) // implicit conversion, possible loss of data
#endif

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json/reader.h>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "antares-xpansion/core/ProblemFormat.h"

enum class MasterFormulation
{
    INTEGER,
    RELAXED
};

enum class SOLVER
{
    BENDERS,
    OUTER_LOOP,
    MERGE_MPS
};

struct Predicate;
typedef std::map<std::string, double> Point;

typedef std::shared_ptr<Point> PointPtr;

const double EPSILON_PREDICATE = 1e-8;

using problem_names = std::set<std::string>;
using VariableMap = std::map<std::string, int>;
using Int2Str = std::map<int, std::string>;
using Str2Dbl = std::map<std::string, double>;
using IntVector = std::vector<int>;
using CharVector = std::vector<char>;
using DblVector = std::vector<double>;
using StrVector = std::vector<std::string>;
using CouplingMap = std::map<std::string, VariableMap>;

using SlaveCutId = std::map<std::string, IntVector>;
using ActiveCut = std::tuple<int, std::string, int, bool>;
using ActiveCutStorage = std::vector<ActiveCut>;

using mps_coupling = std::pair<std::string, std::string>;
using mps_coupling_list = std::list<mps_coupling>;

using SubProblemNamesInCut = std::vector<std::pair<std::string, int>>;

struct Predicate
{
    bool operator()(const PointPtr& lhs, const PointPtr& rhs) const
    {
        return *lhs < *rhs;
    }

    bool operator()(const Point& lhs, const Point& rhs) const
    {
        Point::const_iterator it1(lhs.begin());
        Point::const_iterator it2(rhs.begin());

        Point::const_iterator end1(lhs.end());
        Point::const_iterator end2(rhs.end());

        while (it1 != end1 && it2 != end2)
        {
            if (it1->first != it2->first)
            {
                return it1->first < it2->first;
            }
            else
            {
                if (std::fabs(it1->second - it2->second) < EPSILON_PREDICATE)
                {
                    ++it1;
                    ++it2;
                }
                else
                {
                    return it1->second < it2->second;
                }
            }
        }

        if (it1 == end1 && it2 == end2)
        {
            return false;
        }
        else
        {
            return (it1 == end1);
        }
    }
};

/*!
 *  \brief Stream output overloading for point
 *
 *  \param stream : stream output
 *
 *  \param rhs : point
 */
inline std::ostream& operator<<(std::ostream& stream, const Point& rhs)
{
    for (const auto& kvp: rhs)
    {
        if (kvp.second > 0)
        {
            if (kvp.second == 1)
            {
                stream << "+";
                stream << kvp.first;
            }
            else
            {
                stream << "+";
                stream << kvp.second;
                stream << kvp.first;
            }
        }
        else if (kvp.second < 0)
        {
            stream << kvp.second;
            stream << kvp.first;
        }
    }
    return stream;
}

double norm_point(const Point& x0, const Point& x1);

std::ostream& operator<<(std::ostream& stream, const std::vector<IntVector>& rhs);

const std::string SUBPROBLEM_WEIGHT_CST_STR("CONSTANT");
const std::string SUBPROBLEM_WEIGHT_UNIFORM_CST_STR("UNIFORM");
const std::string WEIGHT_SUM_CST_STR("WEIGHT_SUM");
const std::string MPS_SUFFIX = ".mps";
const std::string SAVE_SUFFIX = ".svf";

struct BaseOptions
{
    int LOG_LEVEL = 0;

    std::string INPUTROOT;
    std::string OUTPUTROOT;
    std::string STRUCTURE_FILE;
    std::string MASTER_NAME;
    std::string SOLVER_NAME;

    ProblemsFormat PROBLEMS_FORMAT = ProblemsFormat::MPS_FILE;
};

struct PresolveOptions: public BaseOptions
{
    PresolveOptions() = default;

    explicit PresolveOptions(const BaseOptions& other):
        BaseOptions(other)
    {
    }

    bool KEEP_FULL;
    std::string FULL_DIR;
};

struct SolverBaseOptions: public BaseOptions
{
    SolverBaseOptions() = default;

    explicit SolverBaseOptions(const BaseOptions& other):
        BaseOptions(other)
    {
    }

    std::string SLAVE_WEIGHT;
    double SLAVE_WEIGHT_VALUE = 0;
    Str2Dbl weights;
};

typedef SolverBaseOptions MergeMPSOptions;

struct ExternalLoopOptions
{
    bool DO_OUTER_LOOP = false;
    std::string OUTER_LOOP_OPTION_FILE;
};

struct BendersBaseOptions: public SolverBaseOptions
{
    explicit BendersBaseOptions(const SolverBaseOptions& other):
        SolverBaseOptions(other)
    {
    }

    int MAX_ITERATIONS = -1;

    double ABSOLUTE_GAP = 0;
    double RELATIVE_GAP = 0;
    double RELAXED_GAP = 0;
    double TIME_LIMIT = 0;
    double SEPARATION_PARAM = 1;
    double MASTER_SOLUTION_TOLERANCE = 1e-4;
    double CUT_COEFFICIENT_TOLERANCE = 5e-3;

    bool RESUME = false;
    int AGGREGATION = 0;
    bool TRACE = false;
    bool BOUND_ALPHA = false;
    bool CACHE_PROBLEMS = false;

    MasterFormulation MASTER_FORMULATION;

    std::string AREA_FILE;
    std::string CSV_NAME;
    std::string LAST_MASTER_MPS;
    std::string LAST_MASTER_BASIS;
    std::string LAST_ITERATION_JSON_FILE;

    size_t BATCH_SIZE;

    ExternalLoopOptions EXTERNAL_LOOP_OPTIONS;
};

void usage(int argc);

Json::Value get_json_file_content(const std::filesystem::path& json_file);

bool mkdir(const std::filesystem::path& path_to_folder);

template<typename T>
concept OStreamable = requires(std::ostream& os, T obj) {
    { os << obj } -> std::same_as<std::ostream&>;
};

template<typename T>
concept OStreamableIntegral = OStreamable<T> && std::integral<T>;

template<OStreamable MPSPath, OStreamable CandidateName, OStreamableIntegral ColId>
void export_structure_file(const std::filesystem::path& output_path,
                           const std::map<MPSPath, std::map<CandidateName, ColId>>& structure)
{
    std::ofstream structure_file{output_path};
    for (const auto& [mps_file_path, candidates_name_and_colId]: structure)
    {
        for (const auto& [candidate_name, colId]: candidates_name_and_colId)
        {
            // Adding the space to make sure there is a demarkation even when the names might exceed
            // 50 characters
            structure_file << std::setw(50) << mps_file_path;
            structure_file << " " << std::setw(50) << candidate_name;
            structure_file << " " << std::setw(10) << colId;
            structure_file << std::endl;
        }
    }
    structure_file.close();
}
