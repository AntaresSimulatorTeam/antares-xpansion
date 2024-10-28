#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"

/*!
 * \brief Get point
 */
Point WorkerMasterData::get_x_cut() const { return *_x_cut; }

Point WorkerMasterData::get_min_invest() const { return *_min_invest; }

Point WorkerMasterData::get_max_invest() const { return *_max_invest; }

