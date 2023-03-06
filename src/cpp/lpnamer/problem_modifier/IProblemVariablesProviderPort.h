//
// Created by marechaljas on 08/11/22.
//

#pragma once
#include <map>
#include <string>
#include <vector>

#include "ColumnToChange.h"

struct ProblemVariables {
  std::vector<std::string> variable_names;
  std::map<colId, ColumnsToChange> ntc_columns;
  std::map<colId, ColumnsToChange> direct_cost_columns;
  std::map<colId, ColumnsToChange> indirect_cost_columns;
};

class IProblemVariablesProviderPort {
 public:
  virtual ~IProblemVariablesProviderPort() = default;
  virtual ProblemVariables Provide() = 0;
};