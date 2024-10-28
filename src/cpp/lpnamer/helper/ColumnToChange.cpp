#include "antares-xpansion/lpnamer/helper/ColumnToChange.h"

bool ColumnToChange::operator==(const ColumnToChange& other) const {
  bool result = id == other.id;
  result &= time_step == other.time_step;
  return result;
}