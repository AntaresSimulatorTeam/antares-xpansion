#include "antares-xpansion/benders/outer_loop/OuterLoopInputDataReader.h"

#include <utility>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

using namespace Outerloop;

/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
 * /!\ body could be := area name or equivalent or nothing
 */
OuterLoopPattern::OuterLoopPattern(std::string prefix,
                                   std::string body)
    : prefix_(std::move(prefix)), body_(std::move(body)) {}

/**
 * just do
 * just cat ;)
 */
std::regex OuterLoopPattern::MakeRegex() const {
  auto pattern = "(^" + prefix_ + "area<" + body_ + ">" + ")";
  return std::regex(pattern);
}
const std::string &OuterLoopPattern::GetPrefix() const { return prefix_; }
void OuterLoopPattern::SetPrefix(const std::string &prefix) {
  prefix_ = prefix;
}
const std::string &OuterLoopPattern::GetBody() const { return body_; }

void OuterLoopPattern::SetBody(const std::string &body) { body_ = body; }

OuterLoopSingleInputData::OuterLoopSingleInputData(const std::string &prefix,
                                                   const std::string &body,
                                                   double criterion)
    : outer_loop_pattern_(prefix, body), criterion_(criterion) {}

OuterLoopPattern OuterLoopSingleInputData::Pattern() const {
  return outer_loop_pattern_;
}
double OuterLoopSingleInputData::Criterion() const { return criterion_; }

void OuterLoopSingleInputData::SetCriterion(double criterion) {
  criterion_ = criterion;
}
void OuterLoopSingleInputData::ResetPattern(const std::string &prefix,
                                            const std::string &body) {
  outer_loop_pattern_.SetPrefix(prefix);
  outer_loop_pattern_.SetBody(body);
}

void OuterLoopInputData::AddSingleData(const OuterLoopSingleInputData &data) {
  outer_loop_data_.push_back(data);
}

const std::vector<OuterLoopSingleInputData> &OuterLoopInputData::OuterLoopData()
    const {
  return outer_loop_data_;
}

std::vector<std::string> OuterLoopInputData::PatternBodies() const {
  std::vector<std::string> ret;
  for (const auto &data : outer_loop_data_) {
    ret.push_back(data.Pattern().GetBody());
  }
  return ret;
}

std::string OuterLoopInputData::PatternsPrefix() const {
  std::string ret("");
  if (!outer_loop_data_.empty()) {
    ret =
        StringManip::split(outer_loop_data_[0].Pattern().GetPrefix(), "::")[0];
  }
  return ret;
}

void OuterLoopInputData::SetStoppingThreshold(
    double outer_loop_stopping_threshold) {
  outer_loop_stopping_threshold_ = outer_loop_stopping_threshold;
}

double OuterLoopInputData::StoppingThreshold() const {
  return outer_loop_stopping_threshold_;
}
void OuterLoopInputData::SetCriterionCountThreshold(
    double criterion_count_threshold) {
    criterion_count_threshold_ = criterion_count_threshold;
}
double OuterLoopInputData::CriterionCountThreshold() const { return criterion_count_threshold_; }

OuterLoopInputData OuterLoopInputFromYaml::Read(
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

  return yaml_content.as<OuterLoopInputData>();
}

namespace YAML {

template <>
struct convert<OuterLoopSingleInputData> {
  static Node encode(const OuterLoopSingleInputData &rhs) {
    return {};
  }

  static bool decode(const Node &pattern, OuterLoopSingleInputData &rhs) {
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
struct convert<OuterLoopInputData> {
  static Node encode(const OuterLoopInputData &rhs) {
    return {};
  }

  static void DecodePatterns(const Node &patterns, OuterLoopInputData &rhs) {
    if (!patterns.IsSequence()) {
      std::ostringstream err_msg;
      err_msg << "In outer loop input file 'patterns' should be an array."
              << "\n";
      throw OuterLoopInputPatternsShouldBeArray(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop"), err_msg.str(),
          LOGLOCATION);
    }

    for (const auto &pattern : patterns) {
      rhs.AddSingleData(pattern.as<OuterLoopSingleInputData>());
    }
  }

  static bool decode(const Node &node, OuterLoopInputData &rhs) {
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