//
// Created by marechaljas on 20/09/22.
//

#include "ForProvidingXpansionStudy.h"

#include <algorithm>
#include <utility>

#include "XpansionStudy.h"

XpansionStudy::XpansionStudy ForProvidingXpansionStudy::provide(const std::filesystem::path& study_path) const {
  auto study = study_adapter_->Study();
  study.RemoveInvalidLinks();
  return study;
}
ForProvidingXpansionStudy::ForProvidingXpansionStudy(std::shared_ptr<IStudyAdapter> adapter):
  study_adapter_(std::move(adapter))
{

}
