#pragma once

#include "Worker.h"
#include "common.h"

typedef std::pair<Point, IntVector> SubproblemCutData1;
typedef std::pair<SubproblemCutData1, DblVector> SubproblemCutData2;
typedef std::pair<SubproblemCutData2, StrVector> SubproblemCutData3;
typedef SubproblemCutData3 SubproblemCutData;

typedef std::shared_ptr<SubproblemCutData> SubproblemCutDataPtr;

typedef std::map<std::string, SubproblemCutData> SubproblemCutPackage;
typedef std::vector<SubproblemCutPackage> AllCutPackage;

class SubproblemCutTrimmer;
typedef std::set<SubproblemCutTrimmer> SubproblemCutStorage;
typedef std::map<std::string, SubproblemCutStorage> AllCutStorage;

class SubproblemCutDataHandler;
typedef std::shared_ptr<SubproblemCutDataHandler> SubproblemCutDataHandlerPtr;

typedef std::vector<std::tuple<Point, double, double>> DynamicAggregateCuts;

typedef std::set<SubproblemCutDataHandlerPtr, Predicate>
    SubproblemCutDataHandlerPtrSet;

void BuildSubproblemCutData(SubproblemCutData &);

enum SubproblemCutInt { SIMPLEXITER = 0, LPSTATUS, MAXINTEGER };

enum SubproblemCutDbl { SUBPROBLEM_COST = 0, ALPHA_I,
  SUBPROBLEM_TIMER, MAXDBL };

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
  virtual ~SubproblemCutDataHandler();

  SubproblemCutDataPtr _data;
};

class SubproblemCutTrimmer {
 public:
  SubproblemCutDataHandlerPtr _data_cut;
  Point _x0;
  double _const_cut;

  SubproblemCutTrimmer(SubproblemCutDataHandlerPtr &data, Point &x0);
  Point const &get_subgradient() const;

  bool operator<(SubproblemCutTrimmer const &other) const;

  void print(std::ostream &stream) const;
};

std::ostream &operator<<(std::ostream &stream, SubproblemCutTrimmer const &rhs);

std::ostream &operator<<(std::ostream &stream, SubproblemCutData const &rhs);
