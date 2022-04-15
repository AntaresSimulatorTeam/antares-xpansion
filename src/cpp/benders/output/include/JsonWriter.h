
#pragma once

#include <json/writer.h>

#include <filesystem>

#include "Clock.h"
#include "OutputWriter.h"

inline void localtime_platform(const std::time_t &time_p,
                               struct tm &local_time) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  localtime_s(&local_time, &time_p);
#else  // defined(__unix__) || (__APPLE__)
  localtime_r(&time_p, &local_time);
#endif
}
namespace clock_utils {
std::string timeToStr(const std::time_t &time_p);
}
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
  // attributes of the optimization execution
  Json::Value _output;

  /*!
   *  \brief updates the execution begin time
   */
  virtual void updateBeginTime();

  /*!
   *  \brief updates the end of execution time
   */
  virtual void updateEndTime();
  virtual void write_iterations(const IterationsData &iterations_data);

 public:
  /*!
   *  \brief JsonWriter default constructor
   */
  JsonWriter() = delete;

  JsonWriter(std::shared_ptr<Clock> p_clock,
             const std::filesystem::path &json_filename);

  /*!
   *  \brief destructor of class JsonWriter
   */
  virtual ~JsonWriter() = default;

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

  void write_solver_name(const std::string &solver_name) override;
  void write_master_name(const std::string &master_name) override;
  void write_log_level(const int log_level) override;
};
}  // namespace Output