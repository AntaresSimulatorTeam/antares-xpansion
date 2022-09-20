//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include "IStudyAdapter.h"
#include "Link.h"
class InMemoryStudyAdapter: public IStudyAdapter {
 public:
  void addLink(Link link);
  [[nodiscard]] std::vector<Link> Links() const override;
  std::vector<Link> links_;
};
