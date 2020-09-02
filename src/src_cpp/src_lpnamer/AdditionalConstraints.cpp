
#include <iostream>

#include "AdditionalConstraints.h"
#include "AdditionalConstraintsReader.h"

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
    for(auto sectionName_l : reader_l.getSections())
    {
        if (sectionName_l != "variables")
        {
            AdditionalConstraint constraint_l(sectionName_l);
            std::string constraintName_l;
            for(auto pairAttributeValue_l : reader_l.getSection(sectionName_l))
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
                    std::string::size_type sz;
                    double rhs_l = std::stod(pairAttributeValue_l.second, &sz);
                    constraint_l.setRHS(rhs_l);
                }
                else if(pairAttributeValue_l.first == "sign")
                {
                    constraint_l.setSign(pairAttributeValue_l.second);
                }
                else
                {
                    std::string::size_type sz;
                    double coeff_l = std::stod(pairAttributeValue_l.second, &sz);
                    constraint_l.setCoeff(pairAttributeValue_l.first, coeff_l);
                }
            }
            (*this)[constraintName_l] = constraint_l;
        }
    }

}

void AdditionalConstraints::addVariableToBinarise(std::string oldVarName_p, std::string binVarName_p)
{
    if(!_binaryVariables.insert(binVarName_p).second)
    {
        std::cout << "Duplicate Binary variable name: " << binVarName_p << " was already added.\n";
        std::exit(0);
    }
    _variablesToBinarise[oldVarName_p] = binVarName_p;
}

std::map<std::string, std::string> AdditionalConstraints::getVariablesToBinarise()
{
    return _variablesToBinarise;
}