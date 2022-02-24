#pragma once

#include "SlaveCut.h"
#include "Worker.h"

/*!
 * \class WorkerMasterData
 * \brief Class use to store trace information during the algorithm run
 */
class WorkerMasterData {
 public:
  WorkerMasterData() = default;

  bool _valid = false;
  double _lb;
  double _ub;
  double _bestub;
  int _deleted_cut;
  int _nbasis;
  double _time;
  PointPtr _x0;
  PointPtr _min_invest;
  PointPtr _max_invest;
  std::map<std::string, SlaveCutDataPtr> _cut_trace;

  double _invest_cost;
  double _operational_cost;

  Point get_point() const;
  Point get_min_invest() const;
  Point get_max_invest() const;
};

using WorkerMasterDataPtr = std::shared_ptr<WorkerMasterData>;
using BendersTrace = std::vector<WorkerMasterDataPtr>;

LogData defineLogDataFromBendersDataAndTrace(const BendersData& data,
                                             const BendersTrace& trace);
