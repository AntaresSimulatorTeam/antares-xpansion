#include "BendersStructsDatas.h"

/*!
 * \brief Get point
 */
Point WorkerMasterData::get_x_cut() const { return *_x_cut; }

Point WorkerMasterData::get_min_invest() const { return *_min_invest; }

Point WorkerMasterData::get_max_invest() const { return *_max_invest; }

MathLoggerData MathLoggerDataFromCurrentIterationData(
    const CurrentIterationData& data) {
  return {/*.iteration =*/data.it,
          /*.lower_bound =*/data.lb,
          /*.upper_bound =*/data.ub,
          /*.best_upper_bound =*/data.best_ub,
          /*.optimality_gap =*/0,
          /*.relative_gap =*/0,
          /*.max_simplexiter =*/0,
          /*.min_simplexiter =*/0,
          /*.deletedcut =*/data.deletedcut,
          /*.time_master =*/data.timer_master,
          /*.time_subproblems =*/data.subproblems_cputime

  };
}