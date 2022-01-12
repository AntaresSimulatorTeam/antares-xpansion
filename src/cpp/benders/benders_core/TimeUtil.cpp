#include "TimeUtil.h"
#include <string>
#include <ctime>

namespace
{
    std::string timeToStr(const std::time_t *time_p)
    {
        char buffer_l[100];
        strftime(buffer_l, sizeof(buffer_l), "%d-%m-%Y %H:%M:%S", std::localtime(time_p));
        std::string strTime_l(buffer_l);

        return strTime_l;
    }
}
TimeUtil::TimeUtil() : _beginTime(std::time(0)),
                       _endTime(std::time(0))
{
}

TimeUtil::~TimeUtil()
{
}

void TimeUtil::updateBeginTime()
{
    _beginTime = std::time(0);
}

void TimeUtil::updateEndTime()
{
    _endTime = std::time(0);
}

std::string TimeUtil::getBegin()
{
    return timeToStr(&_beginTime);
}

std::string TimeUtil::getEnd()
{
    return timeToStr(&_endTime);
}

double TimeUtil::getDuration()
{
    return std::difftime(_endTime, _beginTime);
}
