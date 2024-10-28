#include "antares-xpansion/lpnamer/input_reader/AdditionalConstraintsReader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

namespace {

// trim leading whitespaces
std::string ltrim(const std::string& s) {
  size_t start = s.find_first_not_of(" \n\r\t\f\v");
  return (start == std::string::npos) ? "" : s.substr(start);
}

// trim trailing whitespaces
std::string rtrim(const std::string& s) {
  size_t end = s.find_last_not_of(" \n\r\t\f\v");
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// trim leading and trailing whitespaces
std::string trim(const std::string& s) { return rtrim(ltrim(s)); }

}  // namespace

std::string AdditionalConstraintsReader::illegal_chars = " \n\r\t\f\v-+=:[]()";

void AdditionalConstraintsReader::processSectionLine() {
  if (_line[_line.length() - 1] != ']') {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "additional constraints file:  line " << _lineNb
        << " : section line not ending with ']'.\n";
    std::exit(1);
  }

  _section = _line.substr(1, _line.find(']') - 1);

  if (!_sections.insert(_section).second) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "additional constraints file:  line " << _lineNb
        << " : duplicate section " << _section << "!\n";
    std::exit(1);
  }
}

void AdditionalConstraintsReader::processEntryLine() {
  size_t delimiterIt_l = _line.find(" = ");
  if (delimiterIt_l == std::string::npos) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "additional constraints file: line " << _lineNb
        << " : incorrect entry line format. Expected format 'attribute = "
           "value'!\n";
    std::exit(1);
  }

  std::string attribute_l = rtrim(_line.substr(0, delimiterIt_l));
  std::string value_l = ltrim(_line.substr(delimiterIt_l + 3));

  size_t illegalCharIndex_l =
      attribute_l.find_first_of(AdditionalConstraintsReader::illegal_chars);
  if (illegalCharIndex_l != std::string::npos) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "additional constraints file: line " << _lineNb
        << " : Illegal character '" << attribute_l[illegalCharIndex_l]
        << "' in attribute name!\n";
    std::exit(1);
  }

  if (_values[_section].count(attribute_l)) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "additional constraints file:  line " << _lineNb
        << " : duplicate attribute " << attribute_l << "!\n";
    std::exit(1);
  } else {
    if ((attribute_l == "name") || (_section == "variables")) {
      illegalCharIndex_l =
          value_l.find_first_of(AdditionalConstraintsReader::illegal_chars);
      if (illegalCharIndex_l != std::string::npos) {
        (*logger_)(LogUtils::LOGLEVEL::FATAL)
            << LOGLOCATION << "additional constraints file: line " << _lineNb
            << " : Illegal character '" << value_l[illegalCharIndex_l]
            << "' in value!\n";
        std::exit(1);
      }
    }

    if (attribute_l == "sign") {
      if ((value_l != "greater_or_equal") && (value_l != "less_or_equal") &&
          (value_l != "equal")) {
        (*logger_)(LogUtils::LOGLEVEL::FATAL)
            << LOGLOCATION << "additional constraints file:  line " << _lineNb
            << " : Illegal sign value : " << value_l
            << "! supported values are: "
            << "greater_or_equal, less_or_equal and equal.\n";
        std::exit(1);
      }
    }

    _values[_section][attribute_l] = value_l;
  }
}

AdditionalConstraintsReader::AdditionalConstraintsReader(
    std::string const& constraints_file_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : logger_(logger) {
  std::ifstream file_l(constraints_file_path);

  while (std::getline(file_l, _line)) {
    ++_lineNb;
    _line = trim(_line);
    std::transform(_line.begin(), _line.end(), _line.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if ((_line.length() == 0) || (_line[0] == '#') ||
        (_line[0] == ';')) {  // line is a comment or empty
      continue;
    } else if (_line[0] == '[') {  // line is a section
      processSectionLine();
    } else {
      // check we have a valid section name
      if (_section == "") {
        (*logger_)(LogUtils::LOGLEVEL::FATAL)
            << LOGLOCATION
            << "additional constraints file:  Section line is required "
               "before line "
            << _lineNb << "!\n";
        std::exit(1);
      }

      processEntryLine();
    }
  }
}

std::map<std::string, std::string> const&
AdditionalConstraintsReader::getVariablesSection() {
  return _values["variables"];
}

std::set<std::string> const& AdditionalConstraintsReader::getSections() const {
  return _sections;
}

std::map<std::string, std::string> const&
AdditionalConstraintsReader::getSection(
    std::string const& sectionName_p) const {
  return _values.at(sectionName_p);
}
