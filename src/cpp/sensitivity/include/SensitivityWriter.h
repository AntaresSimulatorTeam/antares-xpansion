#pragma once

#include <json/writer.h>
#include "SensitivityOutputData.h"

const std::string
    ANTARES_C("antares"),
    VERSION_C("version"),
    ANTARES_XPANSION_C("antares_xpansion"),
    EPSILON_C("epsilon"),
    BEST_BENDERS_C("best benders cost"),
    OPT_DIR_C("optimization direction"),
    PB_TYPE_C("problem type"),
    MIN_C("min"),
    MAX_C("max"),
    CAPEX_C("capex"),
    PROJECTION_C("projection"),
    STATUS_C("status"),
    SYSTEM_COST_C("system cost"),
    OBJECTIVE_C("objective"),
    NAME_C("name"),
    INVEST_C("invest"),
    CANDIDATES_C("candidates"),
    SENSITIVITY_SOLUTION_C("sensitivity solutions");

class SensitivityWriter
{
private:
    std::string _filename;
    Json::Value _output;

    void write_sensitivity_output(SensitivityOutputData const &output_data);

public:
    SensitivityWriter() = delete;
    explicit SensitivityWriter(const std::string &json_filename);
    ~SensitivityWriter() = default;

    void dump();
    void end_writing(SensitivityOutputData const &output_data);
};