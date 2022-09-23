//
// Created by marechaljas on 23/09/22.
//

#include "XpansionStudy.h"
std::vector<Link> XpansionStudy::Links() { return links_; }
void XpansionStudy::addLink(Link link) {
  links_.push_back(link);
}
