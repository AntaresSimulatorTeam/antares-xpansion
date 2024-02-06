#ifndef ANTARESXPANSION_COLUMNTOCHANGE_H
#define ANTARESXPANSION_COLUMNTOCHANGE_H

#include <vector>
using colId = unsigned int;
class ColumnToChange {
 public:
  ColumnToChange(colId id, unsigned time_step) : id(id), time_step(time_step){};
  bool operator==(const ColumnToChange& other) const;

  colId id;
  unsigned int time_step;
};

using ColumnsToChange = std::vector<ColumnToChange>;
using linkId = unsigned int;

#endif  // ANTARESXPANSION_COLUMNTOCHANGE_H
