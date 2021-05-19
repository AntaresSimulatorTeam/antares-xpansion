
#pragma once

#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "common.h"

#include <json/writer.h>


/*!
* \class JsonWriter
* \brief JsonWriter class to describe the execuion session of an antares xpansion optimization in a json file
*/
class JsonWriter
{
private:
    // begin time of the optimization
    std::time_t _beginTime;
    // end time of the optimization
    std::time_t _endTime;

    // attributes of the optimization execution
    Json::Value _output;

/*!
*  returns the start of execution time as a string
*/
    std::string getBegin();

/*!
*  returns the end of execution time as a string
*/
    std::string getEnd();

/*!
*  \brief return a double indicating the exection time in seconds
*/
    double getDuration();

public:

/*!
*  \brief JsonWriter default constructor
*/
    JsonWriter();

/*!
*  \brief destructor of class JsonWriter
*/
    virtual ~JsonWriter();

/*!
*  \brief updates the execution begin time
*/
    void updateBeginTime();

/*!
*  \brief updates the end of execution time
*/
    void updateEndTime();

/*!
*  \brief saves the options of the benders algorithm to be later written to the json file
*
*  \param bendersOptions_p : set of options used for the optimization
*/
    void write(BendersOptions const & bendersOptions_p);

/*!
*  \brief saves some entries to be later written to the json file
*
*  \param nbWeeks_p : number of the weeks in the study
*  \param bendersTrace_p : trace to be written ie iterations details
*  \param bendersData_p : final benders data to get the best iteration
*/
    void write(int const & nbWeeks_p, BendersTrace const & bendersTrace_p, BendersData const & bendersData_p);

/*!
*  \brief  saves some entries to be later written to the json file
*
*  \param nbWeeks_p : number of the weeks in the study
*  \param lb_p : solution lower bound
*  \param ub_p : solution upper bound
*  \param investCost_p : investment cost
*  \param operationalCost_p : operational cost
*  \param overallCost_p : total cost, sum of invest and operational
*  \param solution_p : point giving the solution and the candidates
*  \param optimality_p : indicates if optimality was reached
*/
    void write(int nbWeeks_p, double const & lb_p, double const & ub_p, double const & investCost_p,
                double const& operationalCost_p, double const & overallCost_p,
                Point const & solution_p, bool const & optimality_p);

/*!
*  \brief write an a priori errored json output, overwritten if optimization ends
*/
    void write_failure();

/*!
*  \brief write the json data into a file
*
*  \param filename_p : name of the file to be written
*/
    void dump(std::string const & filename_p);
};
