
#pragma once

#include "Timer.h"
#include <ctime>

class TimeUtil
{
public:
    /*!
     *  \brief TimeUtil default constructor
     */
    TimeUtil();

    /*!
     *  \brief destructor of class TimeUtil
     */
    virtual ~TimeUtil();
    /*!
     *  \brief updates the execution begin time
     */
    virtual void updateBeginTime();

    /*!
     *  \brief updates the end of execution time
     */
    virtual void updateEndTime();

    // attributes of the optimization execution

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

private:
    // begin time of the optimization
    std::time_t _beginTime;
    // end time of the optimization
    std::time_t _endTime;
};