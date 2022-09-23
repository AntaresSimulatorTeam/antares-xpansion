//
// Created by marechaljas on 23/09/22.
//

#pragma once

#include <vector>

#include "Link.h"
class XpansionStudy {
 public:
  std::vector<Link> Links();
  void addLink(Link link);
  void RemoveInvalidLinks();

 private:
  std::vector<Link> links_;
};
