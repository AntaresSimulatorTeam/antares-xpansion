#pragma once

#include <map>
#include <set>

/*!
 *  \class AdditionalConstraint
 *  \brief additional constraint to add to master problem
 *
 *  inherites from a map where keys are variable names and values are the variables coeffs
 */
class AdditionalConstraint : public std::map<std::string, double>
{

private:
    std::string _sectionName;
    std::string _name;
    std::string _sign;
    double _rhs;

public:

/**
 * \brief AdditionalConstraint default constructor
 */
    AdditionalConstraint() :
    _sectionName(""),
    _name(""),
    _sign(""),
    _rhs(0)
    {
    }

/**
 * @brief AdditionalConstraint constructor
 *
 * @param sectionName_p  : string value : name of the section in the ini file that declared this constraint.
 * @param constraintName_p  : string value : name of the constraint.
 * @param sign_p : string value : sign of the constraint. One of "less_or_equal", "equal" or "greater_or_equal"
 * @param rhs_p : double RHS of the constraint
 */
    AdditionalConstraint(std::string const & sectionName_p, std::string const & constraintName_p, std::string const & sign_p, double rhs_p) :
    _sectionName(sectionName_p),
    _name(constraintName_p),
    _sign(sign_p),
    _rhs(rhs_p)
    {
    }

/**
 * @brief AdditionalConstraint destructor
 */
    virtual ~AdditionalConstraint()
    {
    }

/**
 * @brief AdditionalConstraint::_name getter
 *
 * @param name_p  : string value : name of the constraint
 */
    std::string getName()
    {
        return _name;
    }

/**
 * @brief AdditionalConstraint::_rhs getter
 *
 * @param rhs_p  : double value : rhs of the constraint
 */
    double getRHS()
    {
        return _rhs;
    }

/**
 * @brief AdditionalConstraint::_sign getter
 *
 * @param name_p  : string value : sign of the constraint
 */
    std::string getSign()
    {
        return _sign;
    }

/**
 * @brief adds a term to the constraint
 *
 * @param varName_p  : string value : name of the variable concerned
 * @param varCoeff_p  : double value : coefficient of the variable concerned
 */
    void setCoeff(std::string varName_p, double varCoeff_p)
    {
        (*this)[varName_p] = varCoeff_p;
    }
};

/*!
 *  \struct AdditionalConstraints
 *  \brief candidate exclusion constraint structure
 *
 *  inherites from a map where keys are unique constraint names and values are the additional constraints
 */
struct AdditionalConstraints : public std::map<std::string, AdditionalConstraint>
{
private:
	//set of variables to which a binary corresponding variable will be created
    std::map<std::string, std::string> _variablesToBinarise;
    std::set<std::string> _binaryVariables;

public:
	AdditionalConstraints() {
	}

	explicit AdditionalConstraints(std::string  const & constraints_file_path);

    void addVariableToBinarise(std::string oldVarName_p, std::string binVarName_p);

    std::map<std::string, std::string> getVariablesToBinarise();
};
