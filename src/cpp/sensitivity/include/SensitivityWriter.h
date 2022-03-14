#pragma once

#include <json/writer.h>

#include "SensitivityOutputData.h"

const std::string ANTARES_C("antares");
const std::string VERSION_C("version");
const std::string ANTARES_XPANSION_C("antares_xpansion");
const std::string EPSILON_C("epsilon");
const std::string BEST_BENDERS_C("best benders cost");
const std::string OPT_DIR_C("optimization direction");
const std::string PB_TYPE_C("problem type");
const std::string CAPEX_C("capex");
const std::string PROJECTION_C("investment");
const std::string STATUS_C("status");
const std::string SYSTEM_COST_C("system cost");
const std::string OBJECTIVE_C("objective");
const std::string NAME_C("name");
const std::string INVEST_C("invest");
const std::string CANDIDATES_C("candidates");
const std::string SENSITIVITY_SOLUTION_C("sensitivity solutions");

class SensitivityWriter {
 private:
  const std::string _filename;

 public:
  SensitivityWriter() = delete;
  explicit SensitivityWriter(std::string json_filename);
  ~SensitivityWriter() = default;

  void end_writing(SensitivityOutputData const &output_data) const;
};