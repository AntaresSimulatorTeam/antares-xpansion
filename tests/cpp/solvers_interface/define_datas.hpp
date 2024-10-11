#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

/* Contains all the data to check the results of the tests on an instance*/
class InstanceData {
 public:
  std::filesystem::path _path;
  int _ncols;
  int _nintegervars;
  int _nrows;
  int _nelems;
  std::vector<double> _obj;

  std::vector<double> _matval;
  std::vector<int> _mind;
  std::vector<int> _mstart;

  std::vector<double> _matval_delete;
  std::vector<int> _mind_delete;
  std::vector<int> _mstart_delete;

  std::vector<double> _rhs;
  std::vector<char> _coltypes;
  std::vector<char> _rowtypes;
  std::vector<double> _lb;
  std::vector<double> _ub;
  double _optval;
  std::vector<double> _primals;
  std::vector<double> _duals;
  std::vector<double> _slacks;
  std::vector<double> _reduced_costs;

  std::vector<std::string> _status;
  std::vector<int> _status_int;

  std::vector<std::string> _col_names;
  std::vector<std::string> _row_names;
};

enum INSTANCES {
  MIP_TOY,
  LP_TOY,
  MULTIKP,
  UNBD_PRB,
  INFEAS_PRB,
  NET_MASTER,
  NET_SP1,
  NET_SP2,
  SLACKS,
  REDUCED
};

typedef std::vector<InstanceData> AllDatas;

void fill_datas(AllDatas& datas);