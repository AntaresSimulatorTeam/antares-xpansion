#pragma once

#ifdef _MSC_VER
#pragma warning(disable : 4267)  // implicit conversion, possible loss of data
#endif

#include <json/reader.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
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

enum class MasterFormulation { INTEGER, RELAXED };
enum class SOLVER { BENDERS, OUTER_LOOP, MERGE_MPS };

struct Predicate;
typedef std::map<std::string, double> Point;

typedef std::shared_ptr<Point> PointPtr;

double const EPSILON_PREDICATE = 1e-8;

typedef std::set<std::string> problem_names;
typedef std::map<std::string, int> VariableMap;
typedef std::map<int, std::string> Int2Str;
typedef std::map<std::string, double> Str2Dbl;
typedef std::vector<int> IntVector;
typedef std::vector<char> CharVector;
typedef std::vector<double> DblVector;
typedef std::vector<std::string> StrVector;
typedef std::map<std::string, VariableMap> CouplingMap;

typedef std::map<std::string, IntVector> SlaveCutId;
typedef std::tuple<int, std::string, int, bool> ActiveCut;
typedef std::vector<ActiveCut> ActiveCutStorage;

typedef std::pair<std::string, std::string> mps_coupling;
typedef std::list<mps_coupling> mps_coupling_list;

enum class BENDERSMETHOD {
  BENDERS,
  BENDERS_BY_BATCH,
  BENDERS_OUTERLOOP,
  BENDERS_BY_BATCH_OUTERLOOP
};

inline std::string bendersmethod_to_string(BENDERSMETHOD method) {
  switch (method) {
    case BENDERSMETHOD::BENDERS:
      return "Benders";
    case BENDERSMETHOD::BENDERS_BY_BATCH:
      return "Benders by batch";
    case BENDERSMETHOD::BENDERS_OUTERLOOP:
      return "Outerloop around Benders";
    case BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP:
      return "Outerloop around Benders by batch";
    default:
      return "Unknown";
  }
}

struct Predicate {
  bool operator()(PointPtr const &lhs, PointPtr const &rhs) const {
    return *lhs < *rhs;
  }
  bool operator()(Point const &lhs, Point const &rhs) const {
    Point::const_iterator it1(lhs.begin());
    Point::const_iterator it2(rhs.begin());

    Point::const_iterator end1(lhs.end());
    Point::const_iterator end2(rhs.end());

    while (it1 != end1 && it2 != end2) {
      if (it1->first != it2->first) {
        return it1->first < it2->first;
      } else {
        if (std::fabs(it1->second - it2->second) < EPSILON_PREDICATE) {
          ++it1;
          ++it2;
        } else {
          return it1->second < it2->second;
        }
      }
    }

    if (it1 == end1 && it2 == end2) {
      return false;
    } else {
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
inline std::ostream &operator<<(std::ostream &stream, Point const &rhs) {
  for (auto const &kvp : rhs) {
    if (kvp.second > 0) {
      if (kvp.second == 1) {
        stream << "+";
        stream << kvp.first;
      } else {
        stream << "+";
        stream << kvp.second;
        stream << kvp.first;
      }
    } else if (kvp.second < 0) {
      stream << kvp.second;
      stream << kvp.first;
    }
  }
  return stream;
}

double norm_point(Point const &x0, Point const &x1);

std::ostream &operator<<(std::ostream &stream,
                         std::vector<IntVector> const &rhs);

const std::string SUBPROBLEM_WEIGHT_CST_STR("CONSTANT");
const std::string SUBPROBLEM_WEIGHT_UNIFORM_CST_STR("UNIFORM");
const std::string WEIGHT_SUM_CST_STR("WEIGHT_SUM");
const std::string MPS_SUFFIX = ".mps";

struct BaseOptions {
  std::string OUTPUTROOT;
  std::string INPUTROOT;
  std::string STRUCTURE_FILE;
  std::string LAST_ITERATION_JSON_FILE;
  std::string MASTER_NAME;
  std::string SOLVER_NAME;
  std::string SLAVE_WEIGHT;
  std::string AREA_FILE;

  int LOG_LEVEL = 0;

  double SLAVE_WEIGHT_VALUE = 0;
  bool RESUME = false;

  Str2Dbl weights;
};
typedef BaseOptions MergeMPSOptions;

struct ExternalLoopOptions {
  bool DO_OUTER_LOOP = false;
  std::string OUTER_LOOP_OPTION_FILE;
};

struct BendersBaseOptions : public BaseOptions {
  explicit BendersBaseOptions(const BaseOptions &base_to_copy)
      : BaseOptions(base_to_copy) {}

  int MAX_ITERATIONS = -1;

  double ABSOLUTE_GAP = 0;
  double RELATIVE_GAP = 0;
  double RELAXED_GAP = 0;
  double TIME_LIMIT = 0;
  double SEPARATION_PARAM = 1;

  bool AGGREGATION = false;
  bool TRACE = false;
  bool BOUND_ALPHA = false;

  MasterFormulation MASTER_FORMULATION;

  std::string CSV_NAME;
  std::string LAST_MASTER_MPS;
  std::string LAST_MASTER_BASIS;

  size_t BATCH_SIZE;
  ExternalLoopOptions EXTERNAL_LOOP_OPTIONS;
};

void usage(int argc);
CouplingMap build_input(const std::filesystem::path &structure_path);
Json::Value get_json_file_content(const std::filesystem::path &json_file);
