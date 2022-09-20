//
// Created by marechaljas on 20/09/22.
//

#include "InMemoryStudyAdapter.h"
std::vector<Link> InMemoryStudyAdapter::Links() const {
  return links_;
}
void InMemoryStudyAdapter::addLink(Link link) {
  links_.push_back(link);
}
