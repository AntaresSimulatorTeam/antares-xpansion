//
// Created by marechaljas on 26/09/22.
//

#pragma once

#include <string>
#include <vector>

#include "Model/Link.h"

namespace StudyFileReader {
  class ILinkFileReader {
   public:
    virtual std::vector<Link> Links(const std::string &study_path) const = 0;
  };
}

