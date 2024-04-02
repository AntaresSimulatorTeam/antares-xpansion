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

OuterLoopSingleInputData::OuterLoopSingleInputData(const std::string &prefix,
                                                   const std::string &body,
                                                   double criterion)
    : outer_loop_pattern_(prefix, body), criterion_(criterion) {}

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
  "epsilon" : 1e-4,  // critère d'arrêt de l'algo
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
  Json::Value outer_loop_stopping_threshold_default =
      1e-5;  //=outer_loop_stopping_threshold_; default value
             // Json::Value outer_loop_stopping_threshold_;
  auto outer_loop_stopping_threshold = json_content.get(
      "outer_loop_stopping_threshold", outer_loop_stopping_threshold_default);

  Json::Value patterns;
  if (patterns =
          json_content.get("patterns", Json::nullValue) == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "outer loop input file must contains at least one pattern."
            << "\n";
    throw OuterLoopInputFileNoPatternFound(err_msg.str(), LOGLOCATION);
  }

  auto outer_loop_patterns = DecodePatterns(patterns);
}

std::vector<OuterLoopSingleInputData> OuterLoopInputFromJson::DecodePatterns(
    const Json::Value &patterns) const {
  std::vector<OuterLoopSingleInputData> outer_loop_patterns;

  if (!patterns.isArray()) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "In outer loop input file 'patterns' should be an array."
            << "\n";
    throw OuterLoopInputPatternsShouldBeArray(err_msg.str(), LOGLOCATION);
  }

  for (const auto pattern : patterns) {
    outer_loop_patterns.push_back(DecodePattern(pattern));
  }

  return outer_loop_patterns;
}

OuterLoopSingleInputData OuterLoopInputFromJson::DecodePattern(
    const Json::Value &pattern) const {
  Json::Value body;

  // specify line And OR #pattern
  if (body = pattern.get("area", Json::nullValue) == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "Error could not read 'area' field in outer loop input file"
            << "\n";
    throw OuterLoopCouldNotReadAreaField(err_msg.str(), LOGLOCATION);
  }
  Json::Value criterion;

  if (criterion =
          pattern.get("criterion", Json::nullValue) == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "Error could not read 'criterion' field in outer loop input file"
            << "\n";
    throw OuterLoopCouldNotReadCriterionField(err_msg.str(), LOGLOCATION);
  }

  return {"PositiveUnsuppliedEnergy::", body.asString(), criterion.asDouble()};
}