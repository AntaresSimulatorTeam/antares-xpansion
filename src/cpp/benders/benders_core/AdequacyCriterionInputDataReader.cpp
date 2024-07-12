#include "AdequacyCriterionInputDataReader.h"

#include <utility>

using namespace AdequacyCriterionSpace;

/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
 * /!\ body could be := area name or equivalent or nothing
 */
AdequacyCriterionPattern::AdequacyCriterionPattern(std::string prefix,
                                                   std::string body)
    : prefix_(std::move(prefix)), body_(std::move(body)) {}

/**
 * just do
 * just cat ;)
 */
std::regex AdequacyCriterionPattern::MakeRegex() const {
  auto pattern = "(^" + prefix_ + "area<" + body_ + ">" + ")";
  return std::regex(pattern);
}
const std::string &AdequacyCriterionPattern::GetPrefix() const {
  return prefix_;
}
void AdequacyCriterionPattern::SetPrefix(const std::string &prefix) {
  prefix_ = prefix;
}
const std::string &AdequacyCriterionPattern::GetBody() const { return body_; }

void AdequacyCriterionPattern::SetBody(const std::string &body) {
  body_ = body;
}

AdequacyCriterionSingleInputData::AdequacyCriterionSingleInputData(
    const std::string &prefix, const std::string &body, double criterion)
    : adequacy_criterion_pattern_(prefix, body), criterion_(criterion) {}

AdequacyCriterionPattern AdequacyCriterionSingleInputData::Pattern() const {
  return adequacy_criterion_pattern_;
}
double AdequacyCriterionSingleInputData::Criterion() const {
  return criterion_;
}

void AdequacyCriterionSingleInputData::SetCriterion(double criterion) {
  criterion_ = criterion;
}
void AdequacyCriterionSingleInputData::ResetPattern(const std::string &prefix,
                                                    const std::string &body) {
  adequacy_criterion_pattern_.SetPrefix(prefix);
  adequacy_criterion_pattern_.SetBody(body);
}

void AdequacyCriterionInputData::AddSingleData(
    const AdequacyCriterionSingleInputData &data) {
  adequacy_criterion_data_.push_back(data);
}

std::vector<std::string> AdequacyCriterionInputData::PatternBodies() const {
  std::vector<std::string> ret;
  for (const auto &data : adequacy_criterion_data_) {
    ret.push_back(data.Pattern().GetBody());
  }
  return ret;
}

std::string AdequacyCriterionInputData::PatternsPrefix() const {
  std::string ret("");
  if (!adequacy_criterion_data_.empty()) {
    ret = StringManip::split(adequacy_criterion_data_[0].Pattern().GetPrefix(),
                             "::")[0];
  }
  return ret;
}
std::vector<AdequacyCriterionSingleInputData>
AdequacyCriterionInputData::AdequacyCriterionData() const {
  return adequacy_criterion_data_;
}

void AdequacyCriterionInputData::SetStoppingThreshold(
    double adequacy_criterion_stopping_threshold) {
  adequacy_criterion_stopping_threshold_ =
      adequacy_criterion_stopping_threshold;
}

double AdequacyCriterionInputData::StoppingThreshold() const {
  return adequacy_criterion_stopping_threshold_;
}
void AdequacyCriterionInputData::SetCriterionCountThreshold(
    double criterion_count_threshold) {
  criterion_count_threshold_ = criterion_count_threshold;
}
double AdequacyCriterionInputData::CriterionCountThreshold() const {
  return criterion_count_threshold_;
}

AdequacyCriterionInputData AdequacyCriterionInputFromYaml::Read(
    const std::filesystem::path &input_file) {
  YAML::Node yaml_content;
  try {
    yaml_content = YAML::LoadFile(input_file.string());
  } catch (const std::exception &e) {
    std::ostringstream err_msg;
    err_msg << "Could not read outer loop input file: " << input_file << "\n"
            << e.what();
    throw AdequacyCriterionInputFileError(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Adequacy Criterion"),
        err_msg.str(), LOGLOCATION);
  }
  if (yaml_content.IsNull()) {
    std::ostringstream err_msg;
    err_msg << "outer loop input file is empty: " << input_file << "\n";
    throw AdequacyCriterionInputFileIsEmpty(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Adequacy Criterion"),
        err_msg.str(), LOGLOCATION);
  }

  return yaml_content.as<AdequacyCriterionInputData>();
}

/*
# critère d'arrêt de l'algo
stopping_threshold: 1e-4
# seuil
criterion_count_threshold: 1e-1
patterns:
  - area: "N0"
    criterion: 1
  - area: "N1"
    criterion: 1
  - area: "N2"
    criterion: 1
  - area: "N3"
    criterion: 1
*/

namespace YAML {

template <>
struct convert<AdequacyCriterionSingleInputData> {
  static Node encode(const AdequacyCriterionSingleInputData &rhs) { return {}; }

  static bool decode(const Node &pattern,
                     AdequacyCriterionSingleInputData &rhs) {
    auto body = pattern["area"];

    // specify line And OR #pattern
    if (body.IsNull()) {
      std::ostringstream err_msg;
      err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Adequacy Criterion")
              << "Error could not read 'area' field in outer loop input file"
              << "\n";
      throw AdequacyCriterionCouldNotReadAreaField(err_msg.str(), LOGLOCATION);
    }
    auto criterion = pattern["criterion"];

    if (criterion.IsNull()) {
      std::ostringstream err_msg;
      err_msg
          << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Adequacy Criterion")
          << "Error could not read 'criterion' field in outer loop input file"
          << "\n";
      throw AdequacyCriterionCouldNotReadCriterionField(err_msg.str(),
                                                        LOGLOCATION);
    }

    rhs.SetCriterion(criterion.as<double>());
    rhs.ResetPattern("PositiveUnsuppliedEnergy::", body.as<std::string>());
    return true;
  }
};
template <>
struct convert<AdequacyCriterionInputData> {
  static Node encode(const AdequacyCriterionInputData &rhs) { return {}; }

  static void DecodePatterns(const Node &patterns,
                             AdequacyCriterionInputData &rhs) {
    if (!patterns.IsSequence()) {
      std::ostringstream err_msg;
      err_msg << "In outer loop input file 'patterns' should be an array."
              << "\n";
      throw AdequacyCriterionInputPatternsShouldBeArray(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Adequacy Criterion"),
          err_msg.str(), LOGLOCATION);
    }

    for (const auto &pattern : patterns) {
      rhs.AddSingleData(pattern.as<AdequacyCriterionSingleInputData>());
    }
  }

  static bool decode(const Node &node, AdequacyCriterionInputData &rhs) {
    rhs.SetStoppingThreshold(node["stopping_threshold"].as<double>(1e-4));
    rhs.SetCriterionCountThreshold(
        node["criterion_count_threshold"].as<double>(1));

    if (auto patterns = node["patterns"]) {
      DecodePatterns(patterns, rhs);
    } else {
      std::ostringstream err_msg;
      err_msg << "outer loop input file must contains at least one pattern."
              << "\n";
      throw AdequacyCriterionInputFileNoPatternFound(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Adequacy Criterion"),
          err_msg.str(), LOGLOCATION);
    }
    return true;
  }
};
}  // namespace YAML