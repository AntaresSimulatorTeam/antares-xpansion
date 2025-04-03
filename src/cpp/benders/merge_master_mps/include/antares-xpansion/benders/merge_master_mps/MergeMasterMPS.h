#pragma once

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/solver_utils.h"


struct MergeMasterMPSOptions
{
    std::string INPUTROOT;
    std::string OUTPUTROOT;
    std::string STRUCTURE_FILE;
    std::string SOLVER_TO_USE = "CBC";
    int LOG_LEVEL = 1;
};



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
