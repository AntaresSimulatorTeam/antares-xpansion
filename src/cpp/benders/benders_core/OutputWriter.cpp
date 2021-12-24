#include "OutputWriter.h"
#include "config.h"

namespace
{
    std::string timeToStr(std::time_t *time_p)
    {
        char buffer_l[100];
        strftime(buffer_l, sizeof(buffer_l), "%d-%m-%Y %H:%M:%S", std::localtime(time_p));
        std::string strTime_l(buffer_l);

        return strTime_l;
    }
}
namespace Output
{
    Time::Time() : _beginTime(std::time(0)),
                   _endTime(std::time(0))
    {
    }

    Time::~Time()
    {
    }

    void Time::updateBeginTime()
    {
        _beginTime = std::time(0);
    }

    void Time::updateEndTime()
    {
        _endTime = std::time(0);
    }

    std::string Time::getBegin()
    {
        return timeToStr(&_beginTime);
    }

    std::string Time::getEnd()
    {
        return timeToStr(&_endTime);
    }

    double Time::getDuration()
    {
        return std::difftime(_endTime, _beginTime);
    }

    OutputWriter::OutputWriter() : _time(Time())
    {
    }

}