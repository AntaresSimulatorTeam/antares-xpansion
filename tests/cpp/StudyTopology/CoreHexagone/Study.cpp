//
// Created by marechaljas on 26/09/22.
//

#include "Study.h"

#include "Area.h"
#include "Candidate.h"
#include "Link.h"

namespace XpansionStudy {
std::vector<Link> Study::Links() const { return links_; }
std::vector<Candidate> Study::Candidates() const { return {}; }
std::vector<Area> Study::Areas() const { return {}; }
Study::Study(std::vector<Link>&& links) {
  links_ = std::move(links);
}
}  // namespace XpansionStudy