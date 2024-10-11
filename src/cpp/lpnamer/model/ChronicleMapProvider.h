//
// Created by marechaljas on 29/04/2022.
//

#ifndef ANTARESXPANSION_TESTS_CPP_LP_NAMER_CHRONICLEMAPPROVIDERTEST_CPP_CHRONICLEMAPPROVIDER_H_
#define ANTARESXPANSION_TESTS_CPP_LP_NAMER_CHRONICLEMAPPROVIDERTEST_CPP_CHRONICLEMAPPROVIDER_H_
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <utility>

#include "ChronicleMapReader.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

class DirectAccessScenarioToChronicleProvider {
 public:
  explicit DirectAccessScenarioToChronicleProvider(
      std::filesystem::path ts_info_root,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : ts_info_root_(std::move(ts_info_root)), logger_(logger) {}

  [[nodiscard]] std::map<unsigned int, unsigned int> GetMap(
      std::string const& link_from, std::string const& link_to) const;

 private:
  std::filesystem::path GetPath(const std::string& link_from,
                                const std::string& link_to) const;
  std::filesystem::path ts_info_root_;
  ScenarioToChronicleReader chronicle_map_reader_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

#endif  // ANTARESXPANSION_TESTS_CPP_LP_NAMER_CHRONICLEMAPPROVIDERTEST_CPP_CHRONICLEMAPPROVIDER_H_
