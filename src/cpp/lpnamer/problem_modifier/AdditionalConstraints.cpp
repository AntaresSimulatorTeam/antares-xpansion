
#include "antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h"

#include <iostream>
#include <utility>

#include "antares-xpansion/lpnamer/input_reader/AdditionalConstraintsReader.h"
#include "LogUtils.h"

AdditionalConstraints::AdditionalConstraints(
    std::string constraints_file_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : constraintsFilePath_(std::move(constraints_file_path)),
      logger_(std::move(logger)) {
  ReadConstraintsFile();
}
void AdditionalConstraints::ReadConstraintsFile() {
  AdditionalConstraintsReader reader_l(constraintsFilePath_, logger_);

  // treat variables section
  std::map<std::string, std::string> const& variables_section =
      reader_l.getVariablesSection();
  addVariablesToBinarise(variables_section);

  // treat constraints sections
  for (const std::string& sectionName_l : reader_l.getSections()) {
    if (sectionName_l != "variables") {
      std::map<std::string, std::string> const& constarintsSection_l =
          reader_l.getSection(sectionName_l);
      constructAdditionalConstraints(sectionName_l, constarintsSection_l);
    }
  }
}
void AdditionalConstraints::addVariableToBinarise(
    const std::string& oldVarName_p, const std::string& binVarName_p) {
  if (!_binaryVariables.insert(binVarName_p).second) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "Duplicate Binary variable name: " << binVarName_p
        << " was already added.\n";
    std::exit(1);
  }
  _variablesToBinarise[oldVarName_p] = binVarName_p;
}
void AdditionalConstraints::addVariablesToBinarise(
    std::map<std::string, std::string> const& variables_section) {
  for (const auto& pairOldvarNewvar_l : variables_section) {
    addVariableToBinarise(pairOldvarNewvar_l.first, pairOldvarNewvar_l.second);
  }
}

std::map<std::string, std::string> const&
AdditionalConstraints::getVariablesToBinarise() const {
  return _variablesToBinarise;
}

void AdditionalConstraints::constructAdditionalConstraints(
    const std::string& sectionName_l,
    const std::map<std::string, std::string>& constarintsSection_l) {

  // check that section has defined a unique constraint name
  std::string constraintName_l =
      checkAndReturnConstraintName(sectionName_l, constarintsSection_l);
  // check that section has defined a sign
  std::string constraintSign_l =
      checkAndReturnSectionSign(sectionName_l, constarintsSection_l);
  // check that section has defined a rhs
  double constraintRHS_l =
      checkAndReturnSectionRhs(sectionName_l, constarintsSection_l);

  // create and fill the constraint
  AdditionalConstraint constraint_l(sectionName_l, constraintName_l,
                                    constraintSign_l, constraintRHS_l);
  bool emptyCstr_l = true;
  for (const auto& pairAttributeValue_l : constarintsSection_l) {
    if (pairAttributeValue_l.first == "name" ||
        pairAttributeValue_l.first == "rhs" ||
        pairAttributeValue_l.first == "sign") {
      continue;
    } else {
      try {
        std::string::size_type sz;
        double coeff_l = std::stod(pairAttributeValue_l.second, &sz);
        if (coeff_l != 0) {
          emptyCstr_l = false;
        }
        constraint_l.setCoeff(pairAttributeValue_l.first, coeff_l);
      } catch (const std::invalid_argument& e) {
        (*logger_)(LogUtils::LOGLEVEL::FATAL)
            << LOGLOCATION << "Invalid value " << pairAttributeValue_l.second
            << " in section " << sectionName_l
            << ": coeff value must be a double!\n";
        std::exit(1);
      }
    }
  }

  if (emptyCstr_l) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "section " << sectionName_l
        << " defines an empty constraint.\n";
    std::exit(1);
  }

  (*this)[constraintName_l] = constraint_l;
}

std::string AdditionalConstraints::checkAndReturnConstraintName(
    const std::string& sectionName_l,
    const std::map<std::string, std::string>& constarintsSection_l) const {
  std::string constraintName_l;
  // check that section has defined a unique constraint name
  auto temporatyIterator_l = constarintsSection_l.find("name");
  if (temporatyIterator_l == constarintsSection_l.end()) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "section " << sectionName_l
        << " is missing a name.\n";
    std::exit(1);
  } else {
    constraintName_l = temporatyIterator_l->second;
    if (this->count(constraintName_l)) {
      (*logger_)(LogUtils::LOGLEVEL::FATAL)
          << LOGLOCATION << "Duplicate constraint name " << constraintName_l
          << ".\n";
      std::exit(1);
    }
  }
  return constraintName_l;
}

std::string AdditionalConstraints::checkAndReturnSectionSign(
    const std::string& sectionName_l,
    const std::map<std::string, std::string>& constarintsSection_l) const {
  // check that section has defined a sign

  std::string constraintSign_l;
  auto temporatyIterator_l = constarintsSection_l.find("sign");
  if (temporatyIterator_l == constarintsSection_l.end()) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "section " << sectionName_l
        << " is missing a sign.\n";
    std::exit(1);
  } else {
    constraintSign_l = temporatyIterator_l->second;
  }
  return constraintSign_l;
}

double AdditionalConstraints::checkAndReturnSectionRhs(
    const std::string& sectionName_l,
    const std::map<std::string, std::string>& constarintsSection_l) const {
  // check that section has defined a rhs
  double constraintRHS_l = 0;
  auto temporatyIterator_l = constarintsSection_l.find("rhs");
  if (temporatyIterator_l == constarintsSection_l.end()) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "section " << sectionName_l << " is missing a rhs.\n";
    std::exit(1);
  } else {
    try {
      std::string::size_type sz;
      constraintRHS_l = std::stod(temporatyIterator_l->second, &sz);
    } catch (const std::invalid_argument& e) {
      (*logger_)(LogUtils::LOGLEVEL::FATAL)
          << LOGLOCATION << "Invalid value " << temporatyIterator_l->second
          << " in section " << sectionName_l
          << ": rhs value must be a double!\n";
      std::exit(1);
    }
  }
  return constraintRHS_l;
}