#pragma once
#include <string>

enum class BENDERSMETHOD
{
    BENDERS,
    BENDERS_BY_BATCH,
    BENDERS_OUTERLOOP,
    BENDERS_BY_BATCH_OUTERLOOP
};

inline std::string bendersmethod_to_string(BENDERSMETHOD method)
{
    using namespace std::string_literals;
    switch (method)
    {
    case BENDERSMETHOD::BENDERS:
        return "Benders"s;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
        return "Benders by batch"s;
    case BENDERSMETHOD::BENDERS_OUTERLOOP:
        return "Outerloop around Benders"s;
    case BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP:
        return "Outerloop around Benders by batch"s;
    default:
        return "Unknown"s;
    }
}
