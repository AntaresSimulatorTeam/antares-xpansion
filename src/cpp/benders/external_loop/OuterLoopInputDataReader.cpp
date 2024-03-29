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
  if (json_content.get("patterns", patterns) == Json::nullValue) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "outer loop input file must contains at least one pattern: "
            << "\n";
    throw OuterLoopInputFileNoPatternFound(err_msg.str(), LOGLOCATION);
  }
}
