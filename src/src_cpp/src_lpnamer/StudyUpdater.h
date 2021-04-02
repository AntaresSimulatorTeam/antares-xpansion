#pragma once

#include "IntercoDataMps.h"


/*!
* \class StudyUpdater
* \brief Class that updates an antares study after an antares-xpansion execution
*/
class StudyUpdater
{
private:
    // folder containing the links files in the antares study
    static std::string linksSubfolder_;
    // path to the antares study
    std::string studyPath_;
    // path to the links folder
    std::string linksPath_;
    // antares version
    int antaresVersion_;

public:

/*!
 * \brief constructor of class StudyUpdater
 *
 * \param studyPath_p : path to the antares study folder
 */
    explicit StudyUpdater(std::string const & studyPath_p);

/*!
 * \brief default destructor of calass StudyUpdater
 */
    virtual ~StudyUpdater();

/*!
 * \brief getter for attribute StudyUpdater::antaresVersion_
 */
    int getAntaresVersion() const;

/*!
 * \brief reads the antares version from the "study.antares" file and set StudyUpdater::antaresVersion_
 */
    void readAntaresVersion();

/*!
 * \brief returns the path to the linkdata file related to a candidate
 *
 * \param candidate_p : candidate for which the datalink file path will be returned
 */
    std::string getLinkdataFilepath(Candidate const & candidate_p) const;

/*!
 * \brief computes the new capacities of related to a candidate
 *
 * \param investment_p : investment to consider for the candidate
 * \param candidate_p : candidate for which the capacities will be computed
 * \param timepoint_p : timepoint where the capcities will be computed
 *
 * \return a pair of the computed direct and indirect capacities
 */
    std::pair<double, double> computeNewCapacities(double investment_p, Candidate & candidate_p, int timepoint_p) const;

/*!
 * \brief updates the linkdata file for a given candidate based on a given investment
 *
 * \param candidate_p : candidate for which the linkdata file will be updated
 * \param investment_p : value of investment to consider for the update
 *
 * \return 1 if the update fails, 0 otherwise
 */
    int updateLinkdataFile(Candidate candidate_p, double investment_p) const;

/*!
 * \brief updates the linkdata files for multiple candidates from a solution Point
 *
 * \param candidates_p : the candidates for which the linkdata file will be updated
 * \param investments_p : a map giving investments per candidate
 *
 * \return number of candidates we failed to update
 */
    int update(Candidates const & candidates_p, std::map<std::string, double> investments_p) const;

/*!
 * \brief updates the linkdata files for multiple candidates from a json file
 *
 * \param candidates_p : the candidates for which the linkdata file will be updated
 * \param jsonPath_p : path to a json file to read the investment solution from
 *
 * \return number of candidates we failed to update
 */
    int update(Candidates const & candidates_p, std::string const & jsonPath_p) const;
};

/*!
* \struct LinkdataRecord
* \brief struct describing a line in a linkdata file of antares
*/
struct LinkdataRecord
{
    //! should be set to true if antares version is 700 or more recent
    const bool modernAntaresVersion_;

    //! 1st column of the file : direct capacity of the link
    double directCapacity_;
    //! 2nd column of the file : indirect capacity of the link
    double indirectCapacity_;
    //! 3rd column of the file : direct hurdles cost
    double directHurdlesCost_;
    //! 4th column of the file : indirect hurdles cost
    double indirectHurdlesCost_;
    //! 5th column of the file : impedances
    double impedances_;
    //! 6th column of the file : loop flow
    double loopFlow_;
    //! 7th column of the file : power shift min.
    double pShiftMin_;
    //! 8th column of the file : power shift max.
    double pShiftMax_;

/*!
 * \brief LinkdataRecord constructor
 *
 * \param modernAntaresVersion_p : value to set to modernAntaresVersion_p
 */
    explicit LinkdataRecord(bool modernAntaresVersion_p);


/*!
 * \brief LinkdataRecord constructor to use with modern antares studies versions
 *
 * \param directCapacity_p : value to set to directCapacity_
 * \param indirectCapacity_p : value to set to indirectCapacity_
 * \param directHurdlesCost_p : value to set to directHurdlesCost_
 * \param indirectHurdlesCost_p : value to set to indirectHurdlesCost_
 * \param impedances_p : value to set to impedances_
 * \param loopFlow_p : value to set to loopFlow_
 * \param pShiftMin_p : value to set to pShiftMin_
 * \param pShiftMax_p : value to set to pShiftMax_
 */
    LinkdataRecord(double directCapacity_p, double indirectCapacity_p,
        double directHurdlesCost_p, double indirectHurdlesCost_p, double impedances_p,
        double loopFlow_p, double pShiftMin_p, double pShiftMax_p);

/*!
 * \brief LinkdataRecord constructor to use with old antares studies versions
 *
 * \param directCapacity_p : value to set to directCapacity_
 * \param indirectCapacity_p : value to set to indirectCapacity_
 * \param directHurdlesCost_p : value to set to directHurdlesCost_
 * \param indirectHurdlesCost_p : value to set to indirectHurdlesCost_
 * \param impedances_p : value to set to impedances_
 */
    LinkdataRecord(double directCapacity_p, double indirectCapacity_p,
        double directHurdlesCost_p, double indirectHurdlesCost_p, double impedances_p);

/*!
 * \brief returns a one-line string describing the LinkdataRecord
 *
 * \param sep_p : delimiter used to separate record's attributes in the created line string
 */
    std::string to_row(std::string const & sep_p) const;

/*!
 * \brief update the capacities of the LinkdataRecord
 *
 * \param directCapacity_p : value to set to directCapacity_
 * \param indirectCapacity_p : value to set to indirectCapacity_
 */
    void updateCapacities(double directCapacity_p, double indirectCapacity_p);

/*!
 * \brief populates the LinkdataRecord attributes for a string
 *
 * \param line_p : string describing the record
 */
    void fillFromRow(std::string const &  line_p);

/*!
 * \brief reset the LinkdataRecord attributes to 0
 */
    void reset();
};
