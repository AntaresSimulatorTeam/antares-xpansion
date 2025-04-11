#pragma once

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/solver_utils.h"


typedef BaseOptions MergeMasterMPSOptions;



class MergeMasterMPS
{
public:
    MergeMasterMPS(MergeMasterMPSOptions options, 
                    Logger logger, 
                    std::shared_ptr<Output::OutputWriter> writer = build_void_writer());

    void launch();

    MergeMasterMPSOptions _options;
    Logger _logger;
    std::shared_ptr<Output::OutputWriter> _writer;
};
