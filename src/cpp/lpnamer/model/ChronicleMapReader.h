//
// Created by marechaljas on 28/04/2022.
//

#ifndef ANTARESXPANSION_TESTS_CPP_LP_NAMER_CHRONICLEMAPREADERTEST_CPP_CHRONICLEMAPREADER_H_
#define ANTARESXPANSION_TESTS_CPP_LP_NAMER_CHRONICLEMAPREADERTEST_CPP_CHRONICLEMAPREADER_H_
#include <map>
#include <sstream>

class ScenarioToChronicleReader {
 private:
  mutable std::map<unsigned, unsigned > chronicle_map;

  void ignoreFirstLine(std::istream& ss) const;

  void AssignChronicleValueToMCYear(std::istream& ss,
                                    unsigned int montecarlo_year) const;
  void AssignChronicleValuesToMCYears(std::istream& ss) const;

 public:
  [[nodiscard]] std::map<unsigned int, unsigned int> read(std::string const& input) const;

  [[nodiscard]] std::map<unsigned int, unsigned int> read(std::istream& input) const;
};
#endif  // ANTARESXPANSION_TESTS_CPP_LP_NAMER_CHRONICLEMAPREADERTEST_CPP_CHRONICLEMAPREADER_H_
