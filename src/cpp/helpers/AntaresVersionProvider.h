//
// Created by marechaljas on 03/06/22.
//

#ifndef ANTARESXPANSION_SRC_CPP_HELPERS_ANTARESVERSIONPROVIDER_H_
#define ANTARESXPANSION_SRC_CPP_HELPERS_ANTARESVERSIONPROVIDER_H_

#include <filesystem>
#include <optional>

constexpr int DEFAULT_ANTARES_VERSION = 710;

class AntaresVersionProvider {
 public:
  AntaresVersionProvider() = default;
  virtual ~AntaresVersionProvider() = default;
  [[nodiscard]] virtual int getAntaresVersion(const std::filesystem::path& study_path) const;
};

#endif  // ANTARESXPANSION_SRC_CPP_HELPERS_ANTARESVERSIONPROVIDER_H_
