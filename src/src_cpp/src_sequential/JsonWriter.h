
#pragma once

#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "common.h"

#include <json/writer.h>

class JsonWriter
{
private:
    std::time_t _beginTime;
    std::time_t _endTime;

    Json::Value _output;

    std::string getBegin();
    std::string getEnd();
    double getDuration();

public:
    JsonWriter();
    virtual ~JsonWriter();

    void updateBeginTime();
    void updateEndTime();

    void write(BendersOptions const & bendersOptions_p);
    void write(int const & nbWeeks_p, BendersTrace const & bendersTrace_p, BendersData const & bendersData_p);
    void write(int nbWeeks_p, double const & lb_p, double const & ub_p, double const & investCost_p, Point const & solution_p, bool const & optimality_p);

    void dump(std::string const & filename_p);
};
