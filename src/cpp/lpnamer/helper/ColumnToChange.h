#ifndef ANTARESXPANSION_COLUMNTOCHANGE_H
#define ANTARESXPANSION_COLUMNTOCHANGE_H

#include <vector>
using colId = unsigned int;
struct ColumnToChange {
  ColumnToChange(colId id, unsigned int time_step)
      : id(id), time_step(time_step){};
  bool operator==(const ColumnToChange& other) const;

  colId id;
  int time_step;
};

using ColumnsToChange = std::vector<ColumnToChange>;
using linkId = unsigned int;

#endif  // ANTARESXPANSION_COLUMNTOCHANGE_H
