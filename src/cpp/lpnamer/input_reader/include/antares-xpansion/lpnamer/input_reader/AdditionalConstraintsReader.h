#pragma once

#include <map>
#include <set>
#include <string>

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

/*!
 *  \struct AdditionalConstraintsReader
 *  \brief candidate exclusion constraints reading structure
 *
 */
struct AdditionalConstraintsReader
{
    /*!
     *  characters to forbid in variables and constraints names
     */
    static std::string illegal_chars;

private:
    //! set of section names contained between [] in the ini file
    std::set<std::string> _sections;
    //! map containing the string value per section per attribute _values[section
    //! name][attribute]
    std::map<std::string, std::map<std::string, std::string>> _values;

    //! the section that is being currently processed
    std::string _section = "";
    //! line string that is being currently processed
    std::string _line = "";
    //! number of the line that is being currently processed
    int _lineNb = 0;
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;

public:
    /*!
     *  default constructor for struct AdditionalConstraintsReader
     */
    AdditionalConstraintsReader(
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger):
        logger_(logger)
    {
    }

    /*!
     * \brief AdditionalConstraintsReader from an inifile
     *
     * \param constraints_file_path String name of the file to use to read the
     * additional constraints
     */
    explicit AdditionalConstraintsReader(
      const std::string& constraints_file_path,
      std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger);

    /*!
     * \brief return the section defining the binary variables to add
     */
    const std::map<std::string, std::string>& getVariablesSection();

    /*!
     * \brief returns the set of the names of sections defined in the file
     *
     * \param constraints_file_path String name of the file to use to read the
     * additional constraints
     */
    const std::set<std::string>& getSections() const;

    /*!
     * \brief returns a section
     *
     * \param sectionName_p String name of the section to return
     *
     * \throw if the section does not exist
     */
    const std::map<std::string, std::string>& getSection(const std::string& sectionName_p) const;

private:
    /*!
     * \brief process a section line ie a line that indicates a section name
     *
     * checks that the line ends with "]" and that the section name is not
     * duplicate updates the _section attribute
     */
    void processSectionLine();

    /*!
     * \brief processes an entry line ie that indicates an attribute and a value
     *
     * adds the entry to the values map
     */
    void processEntryLine();
};
