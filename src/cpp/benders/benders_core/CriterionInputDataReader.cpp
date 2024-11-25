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
    : pattern_(prefix, body), criterion_(criterion) {}

CriterionPattern CriterionSingleInputData::Pattern() const { return pattern_; }
double CriterionSingleInputData::Criterion() const { return criterion_; }

void CriterionSingleInputData::SetCriterion(double criterion) {
  criterion_ = criterion;
}
void CriterionSingleInputData::ResetPattern(const std::string &prefix,
                                            const std::string &body) {
  pattern_.SetPrefix(prefix);
  pattern_.SetBody(body);
}

void CriterionInputData::AddSingleData(const CriterionSingleInputData &data) {
  criterion_vector_.push_back(data);
}

const std::vector<CriterionSingleInputData> &
CriterionInputData::Criteria() const {
  return criterion_vector_;
}

std::vector<std::string> CriterionInputData::PatternBodies() const {
  std::vector<std::string> ret;
  for (const auto &data : criterion_vector_) {
    ret.push_back(data.Pattern().GetBody());
  }
  return ret;
}

std::string CriterionInputData::PatternsPrefix() const {
  std::string ret("");
  if (!criterion_vector_.empty()) {
    ret =
        StringManip::split(criterion_vector_[0].Pattern().GetPrefix(), "::")[0];
  }
  return ret;
}


void CriterionInputData::SetCriterionCountThreshold(double count_threshold) {
  count_threshold_ = count_threshold;
}
double CriterionInputData::CriterionCountThreshold() const {
  return count_threshold_;
}

OuterLoopCriterionInputData CriterionInputFromYaml::Read(
    const std::filesystem::path &input_file) {
  YAML::Node yaml_content;
  try {
    yaml_content = YAML::LoadFile(input_file.string());
  } catch (const std::exception &e) {
    std::ostringstream err_msg;
    err_msg << "Could not read outer loop input file: " << input_file << "\n"
            << e.what();
    throw CriterionInputFileError(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
        LOGLOCATION);
  }
  if (yaml_content.IsNull()) {
    std::ostringstream err_msg;
    err_msg << "outer loop input file is empty: " << input_file << "\n";
    throw CriterionInputFileIsEmpty(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
        LOGLOCATION);
  }

  return yaml_content.as<OuterLoopCriterionInputData>();
}

double OuterLoopCriterionInputData::StoppingThreshold() const {
  return stopping_threshold_;
}

void OuterLoopCriterionInputData::setStoppingThreshold(
    double stoppingThreshold) {
  stopping_threshold_ = stoppingThreshold;
}

namespace YAML {

template <>
class convert<CriterionSingleInputData> {
 public:
  static bool decode(const Node &pattern, CriterionSingleInputData &rhs) {
    auto body = pattern["area"];

    // specify line And OR #pattern
    if (body.IsNull()) {
      std::ostringstream err_msg;
      err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
              << "Error could not read 'area' field in outer loop input file"
              << "\n";
      throw CouldNotReadAreaField(err_msg.str(), LOGLOCATION);
    }
    const auto criterion = pattern["criterion"];

    if (criterion.IsNull()) {
      std::ostringstream err_msg;
      err_msg
          << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
          << "Error could not read 'criterion' field in outer loop input file"
          << "\n";
      throw CouldNotReadCriterionField(err_msg.str(), LOGLOCATION);
    }

    rhs.SetCriterion(criterion.as<double>());
    rhs.ResetPattern(PositiveUnsuppliedEnergy, body.as<std::string>());
    return true;
  }
};
template <>
class convert<OuterLoopCriterionInputData> {
 public:
  static void DecodePatterns(const Node &patterns, CriterionInputData &rhs) {
    if (!patterns.IsSequence()) {
      std::ostringstream err_msg;
      err_msg << "In outer loop input file 'patterns' should be an array."
              << "\n";
      throw CriterionInputPatternsShouldBeArray(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
          LOGLOCATION);
    }

    for (const auto &pattern : patterns) {
      rhs.AddSingleData(pattern.as<CriterionSingleInputData>());
    }
  }

  static bool decode(const Node &node, OuterLoopCriterionInputData &rhs) {
    rhs.setStoppingThreshold(node["stopping_threshold"].as<double>(1e-4));
    Benders::Criterion::CriterionInputData sub_rhs;

    rhs.SetCriterionCountThreshold(
        node["criterion_count_threshold"].as<double>(1));

    if (auto patterns = node["patterns"]) {
      DecodePatterns(patterns, rhs);
    } else {
      std::ostringstream err_msg;
      err_msg << "outer loop input file must contains at least one pattern."
              << "\n";
      throw CriterionInputFileNoPatternFound(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
          LOGLOCATION);
    }
    return true;
  }
};
}  // namespace YAML
