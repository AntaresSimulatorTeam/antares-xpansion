#pragma once

#ifdef _MSC_VER
#pragma warning(disable : 4267)  // implicit conversion, possible loss of data
#endif

#include <algorithm>
#include <cmath>
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

const std::string SLAVE_WEIGHT_CST_STR("CONSTANT");
const std::string SLAVE_WEIGHT_UNIFORM_CST_STR("UNIFORM");
const std::string WEIGHT_SUM_CST_STR("WEIGHT_SUM");
const std::string MPS_SUFFIX = ".mps";

struct BaseOptions {
  std::string OUTPUTROOT;
  std::string INPUTROOT;
  std::string STRUCTURE_FILE;
  std::string MASTER_NAME;
  std::string SOLVER_NAME;
  std::string SLAVE_WEIGHT;

  int LOG_LEVEL;

  double SLAVE_WEIGHT_VALUE;

  Str2Dbl weights;
};
typedef BaseOptions MergeMPSOptions;
struct BendersBaseOptions : public BaseOptions {
  explicit BendersBaseOptions(const BaseOptions &base_to_copy)
      : BaseOptions(base_to_copy) {}

  int MAX_ITERATIONS;

  double ABSOLUTE_GAP;
  double RELATIVE_GAP;
  double TIME_LIMIT;

  bool AGGREGATION;
  bool TRACE;
  bool BOUND_ALPHA;

  std::string CSV_NAME;
  std::string LAST_MASTER_MPS;
};

void usage(int argc);
CouplingMap build_input(const std::string &structure_path,
                        const std::string &master_name);
