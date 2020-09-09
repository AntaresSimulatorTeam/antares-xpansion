
#include <iostream>

#include "AdditionalConstraints.h"
#include "AdditionalConstraintsReader.h"


/**
 * \brief AdditionalConstraints constructor from an ini file path
 *
 * \param constraints_file_path String corresponding to the additional constraints file path
 */
AdditionalConstraints::AdditionalConstraints(std::string  const & constraints_file_path)
{
    AdditionalConstraintsReader reader_l(constraints_file_path);

    //treat variables section
    std::map<std::string, std::string> const & variables_section = reader_l.getVariablesSection();
    for(auto pairOldvarNewvar_l : variables_section)
    {
        addVariableToBinarise(pairOldvarNewvar_l.first, pairOldvarNewvar_l.second);
    }

    //treat constraints sections
    for(std::string sectionName_l : reader_l.getSections())
    {
        if (sectionName_l != "variables")
        {
            bool emptyCstr_l = true;
            std::map<std::string, std::string> const & constarintsSection_l = reader_l.getSection(sectionName_l);
            AdditionalConstraint constraint_l(sectionName_l);
            std::string constraintName_l;

            //check that section has defined a name, sign and rhs
            if(constarintsSection_l.find("name") == constarintsSection_l.end())
            {
                std::cout << "section " << sectionName_l << " is missing a name.\n";
                std::exit(1);
            }
            if(constarintsSection_l.find("rhs") == constarintsSection_l.end())
            {
                std::cout << "section " << sectionName_l << " is missing a rhs.\n";
                std::exit(1);
            }
            if(constarintsSection_l.find("sign") == constarintsSection_l.end())
            {
                std::cout << "section " << sectionName_l << " is missing a sign.\n";
                std::exit(1);
            }

            for(auto pairAttributeValue_l : constarintsSection_l)
            {
                if(pairAttributeValue_l.first == "name")
                {
                    constraintName_l = pairAttributeValue_l.second;
                    if(this->count(constraintName_l))
                    {
                        std::cout << "Duplicate constraint name " << constraintName_l << ".\n";
                        std::exit(0);
                    }
                    constraint_l.setName(constraintName_l);
                }
                else if(pairAttributeValue_l.first == "rhs")
                {
                    try
                    {
                        std::string::size_type sz;
                        double rhs_l = std::stod(pairAttributeValue_l.second, &sz);
                        constraint_l.setRHS(rhs_l);
                    }
                    catch(const std::invalid_argument& e)
                    {
                        std::cerr << "Invalid value " << pairAttributeValue_l.second << " in section " << sectionName_l
                                    << ": rhs value must be a double!\n";
                        std::exit(1);
                    }
                }
                else if(pairAttributeValue_l.first == "sign")
                {
                    constraint_l.setSign(pairAttributeValue_l.second);
                }
                else
                {
                    try
                    {
                        std::string::size_type sz;
                        double coeff_l = std::stod(pairAttributeValue_l.second, &sz);
                        if(coeff_l != 0)
                        {
                            emptyCstr_l = false;
                        }
                        constraint_l.setCoeff(pairAttributeValue_l.first, coeff_l);
                    }
                    catch(const std::invalid_argument& e)
                    {
                        std::cerr << "Invalid value " << pairAttributeValue_l.second << " in section " << sectionName_l
                                    << ": coeff value must be a double!\n";
                        std::exit(1);
                    }
                }
            }
            (*this)[constraintName_l] = constraint_l;

            if(emptyCstr_l)
            {
                std::cerr << "section " << sectionName_l << " defines an empty constraint.\n";
                std::exit(1);
            }
        }
    }

}

/**
 * \brief adds a binary variable to be created and links it to the corresponding variable
 *
 * \param oldVarName_p String name of the old variable
 * \param binVarName_p String name of the new binary variable to link to the old existing one
 *
 * adds an entry to the AdditionalConstraints::_variablesToBinarise
 */
void AdditionalConstraints::addVariableToBinarise(std::string oldVarName_p, std::string binVarName_p)
{
    if(!_binaryVariables.insert(binVarName_p).second)
    {
        std::cout << "Duplicate Binary variable name: " << binVarName_p << " was already added.\n";
        std::exit(0);
    }
    _variablesToBinarise[oldVarName_p] = binVarName_p;
}

/**
 * \brief getter for AdditionalConstraints::_variablesToBinarise
 *
 * \return std::map<std::string, std::string> whose value is the name of the binary variables to create
 * and whose key is the corresponding existing variable to link to
 */
std::map<std::string, std::string> AdditionalConstraints::getVariablesToBinarise()
{
    return _variablesToBinarise;
}