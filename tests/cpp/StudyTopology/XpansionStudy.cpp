//
// Created by marechaljas on 23/09/22.
//

#include "XpansionStudy.h"
#include <algorithm>

std::vector<Link> XpansionStudy::Links() { return links_; }
void XpansionStudy::addLink(Link link) {
  links_.push_back(link);
}
void XpansionStudy::RemoveInvalidLinks() {
  auto new_end = std::remove_if(links_.begin(), links_.end(), [](Link link) -> bool {
    return link.Candidates().empty();
  });
  links_.erase(new_end, links_.end());
}
