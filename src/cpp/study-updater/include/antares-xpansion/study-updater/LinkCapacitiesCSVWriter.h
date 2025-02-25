//
// Created by marechaljas on 16/06/22.
//

#pragma once
#include "antares-xpansion/lpnamer/problem_modifier/LinkdataRecord.h"
#include "antares-xpansion/study-updater/LinkParametersCSVOverwriter.h"
#include "antares-xpansion/study-updater/StudyUpdater.h"

class LinkCapacitiesCSVWriter
{
public:
    LinkCapacitiesCSVWriter(const ActiveLink& link_p, const std::filesystem::path& linkPath);

    void WriteChronicleTimePoint(double direct_capacity, double indirect_capacity);

    void FinishTimePoint();

private:
    std::ofstream file_direct_;
    std::ofstream file_indirect_;
    bool new_timepoint_ = true;
};
