//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <filesystem>
#include <vector>

#include "Link.h"
class LinkService {
 public:
  [[nodiscard]] std::vector<Link> Load(const std::filesystem::path& study_path) const;
};
