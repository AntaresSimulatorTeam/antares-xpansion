#pragma once

#include <antares-xpansion/benders/plugins/BendersPlugin.h>

class NoOperationPlugin: public BendersPlugin
{
public:
    NoOperationPlugin();
    virtual ~NoOperationPlugin() = default;

    void OnBendersStart();
    void OnBendersEnd();

    void OnBendersIterationStart();
    void OnBendersIterationEnd();

    void OnBendersMasterResolutionStart();
    void OnBendersMasterResolutionEnd();

    void OnBendersMicroIterationStart();
    void OnBendersMicroIterationEnd();
};
