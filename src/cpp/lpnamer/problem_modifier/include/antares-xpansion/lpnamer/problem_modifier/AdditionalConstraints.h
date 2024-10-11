#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

/*!
 *  \class AdditionalConstraint
 *  \brief additional constraint to add to master problem
 *
 *  inherites from a map where keys are variable names and values are the
 * variables coeffs
 */
class AdditionalConstraint : public std::map<std::string, double> {
 private:
  std::string _sectionName;
  std::string _name;
  std::string _sign;
  double _rhs = 0.0;

 public:
  /**
   * \brief AdditionalConstraint default constructor
   */
  AdditionalConstraint() = default;

  /**
   * @brief AdditionalConstraint constructor
   *
   * @param sectionName_p  : string value : name of the section in the ini file
   * that declared this constraint.
   * @param constraintName_p  : string value : name of the constraint.
   * @param sign_p : string value : sign of the constraint. One of
   * "less_or_equal", "equal" or "greater_or_equal"
   * @param rhs_p : double RHS of the constraint
   */
  AdditionalConstraint(std::string sectionName_p, std::string constraintName_p,
                       std::string sign_p, double rhs_p)
      : _sectionName(std::move(sectionName_p)),
        _name(std::move(constraintName_p)),
        _sign(std::move(sign_p)),
        _rhs(rhs_p) {}

  /**
   * @brief AdditionalConstraint destructor
   */
  virtual ~AdditionalConstraint() = default;

  /**
   * @brief AdditionalConstraint::_name getter
   *
   * @param name_p  : string value : name of the constraint
   */
  [[nodiscard]] std::string getName() const { return _name; }

  /**
   * @brief AdditionalConstraint::_rhs getter
   *
   * @param rhs_p  : double value : rhs of the constraint
   */
  [[nodiscard]] double getRHS() const { return _rhs; }

  /**
   * @brief AdditionalConstraint::_sign getter
   *
   * @param name_p  : string value : sign of the constraint (supported values
   * "less_or_equal", "equal", "greater_or_equal")
   */
  [[nodiscard]] std::string getSign() const { return _sign; }

  /**
   * @brief adds a term to the constraint
   *
   * @param varName_p  : string value : name of the variable concerned
   * @param varCoeff_p  : double value : coefficient of the variable concerned
   */
  void setCoeff(const std::string& varName_p, const double varCoeff_p) {
    (*this)[varName_p] = varCoeff_p;
  }
};

/*!
 *  \struct AdditionalConstraints
 *  \brief candidate exclusion constraint structure
 *
 *  inherites from a map where keys are unique constraint names and values are
 * the additional constraints
 */
class AdditionalConstraints
    : public std::map<std::string, AdditionalConstraint> {
 private:
  // set of variables to which a binary corresponding variable will be created
  std::map<std::string, std::string> _variablesToBinarise;
  std::set<std::string> _binaryVariables;
  std::string constraintsFilePath_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;

 public:
  /*!
   *  \brief default constructor for struct AdditionalConstraints
   */
  explicit AdditionalConstraints(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(std::move(logger)) {}

  /*!
   * \brief AdditionalConstraints constructor from an ini file path
   *
   * \param constraints_file_path String corresponding to the additional
   * constraints file path
   */
  explicit AdditionalConstraints(
      std::string constraints_file_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

  void SetConstraintsFile(std::string const& constraints_file_path) {
    constraintsFilePath_ = constraints_file_path;
  }
  void ReadConstraintsFile();
  /*!
   * \brief adds a binary variable to be created and links it to the
   * corresponding variable
   *
   * \param oldVarName_p String name of the old variable
   * \param binVarName_p String name of the new binary variable to link to the
   * old existing one
   *
   * adds an entry to the AdditionalConstraints::_variablesToBinarise
   */
  void addVariableToBinarise(const std::string& oldVarName_p,
                             const std::string& binVarName_p);

  /*!
   * \brief adds a binary variable to be created and links it to the
   * corresponding variable
   *
   * \param variables_section std::map<std::string, std::string> whose value is
   * the name of the binary variables to create and whose key is the
   * corresponding existing variable to link toe
   *
   * adds entries to the AdditionalConstraints::_variablesToBinarise
   */
  void addVariablesToBinarise(
      std::map<std::string, std::string> const& variables_section);

  /*!
   * \brief getter for AdditionalConstraints::_variablesToBinarise
   *
   * \return std::map<std::string, std::string> whose value is the name of the
   * binary variables to create and whose key is the corresponding existing
   * variable to link to
   */
  [[nodiscard]] std::map<std::string, std::string> const&
  getVariablesToBinarise() const;

  /*!
   * \brief the method is responsible for creating and filling constraints
   * \param std::string sectionName_l the section name
   * \param constarintsSection_l is a std::map<std::string, std::string> where
   * keys are the constraints entries ("name", "sign", "rhs")
   */
  void constructAdditionalConstraints(
      const std::string& sectionName_l,
      const std::map<std::string, std::string>& constarintsSection_l);

  /*!
   * \brief the method is responsible for checking that section has defined a
   * unique constraint name \param std::string sectionName_l the section name
   * \param constarintsSection_l is a std::map<std::string, std::string> where
   * keys are the constraints entries ("name", "sign", "rhs") \return
   * std::string the constraint name
   */
  [[nodiscard]] std::string checkAndReturnConstraintName(
      const std::string& sectionName_l,
      const std::map<std::string, std::string>& constarintsSection_l) const;

  /*!
   * \brief the method is responsible for checking that section has defined a
   * sign \param std::string sectionName_l the section name \param
   * constarintsSection_l is a std::map<std::string, std::string> where keys are
   * the constraints entries ("name", "sign", "rhs") \return std::string the
   * constraint sign
   */
  [[nodiscard]] std::string checkAndReturnSectionSign(
      const std::string& sectionName_l,
      const std::map<std::string, std::string>& constarintsSection_l) const;

  /*!
   * \brief the method is responsible for checking that section has defined a
   * rhs \param std::string sectionName_l the section name \param
   * constarintsSection_l is a std::map<std::string, std::string> where keys are
   * the constraints entries ("name", "sign", "rhs") \return std::string the rhs
   */
  [[nodiscard]] double checkAndReturnSectionRhs(
      const std::string& sectionName_l,
      const std::map<std::string, std::string>& constarintsSection_l) const;
};
