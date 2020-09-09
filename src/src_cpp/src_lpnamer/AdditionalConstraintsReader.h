#pragma once

#include <string>
#include <map>
#include <set>


/*!
 *  \struct AdditionalConstraintsReader
 *  \brief candidate exclusion constraints reading structure
 *
 */
struct AdditionalConstraintsReader
{

    //characters to forbid in variables and constraints names
    static std::string illegal_chars;

private:
	//set of variables to which a binary corresponding variable will be created
    std::set<std::string> _sections;
    std::map<std::string, std::map<std::string, std::string>> _values;

    std::string _section;
    std::string _line;
    int _lineNb;

public:
	AdditionalConstraintsReader():
    _section(""),
    _line(""),
    _lineNb(0)
    {
	}

	explicit AdditionalConstraintsReader(std::string  const & constraints_file_path);


    std::map<std::string, std::string> const & getVariablesSection();
    std::set<std::string> getSections();
    std::map<std::string, std::string> const & getSection(std::string const & sectionName_p);

private:
    void processSectionLine();
    void processEntryLine();
};
