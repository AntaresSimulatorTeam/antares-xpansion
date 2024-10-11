//
// Created by marechaljas on 02/11/22.
//

#pragma once

#include "antares-xpansion/lpnamer/problem_modifier/IProblemProviderPort.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"
class ZipProblemProviderAdapter : public IProblemProviderPort {
 public:
  explicit ZipProblemProviderAdapter(std::filesystem::path lp_dir,
                                     std::string problem_name,
                                     std::shared_ptr<ArchiveReader> ptr);
  void reader_extract_file(const std::string& problem_name,
                           ArchiveReader& reader,
                           const std::filesystem::path& lpDir) const;
  const std::filesystem::path lp_dir_;
  const std::string problem_name_;
  [[nodiscard]] std::shared_ptr<Problem> provide_problem(
      const std::string& solver_name,
      SolverLogManager& solver_log_manager) const override;
  std::shared_ptr<ArchiveReader> archive_reader_;
};
