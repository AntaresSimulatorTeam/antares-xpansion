//
// Created by marechaljas on 16/06/22.
//

#pragma once

#include <filesystem>

#include "ActiveLinks.h"
#include "antares-xpansion/helpers/AntaresVersionProvider.h"
#include "LinkProblemsGenerator.h"
/*!
 * \struct LinkdataRecord
 * \brief struct describing a line in a linkdata file of antares
 */
struct LinkdataRecord {
  struct FileColumns {
    double directCapacity_;
    //! 2nd column of the file : indirect capacity of the link
    double indirectCapacity_;
    //! 3rd column of the file : direct hurdles cost
    double directHurdlesCost_;
    //! 4th column of the file : indirect hurdles cost
    double indirectHurdlesCost_;
    //! 5th column of the file : impedances
    double impedances_;
    //! 6th column of the file : loop flow
    double loopFlow_;
    //! 7th column of the file : power shift min.
    double pShiftMin_;
    //! 8th column of the file : power shift max.
    double pShiftMax_;
  };

  FileColumns fileColumns;
  /*!
   * \brief LinkdataRecord constructor
   */
  explicit LinkdataRecord();

  /*!
   * \brief LinkdataRecord constructor
   * versions
   *
   * \param fileColumns : LinkdataRecord::FileColumns object that hold columns
   * infos of the file
   */
  explicit LinkdataRecord(const FileColumns& afileColumns);

  /*!
   * \brief returns a one-line string describing the LinkdataRecord
   *
   * \param sep_p : delimiter used to separate record's attributes in the
   * created line string
   */
  std::string to_row(std::string const& sep_p) const;

  /*!
   * \brief update the capacities of the LinkdataRecord
   *
   * \param directCapacity_p : value to set to fileColumns.directCapacity_
   * \param indirectCapacity_p : value to set to fileColumns.indirectCapacity_
   */
  void updateCapacities(double directCapacity_p, double indirectCapacity_p);

  /*!
   * \brief populates the LinkdataRecord attributes for a string
   *
   * \param line_p : string describing the record
   */
  void fillFromRow(std::string const& line_p);

  /*!
   * \brief reset the LinkdataRecord attributes to 0
   */
  void reset();
};