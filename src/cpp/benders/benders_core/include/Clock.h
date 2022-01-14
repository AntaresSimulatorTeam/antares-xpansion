
#pragma once

#include "Timer.h"
#include <ctime>


class Clock
{
public:
    Clock() = default;
    virtual ~Clock() = default;

    virtual std::time_t getTime();
};