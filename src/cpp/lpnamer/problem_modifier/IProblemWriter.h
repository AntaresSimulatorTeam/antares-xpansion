//
// Created by marechaljas on 08/11/22.
//

#pragma once

#include "Problem.h"
class IProblemWriter {
 public:
  virtual ~IProblemWriter() = default;
  virtual void Write_problem(std::shared_ptr<Problem>& in_prblm) = 0;
};
