//
// Created by marechaljas on 16/06/22.
//
#include "antares-xpansion/study-updater/LinkCapacitiesCSVWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkdataRecord.h"

LinkCapacitiesCSVWriter::LinkCapacitiesCSVWriter(
    const ActiveLink& link_p, const std::filesystem::path& linkPath) {
  auto link_folder = linkPath / link_p.get_linkor() / "capacities";
  auto direct_ntc = link_folder / (link_p.get_linkex() + "_direct.txt");
  auto indirect_ntc = link_folder / (link_p.get_linkex() + "_indirect.txt");
  file_direct_ = std::ofstream(direct_ntc);
  file_indirect_ = std::ofstream(indirect_ntc);
}
void LinkCapacitiesCSVWriter::WriteChronicleTimePoint(
    double direct_capacity, double indirect_capacity) {
  if (!new_timepoint_)
  {
    file_direct_ << "\t";
    file_indirect_ << "\t";
  }
  new_timepoint_ = false;
  file_direct_ << direct_capacity;
  file_indirect_ << indirect_capacity;
}
void LinkCapacitiesCSVWriter::FinishTimePoint() {
  file_direct_ << "\n";
  file_indirect_ << "\n";
  new_timepoint_ = true;
}
