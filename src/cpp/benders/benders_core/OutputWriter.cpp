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

void OutputWriter::initialize(BendersOptions options)
{
    _filename = options.OUTPUTROOT + PATH_SEPARATOR + options.JSON_NAME + ".json";
    write_failure();
    dump();

    write(options);
    updateBeginTime();
}

void OutputWriter::end_writing(int const &nbWeeks_p, BendersTrace const &bendersTrace_p, BendersData const &bendersData_p, double const &min_abs_gap, double const &min_rel_gap, double const &max_iter, std::string const &filename_p)
{
    updateEndTime();
    write(nbWeeks_p, bendersTrace_p, bendersData_p, min_abs_gap, min_rel_gap, max_iter);
    dump();
}