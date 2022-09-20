//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <filesystem>
#include <vector>

#include "Link.h"
#include "StudyAdapter.h"
class LinkService {
 public:
  explicit LinkService(const IStudyAdapter& adapter);
  [[nodiscard]] std::vector<Link> Load(const std::filesystem::path& study_path) const;
};
