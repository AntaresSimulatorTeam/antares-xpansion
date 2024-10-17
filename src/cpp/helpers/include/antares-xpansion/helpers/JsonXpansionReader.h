#pragma once

#include <json/reader.h>

#include <filesystem>
/*!
 * \class JsonXpansionReader
 * \brief Class that reads a json file describing an antares-xpansion solution
 * \note does not consider if the json file was created from sequential, mpi or
 * merge optimizers
 */
class JsonXpansionReader {
 private:
  typedef std::map<std::string, double> Point;

 private:
  // number of the last iteration
  int _lastIterNb;

  // json file content
  Json::Value _input;

 public:
  /*!
   *  \brief JsonXpansionReader default constructor
   */
  JsonXpansionReader();

  /*!
   *  \brief JsonWriter default destructor
   */
  virtual ~JsonXpansionReader() = default;

  /*!
   *  \brief reads a json file describing the execution of an antares xpansion
   * optimisation and populates the jsonXpansionReader class attributes with the
   * data from the file
   *
   *  \param filename_p name of the json file to read from
   */
  void read(const std::filesystem::path& filename_p);

  /*!
   *  \brief returns the index of the best iteration indicated in the json
   * output file \return iteration entry in the solution object of the read json
   * if exists, Json::nullValue otherwise
   */
  int getBestIteration() const;

  /*!
   *  \brief returns the index of the last iteration indicated in the json
   * output file \return index of the last iteration in the read json if
   * possible, Json::nullValue otherwise
   */
  int getLastIteration() const;

  /*!
   *  \brief returns the solution to the xpansion problem indicated in the json
   * output file
   */
  Point getSolutionPoint() const;
};
