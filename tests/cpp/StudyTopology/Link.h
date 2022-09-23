//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <vector>

#include "CandidateID.h"
class Link {
 public:
  std::vector<CandidateID> Candidates();

 private:
  std::vector<CandidateID> candidates_;
};
