#pragma once

#include <optional>
#include <string>

#include "BendersStructsDatas.h"

class IWorker
{
public:
    virtual ~IWorker() = default;
    virtual PlainData::SubProblemData solve(const std::string& problem_name) = 0;
    virtual void setBasis(const std::string& name,
                          const std::vector<int>& rstatus,
                          const std::vector<int>& cstatus)
      = 0;
    virtual std::optional<std::pair<std::vector<int>, std::vector<int>>>
    getBasis(const std::string& name) const = 0;
};
