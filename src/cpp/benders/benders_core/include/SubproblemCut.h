#pragma once

#include <boost/serialization/map.hpp>

#include "Worker.h"
#include "common.h"

struct SubProblemData {
  double subproblem_cost;
  Point var_name_and_subgradient;
  double subproblem_timer;
  int simplex_iter;
  int lpstatus;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &subproblem_cost;
    ar &var_name_and_subgradient;
    ar &subproblem_timer;
    ar &simplex_iter;
    ar &lpstatus;
  }
};
using SubProblemDataMap = std::map<std::string, SubProblemData>;

typedef std::pair<Point, IntVector> SubproblemCutData1;
typedef std::pair<SubproblemCutData1, DblVector> SubproblemCutData2;
typedef std::pair<SubproblemCutData2, StrVector> SubproblemCutData3;
typedef SubproblemCutData3 SubproblemCutData;

typedef std::shared_ptr<SubproblemCutData> SubproblemCutDataPtr;

typedef std::map<std::string, SubproblemCutData> SubproblemCutPackage;
typedef std::vector<SubproblemCutPackage> AllCutPackage;

class SubproblemCutTrimmer;

class SubproblemCutDataHandler;
typedef std::shared_ptr<SubproblemCutDataHandler> SubproblemCutDataHandlerPtr;

typedef std::vector<std::tuple<Point, double, double>> DynamicAggregateCuts;

typedef std::set<SubproblemCutDataHandlerPtr, Predicate>
    SubproblemCutDataHandlerPtrSet;

void BuildSubproblemCutData(SubproblemCutData &);

const int MAXINTEGER = 2;
enum SubproblemCutInt { SIMPLEXITER = 0, LPSTATUS };

const int MAXDBL = 2;
enum SubproblemCutDbl { SUBPROBLEM_COST = 0, ALPHA_I, SUBPROBLEM_TIMER };
enum SubproblemCutStr { MAXSTR = 0 };

class SubproblemCutDataHandler {
 public:
  Point &get_subgradient();
  IntVector &get_int();
  DblVector &get_dbl();
  StrVector &get_str();

  int &get_int(SubproblemCutInt);
  double &get_dbl(SubproblemCutDbl);
  std::string &get_str(SubproblemCutStr);

  int get_int(SubproblemCutInt) const;
  double get_dbl(SubproblemCutDbl) const;
  std::string const &get_str(SubproblemCutStr) const;

  Point const &get_subgradient() const;
  IntVector const &get_int() const;
  DblVector const &get_dbl() const;
  StrVector const &get_str() const;
  void print(std::ostream &stream) const;

 public:
  explicit SubproblemCutDataHandler(SubproblemCutDataPtr const &data);
  explicit SubproblemCutDataHandler(SubproblemCutDataPtr &data);
  virtual ~SubproblemCutDataHandler() = default;

  SubproblemCutDataPtr _data;
};

std::ostream &operator<<(std::ostream &stream, SubproblemCutData const &rhs);
