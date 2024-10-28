#include "antares-xpansion/lpnamer/problem_modifier/LinkdataRecord.h"

LinkdataRecord::LinkdataRecord()
    : fileColumns{0, 0, 0, 0,
                                                                 0, 0, 0, 0} {}

LinkdataRecord::LinkdataRecord(const FileColumns& afileColumns)
    : fileColumns(afileColumns) {}

void LinkdataRecord::reset() {
  fileColumns.directCapacity_ = 0;
  fileColumns.indirectCapacity_ = 0;
  fileColumns.directHurdlesCost_ = 0;
  fileColumns.indirectHurdlesCost_ = 0;
  fileColumns.impedances_ = 0;
  fileColumns.loopFlow_ = 0;
  fileColumns.pShiftMin_ = 0;
  fileColumns.pShiftMax_ = 0;
}

void LinkdataRecord::fillFromRow(std::string const& line_p) {
  std::istringstream iss_l(line_p);

  iss_l >> fileColumns.directCapacity_;
  iss_l >> fileColumns.indirectCapacity_;
  iss_l >> fileColumns.directHurdlesCost_;
  iss_l >> fileColumns.indirectHurdlesCost_;
  iss_l >> fileColumns.impedances_;
  iss_l >> fileColumns.loopFlow_;
  iss_l >> fileColumns.pShiftMin_;
  iss_l >> fileColumns.pShiftMax_;
}

void LinkdataRecord::updateCapacities(double directCapacity_p,
                                      double indirectCapacity_p) {
  fileColumns.directCapacity_ = directCapacity_p;
  fileColumns.indirectCapacity_ = indirectCapacity_p;
}

std::string LinkdataRecord::to_row(std::string const& sep_p) const {
  std::string row_l = std::to_string(fileColumns.directCapacity_) + sep_p +
                      std::to_string(fileColumns.indirectCapacity_) + sep_p +
                      std::to_string(fileColumns.directHurdlesCost_) + sep_p +
                      std::to_string(fileColumns.indirectHurdlesCost_) + sep_p +
                      std::to_string(fileColumns.impedances_) + sep_p +
                      std::to_string(fileColumns.loopFlow_) + sep_p +
                      std::to_string(fileColumns.pShiftMin_) + sep_p +
                      std::to_string(fileColumns.pShiftMax_);


  return row_l;
}
