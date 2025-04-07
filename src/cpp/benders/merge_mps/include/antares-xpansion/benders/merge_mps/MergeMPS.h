#pragma once

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/solver_utils.h"


class MergeMPS
{
public:
    MergeMPS(MergeMPSOptions options, Logger logger, std::shared_ptr<Output::OutputWriter> writer);

    void launch();

    double slave_weight(int nslaves, const std::string& name) const;

    MergeMPSOptions _options;
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;
};
