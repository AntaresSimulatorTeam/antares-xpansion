
#pragma once

#include <json/writer.h>

#include <filesystem>
#include <fstream>

#include "antares-xpansion/xpansion_interfaces/Clock.h"
#include "antares-xpansion/xpansion_interfaces/OutputWriter.h"

namespace Output {

/*!
 * \class JsonWriter
 * \brief JsonWriter class to describe the execuion session of an antares
 * xpansion optimization in a json file
 */
class JsonWriter : public OutputWriter {
 private:
  std::shared_ptr<Clock> _clock;
  std::filesystem::path _filename;
  std::ofstream _jsonOut_l;
  // attributes of the optimization execution
  Json::Value _output;
  void write_iterations(const IterationsData &iterations_data);

  void _open_file();

 public:
  /*!
   *  \brief JsonWriter default constructor
   */
  JsonWriter() = delete;

  JsonWriter(std::shared_ptr<Clock> p_clock,
             const std::filesystem::path &json_filename);

  JsonWriter(const std::filesystem::path &json_filename,
             const Json::Value &json_file_content);

  /*!
   *  \brief destructor of class JsonWriter
   */
  virtual ~JsonWriter() = default;

  /*!
   *  \brief updates the execution begin time
   */
  void updateBeginTime() override;

  /*!
   *  \brief updates the end of execution time
   */
  void updateEndTime() override;

  virtual void update_solution(const SolutionData &solution_data);

  /*!
   *  \brief write the json data into a file
   */
  virtual void dump();

  /*!
   * \brief initialize outputs
   * \param options : set of options used for the optimization
   */
  void initialize();

  void end_writing(const IterationsData &iterations_data);

  void write_iteration(const Iteration &iteration_data,
                       const size_t iteration_num) override;
  void write_solver_name(const std::string &solver_name) override;
  void write_master_name(const std::string &master_name) override;
  void write_log_level(const int log_level) override;
  void write_solution(const SolutionData &solution) override;
  void write_nbweeks(const int nb_weeks) override;
  void write_duration(const double duration) override;
  std::string solution_status() const override;
  void WriteProblem(const ProblemData &problem_data) override;
};
}  // namespace Output