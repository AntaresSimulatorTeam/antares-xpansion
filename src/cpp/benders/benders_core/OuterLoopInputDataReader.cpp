#include "OuterLoopInputDataReader.h"

using namespace Outerloop;

/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
 * /!\ body could be := area name or equivalent or nothing
 */
OuterLoopPattern::OuterLoopPattern(const std::string &prefix,
                                   const std::string &body)
    : prefix_(prefix), body_(body) {}

/**
 * just do
 * just cat ;)
 */
std::regex OuterLoopPattern::MakeRegex() const {
  auto pattern = "(^" + prefix_ + ").*" + body_;
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

std::vector<OuterLoopSingleInputData> OuterLoopInputData::OuterLoopData()
    const {
  return outer_loop_data_;
}

void OuterLoopInputData::SetStoppingThreshold(
    double outer_loop_stopping_threshold) {
  outer_loop_stopping_threshold_ = outer_loop_stopping_threshold;
}

double OuterLoopInputData::StoppingThreshold() const {
  return outer_loop_stopping_threshold_;
}
void OuterLoopInputData::SetCriterionTolerance(double criterion_tolerance) {
  criterion_tolerance_ = criterion_tolerance;
}
double OuterLoopInputData::CriterionTolerance() const { return criterion_tolerance_; }
void OuterLoopInputData::SetCriterionCountThreshold(
    double criterion_count_threshold) {
    criterion_count_threshold_ = criterion_count_threshold;
}
double OuterLoopInputData::CriterionCountThreshold() const { return criterion_count_threshold_; }
OuterLoopInputData OuterLoopInputFromYaml::Read(
    const std::filesystem::path &input_file) {
  /*auto json_content = get_json_file_content(input_file);

  if (json_content.empty()) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "outer loop input file is empty: " << input_file << "\n";
    throw OuterLoopInputFileIsEmpty(err_msg.str(), LOGLOCATION);
  }*/
  auto yaml_content = YAML::LoadFile(input_file.string());

  if (yaml_content.IsNull()) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "outer loop input file is empty: " << input_file << "\n";
    throw OuterLoopInputFileIsEmpty(err_msg.str(), LOGLOCATION);
  }

  // std::cout << "******* " << yaml_content["eps"].as<double>() << "\n";
  outerLoopInputData_ = yaml_content.as<OuterLoopInputData>();
  return outerLoopInputData_;
}

/*
"BendersOuterloop" : {
  "stopping_threshold" : 1e-4,  // critère d'arrêt de l'algo
  "criterion_count_threshold" : 1e-1,  //  seuil
  "criterion_tolerance" : 1e-1,  // tolerance entre seuil et valeur calculée
      "patterns" : [
        {
          "area" : "Zone1",  // ==> verifier que "criterion" est satisfait pour
                             // la défaillance positive dans la Zone1
          "criterion" : 666
        },
        {
          "area" : "all",  // ==> verifier que "criterion" est satisfait pour la
                           // défaillance positive pour toutes les zones
          "criterion" : 159
        }
      ]
}
*/
namespace YAML {

template <>
struct convert<OuterLoopSingleInputData> {
  static Node encode(const OuterLoopSingleInputData &rhs) {
    //
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
    rhs.ResetPattern("PositiveUnsuppliedEnergy::", body.as<std::string>());
    return true;
  }
};
template <>
struct convert<OuterLoopInputData> {
  static Node encode(const OuterLoopInputData &rhs) {
    //
  }

  static void DecodePatterns(const Node &patterns, OuterLoopInputData &rhs) {
    if (!patterns.IsSequence()) {
      std::ostringstream err_msg;
      err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
              << "In outer loop input file 'patterns' should be an array."
              << "\n";
      throw OuterLoopInputPatternsShouldBeArray(err_msg.str(), LOGLOCATION);
    }

    for (const auto &pattern : patterns) {
      rhs.AddSingleData(pattern.as<OuterLoopSingleInputData>());
    }
  }

  static bool decode(const Node &node, OuterLoopInputData &rhs) {
    std::cout << "********* " << node["stopping_threshold"].as<double>()
              << "\n";
    rhs.SetStoppingThreshold(node["stopping_threshold"].as<double>(1e-4));
    rhs.SetCriterionCountThreshold(
        node["criterion_count_threshold"].as<double>(1e-1));
    rhs.SetCriterionTolerance(node["criterion_tolerance"].as<double>(1e-1));

    if (auto patterns = node["patterns"]) {
      DecodePatterns(patterns, rhs);
    } else {
      std::ostringstream err_msg;
      err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
              << "outer loop input file must contains at least one pattern."
              << "\n";
      throw OuterLoopInputFileNoPatternFound(err_msg.str(), LOGLOCATION);
    }
    return true;
  }
};
}  // namespace YAML