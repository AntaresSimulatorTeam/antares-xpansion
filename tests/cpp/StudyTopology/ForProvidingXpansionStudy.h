//
// Created by marechaljas on 20/09/22.
//

#pragma once

#include <filesystem>
#include <vector>

#include "Link.h"
#include "StudyAdapter.h"
#include "XpansionStudy.h"
class ForProvidingXpansionStudy {
 public:
  ForProvidingXpansionStudy() = delete;
  ForProvidingXpansionStudy(const ForProvidingXpansionStudy&) = delete;
  ForProvidingXpansionStudy(ForProvidingXpansionStudy&&) = delete;
  ~ForProvidingXpansionStudy() = default;

  explicit ForProvidingXpansionStudy(std::shared_ptr<IStudyAdapter> adapter);
  [[nodiscard]] XpansionStudy provide(const std::filesystem::path& study_path) const;

 private:
  std::shared_ptr<IStudyAdapter> study_adapter_;
};
