#pragma once

#include <json/writer.h>
#include "SensitivityOutputData.h"

class SensitivityWriter
{
private:
    std::string _filename;
    Json::Value _output;

public:
    SensitivityWriter() = delete;
    SensitivityWriter(const std::string &json_filename);
    ~SensitivityWriter() = default;

    void dump();
    void write_sensitivity_output(SensitivityOutputData const &output_data);
    void end_writing(SensitivityOutputData const &output_data);
};