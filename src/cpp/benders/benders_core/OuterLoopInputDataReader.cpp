#include "OuterLoopInputDataReader.h"

using namespace Outerloop;
/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
 * /!\ body could be := 'Brittany' or equivalent or nothing
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

const std::string &OuterLoopPattern::GetBody() const { return body_; }

OuterLoopSingleInputData::OuterLoopSingleInputData(const std::string &prefix,
                                                   const std::string &body,
                                                   double criterion)
    : outer_loop_pattern_(prefix, body), criterion_(criterion) {}

OuterLoopPattern OuterLoopSingleInputData::Pattern() const {
  return outer_loop_pattern_;
}
double OuterLoopSingleInputData::Criterion() const { return criterion_; }

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
OuterLoopInputData OuterLoopInputFromJson::Read(
    const std::filesystem::path &input_file) {
  auto json_content = get_json_file_content(input_file);

  if (json_content.empty()) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "outer loop input file is empty: " << input_file << "\n";
    throw OuterLoopInputFileIsEmpty(err_msg.str(), LOGLOCATION);
  }

  Decode(json_content);

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
void OuterLoopInputFromJson::Decode(const Json::Value &json_content) {

  outerLoopInputData_.SetStoppingThreshold(
      json_content
          .get("stopping_threshold",
               1e-4)
          .asDouble());
  outerLoopInputData_.SetCriterionCountThreshold(
      json_content
          .get("criterion_count_threshold",
               1e-1)
          .asDouble());
outerLoopInputData_.SetCriterionTolerance(
      json_content
          .get("criterion_tolerance",
               1e-1)
          .asDouble());

  Json::Value patterns = json_content.get("patterns", Json::nullValue);
  if (patterns  == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "outer loop input file must contains at least one pattern."
            << "\n";
    throw OuterLoopInputFileNoPatternFound(err_msg.str(), LOGLOCATION);
  }

  DecodePatterns(patterns);
}

void OuterLoopInputFromJson::DecodePatterns(const Json::Value &patterns) {
  std::vector<OuterLoopSingleInputData> outer_loop_patterns;

  if (!patterns.isArray()) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "In outer loop input file 'patterns' should be an array."
            << "\n";
    throw OuterLoopInputPatternsShouldBeArray(err_msg.str(), LOGLOCATION);
  }

  for (const auto& pattern : patterns) {
    DecodePattern(pattern);
  }
}

void OuterLoopInputFromJson::DecodePattern(const Json::Value &pattern) {
  Json::Value body = pattern.get("area", Json::nullValue);

  // specify line And OR #pattern
  if (body  == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "Error could not read 'area' field in outer loop input file"
            << "\n";
    throw OuterLoopCouldNotReadAreaField(err_msg.str(), LOGLOCATION);
  }
  Json::Value criterion = pattern.get("criterion", Json::nullValue);

  if (criterion  == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "Error could not read 'criterion' field in outer loop input file"
            << "\n";
    throw OuterLoopCouldNotReadCriterionField(err_msg.str(), LOGLOCATION);
  }

  outerLoopInputData_.AddSingleData(
      {"PositiveUnsuppliedEnergy::", body.asString(), criterion.asDouble()});
}
