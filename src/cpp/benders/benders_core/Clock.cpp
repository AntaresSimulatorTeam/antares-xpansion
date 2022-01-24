#include "Clock.h"
#include <string>
#include <ctime>


std::time_t Clock::getTime()
{
    std::time_t beginTime = std::time(nullptr);
    return beginTime;
}

