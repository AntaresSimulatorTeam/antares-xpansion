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
    AdditionalConstraint() :
    _sectionName(""),
    _name(""),
    _sign(""),
    _rhs(0)
    {
    }

    AdditionalConstraint(std::string sectionName_p, std::string constraintName_p, std::string sign_p, double rhs_p) :
    _sectionName(sectionName_p),
    _name(constraintName_p),
    _sign(sign_p),
    _rhs(rhs_p)
    {
    }

    virtual ~AdditionalConstraint()
    {
    }

    void setName(std::string name_p)
    {
        _name = name_p;
    }

    void setRHS(double rhs_p)
    {
        _rhs = rhs_p;
    }

    void setSign(std::string sign_p)
    {
        _sign = sign_p;
    }

    std::string getName()
    {
        return _name;
    }

    double getRHS()
    {
        return _rhs;
    }

    std::string getSign()
    {
        return _sign;
    }

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

	AdditionalConstraints(std::string  const & constraints_file_path);

    void addVariableToBinarise(std::string oldVarName_p, std::string binVarName_p);

    std::map<std::string, std::string> getVariablesToBinarise();
};