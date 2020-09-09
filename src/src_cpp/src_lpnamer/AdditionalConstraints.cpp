
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
            std::map<std::string, std::string> const & constarintsSection_l = reader_l.getSection(sectionName_l);

            std::string constraintName_l = "";
            std::string constraintSign_l = "";
            double constraintRHS_l = 0;

            //check that section has defined a unique constraint name
            auto temporatyIterator_l = constarintsSection_l.find("name");
            if( temporatyIterator_l == constarintsSection_l.end())
            {
                std::cout << "section " << sectionName_l << " is missing a name.\n";
                std::exit(1);
            }
            else
            {
                constraintName_l = temporatyIterator_l->second;
                if(this->count(constraintName_l))
                {
                    std::cout << "Duplicate constraint name " << constraintName_l << ".\n";
                    std::exit(1);
                }
            }

            //check that section has defined a sign
            temporatyIterator_l = constarintsSection_l.find("sign");
            if(temporatyIterator_l == constarintsSection_l.end())
            {
                std::cout << "section " << sectionName_l << " is missing a sign.\n";
                std::exit(1);
            }
            else
            {
                constraintSign_l = temporatyIterator_l->second;
            }

            //check that section has defined a rhs
            temporatyIterator_l = constarintsSection_l.find("rhs");
            if(temporatyIterator_l == constarintsSection_l.end())
            {
                std::cout << "section " << sectionName_l << " is missing a rhs.\n";
                std::exit(1);
            }
            else
            {
                try
                {
                    std::string::size_type sz;
                    constraintRHS_l = std::stod(temporatyIterator_l->second, &sz);
                }
                catch(const std::invalid_argument& e)
                {
                    std::cerr << "Invalid value " << temporatyIterator_l->second << " in section " << sectionName_l
                                << ": rhs value must be a double!\n";
                    std::exit(1);
                }
            }

            //create and fill the constraint
            AdditionalConstraint constraint_l(sectionName_l, constraintName_l, constraintSign_l, constraintRHS_l);
            bool emptyCstr_l = true;
            for(auto pairAttributeValue_l : constarintsSection_l)
            {
                if(pairAttributeValue_l.first == "name" || pairAttributeValue_l.first == "rhs" || pairAttributeValue_l.first == "sign")
                {
                    continue;
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

            if(emptyCstr_l)
            {
                std::cerr << "section " << sectionName_l << " defines an empty constraint.\n";
                std::exit(1);
            }

            (*this)[constraintName_l] = constraint_l;
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
        std::exit(1);
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
