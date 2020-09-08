#include "AdditionalConstraintsReader.h"

#include <iostream>
#include <fstream>
#include <algorithm>

namespace
{

    //trim leading whitespaces
    std::string ltrim(const std::string& s)
    {
        size_t start = s.find_first_not_of(" \n\r\t\f\v");
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    //trim trailing whitespaces
    std::string rtrim(const std::string& s)
    {
        size_t end = s.find_last_not_of(" \n\r\t\f\v");
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    //trim leading and trailing whitespaces
    std::string trim(const std::string& s)
    {
        return rtrim(ltrim(s));
    }

}

std::string AdditionalConstraintsReader::illegal_chars = " \n\r\t\f\v-+=:[]()";

void AdditionalConstraintsReader::processSectionLine()
{
    if ( _line[_line.length()-1] != ']' )
    {
        std::cout << "line " << _lineNb << " : section line not ending with ']'.\n";
        std::exit(0);
    }

    _section = _line.substr(1,_line.find(']')-1);

    if (!_sections.insert(_section).second)
    {
        std::cout << "line " << _lineNb << " : duplicate section " << _section << "!\n";
        std::exit(0);
    }

    if(_section == "variables")
    {
        _foundName = true;
        _foundSign = true;
        _foundRHS = true;
    }
    else
    {
        _foundName = false;
        _foundSign = false;
        _foundRHS = false;
    }
}

void AdditionalConstraintsReader::processEntryLine()
{
    size_t delimiterIt_l = _line.find(" = ");
    if ((delimiterIt_l == std::string::npos))
    {
        std::cout << "line " << _lineNb << " : incorrect entry line format. Expected format 'attribute = value'!\n";
        std::exit(0);
    }
    std::string attribute_l = rtrim( _line.substr(0, delimiterIt_l) );
    std::string value_l = ltrim( _line.substr(delimiterIt_l+3) );

    size_t illegalCharIndex_l = attribute_l.find_first_of(AdditionalConstraintsReader::illegal_chars);
    if( illegalCharIndex_l != std::string::npos)
    {
        std::cout << "line " << _lineNb << " : Illegal character '" << attribute_l[illegalCharIndex_l] << "' in attribute name!\n";
        std::exit(0);
    }

    if(_values[_section].count(attribute_l))
    {
        std::cout << "line " << _lineNb << " : duplicate attribute " << attribute_l << "!\n";
        std::exit(0);
    }
    else
    {
        if((attribute_l == "name") || (_section == "variables"))
        {
            illegalCharIndex_l = value_l.find_first_of(AdditionalConstraintsReader::illegal_chars);
            if( illegalCharIndex_l != std::string::npos)
            {
                std::cout << "line " << _lineNb << " : Illegal character '" << value_l[illegalCharIndex_l] << "' in value!\n";
                std::exit(0);
            }
        }

        if(attribute_l == "name")
        {
            _foundName = true;
        }
        else if (attribute_l == "rhs")
        {
            _foundRHS = true;
        }
        else if (attribute_l == "sign")
        {
            _foundSign = true;

            if ( (value_l != "greater_or_equal") && (value_l != "less_or_equal") && (value_l != "equal") )
            {
                std::cout << "line " << _lineNb << " : Illegal sign value : " << value_l << "!\n";
                std::exit(0);
            }
        }

        _values[_section][attribute_l] = value_l;
    }
}


AdditionalConstraintsReader::AdditionalConstraintsReader(std::string  const & constraints_file_path)
{
    std::ifstream file_l(constraints_file_path);
    _line = "";
    _section = "";
    _lineNb = 0;

    _foundName = true;
    _foundSign = true;
    _foundRHS = true;

    while (std::getline(file_l, _line))
    {
        ++_lineNb;
        _line = trim(_line);
        std::transform(_line.begin(), _line.end(), _line.begin(),
                        [](unsigned char c){ return std::tolower(c); });

        if  ( (_line.length() == 0) || (_line[0] == '#') || (_line[0] == ';') )
        {//line is a comment or empty
            continue;
        }
        else if ( _line[0] == '[' )
        {//line is a section
            //check that previous section had defined a name, sign and rhs
            if(!_foundName || !_foundSign || !_foundRHS)
            {
                std::cout << "section " << _section << " is missing a name, sign or rhs attribute.\n";
                std::exit(0);
            }

            processSectionLine();
        }
        else
        {
            //check we have a valid section name
            if(_section == "")
            {
                std::cout << "Section line is required before line " << _lineNb << "!\n";
                std::exit(0);
            }

            processEntryLine();
        }
    }
}

std::map<std::string, std::string> AdditionalConstraintsReader::getVariablesSection()
{
    return _values["variables"];
}

std::set<std::string> AdditionalConstraintsReader::getSections()
{
    return _sections;
}

std::map<std::string, std::string> AdditionalConstraintsReader::getSection(std::string sectionName_p)
{
    return _values.at(sectionName_p);
}

std::string AdditionalConstraintsReader::getValue(std::string sectionName_p, std::string attributeName_p)
{
    return _values[sectionName_p].at(attributeName_p);
}