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

OutputWriter::OutputWriter() : _beginTime(std::time(0)),
                               _endTime(std::time(0))
{
}

OutputWriter::~OutputWriter()
{
}

void OutputWriter::updateBeginTime()
{
    _beginTime = std::time(0);
}

void OutputWriter::updateEndTime()
{
    _endTime = std::time(0);
}

std::string OutputWriter::getBegin()
{
    return timeToStr(&_beginTime);
}

std::string OutputWriter::getEnd()
{
    return timeToStr(&_endTime);
}

double OutputWriter::getDuration()
{
    return std::difftime(_endTime, _beginTime);
}
