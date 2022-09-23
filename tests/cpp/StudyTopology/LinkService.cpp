//
// Created by marechaljas on 20/09/22.
//

#include <utility>

#include "ForProvidingXpansionStudy.h"
#include "XpansionStudy.h"
#include <algorithm>

XpansionStudy ForProvidingXpansionStudy::provide(const std::filesystem::path& study_path) const {
  auto study = study_adapter_->Study();
  study.RemoveInvalidLinks();
  return study;
}
ForProvidingXpansionStudy::ForProvidingXpansionStudy(std::shared_ptr<IStudyAdapter> adapter):
  study_adapter_(std::move(adapter))
{

}
