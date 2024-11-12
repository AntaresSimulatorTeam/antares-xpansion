#include "include/antares-xpansion/benders/benders_core/CriterionInputDataReader.h"

#include <utility>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

using namespace Benders::Criterion;

/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
 * /!\ body could be := area name or equivalent or nothing
 */
CriterionPattern::CriterionPattern(std::string prefix, std::string body)
    : prefix_(std::move(prefix)), body_(std::move(body)) {}

/**
 * just do
 * just cat ;)
 */
std::regex CriterionPattern::MakeRegex() const {
  auto pattern = "(^" + prefix_ + "area<" + body_ + ">" + ")";
  return std::regex(pattern);
}
const std::string &CriterionPattern::GetPrefix() const { return prefix_; }
void CriterionPattern::SetPrefix(const std::string &prefix) {
  prefix_ = prefix;
}
const std::string &CriterionPattern::GetBody() const { return body_; }

void CriterionPattern::SetBody(const std::string &body) { body_ = body; }

CriterionSingleInputData::CriterionSingleInputData(const std::string &prefix,
                                                   const std::string &body,
                                                   double criterion)
    : outer_loop_pattern_(prefix, body), criterion_(criterion) {}

CriterionPattern CriterionSingleInputData::Pattern() const {
  return outer_loop_pattern_;
}
double CriterionSingleInputData::Criterion() const { return criterion_; }

void CriterionSingleInputData::SetCriterion(double criterion) {
  criterion_ = criterion;
}
void CriterionSingleInputData::ResetPattern(const std::string &prefix,
                                            const std::string &body) {
  outer_loop_pattern_.SetPrefix(prefix);
  outer_loop_pattern_.SetBody(body);
}

void CriterionInputData::AddSingleData(const CriterionSingleInputData &data) {
  criterions_.push_back(data);
}

const std::vector<CriterionSingleInputData> &CriterionInputData::OuterLoopData()
    const {
  return criterions_;
}

std::vector<std::string> CriterionInputData::PatternBodies() const {
  std::vector<std::string> ret;
  for (const auto &data : criterions_) {
    ret.push_back(data.Pattern().GetBody());
  }
  return ret;
}

std::string CriterionInputData::PatternsPrefix() const {
  std::string ret("");
  if (!criterions_.empty()) {
    ret = StringManip::split(criterions_[0].Pattern().GetPrefix(), "::")[0];
  }
  return ret;
}

void CriterionInputData::SetStoppingThreshold(double stopping_threshold) {
  stopping_threshold_ = stopping_threshold;
}

double CriterionInputData::StoppingThreshold() const {
  return stopping_threshold_;
}
void CriterionInputData::SetCriterionCountThreshold(double count_threshold) {
  count_threshold_ = count_threshold;
}
double CriterionInputData::CriterionCountThreshold() const {
  return count_threshold_;
}

CriterionInputData CriterionInputFromYaml::Read(
    const std::filesystem::path &input_file) {
  YAML::Node yaml_content;
  try {
    yaml_content = YAML::LoadFile(input_file.string());
  } catch (const std::exception &e) {
    std::ostringstream err_msg;
    err_msg << "Could not read outer loop input file: " << input_file << "\n"
            << e.what();
    throw OuterLoopInputFileError(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
        LOGLOCATION);
  }
  if (yaml_content.IsNull()) {
    std::ostringstream err_msg;
    err_msg << "outer loop input file is empty: " << input_file << "\n";
    throw OuterLoopInputFileIsEmpty(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
        LOGLOCATION);
  }

  return yaml_content.as<CriterionInputData>();
}

namespace YAML {

template <>
struct convert<CriterionSingleInputData> {
  static Node encode(const CriterionSingleInputData &rhs) { return {}; }

  static bool decode(const Node &pattern, CriterionSingleInputData &rhs) {
    auto body = pattern["area"];

    // specify line And OR #pattern
    if (body.IsNull()) {
      std::ostringstream err_msg;
      err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
              << "Error could not read 'area' field in outer loop input file"
              << "\n";
      throw OuterLoopCouldNotReadAreaField(err_msg.str(), LOGLOCATION);
    }
    auto criterion = pattern["criterion"];

    if (criterion.IsNull()) {
      std::ostringstream err_msg;
      err_msg
          << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
          << "Error could not read 'criterion' field in outer loop input file"
          << "\n";
      throw OuterLoopCouldNotReadCriterionField(err_msg.str(), LOGLOCATION);
    }

    rhs.SetCriterion(criterion.as<double>());
    rhs.ResetPattern(PositiveUnsuppliedEnergy, body.as<std::string>());
    return true;
  }
};
template <>
struct convert<CriterionInputData> {
  static Node encode(const CriterionInputData &rhs) { return {}; }

  static void DecodePatterns(const Node &patterns, CriterionInputData &rhs) {
    if (!patterns.IsSequence()) {
      std::ostringstream err_msg;
      err_msg << "In outer loop input file 'patterns' should be an array."
              << "\n";
      throw OuterLoopInputPatternsShouldBeArray(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
          LOGLOCATION);
    }

    for (const auto &pattern : patterns) {
      rhs.AddSingleData(pattern.as<CriterionSingleInputData>());
    }
  }

  static bool decode(const Node &node, CriterionInputData &rhs) {
    rhs.SetStoppingThreshold(node["stopping_threshold"].as<double>(1e-4));
    rhs.SetCriterionCountThreshold(
        node["criterion_count_threshold"].as<double>(1));

    if (auto patterns = node["patterns"]) {
      DecodePatterns(patterns, rhs);
    } else {
      std::ostringstream err_msg;
      err_msg << "outer loop input file must contains at least one pattern."
              << "\n";
      throw OuterLoopInputFileNoPatternFound(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
          LOGLOCATION);
    }
    return true;
  }
};
}  // namespace YAML