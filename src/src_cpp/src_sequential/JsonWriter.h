
#pragma once

#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"

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
    void write(BendersTrace const & bendersTrace_p, BendersData const & bendersData_p);

    void dump(std::string const & filename_p);
};