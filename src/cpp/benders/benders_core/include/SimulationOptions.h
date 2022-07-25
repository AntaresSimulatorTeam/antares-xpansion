#pragma once

#include "common.h"

class SimulationOptions {
 public:
#define BENDERS_OPTIONS_MACRO(name__, type__, default__, \
                              deserialization_method__)  \
  type__ name__;
#include "SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

  SimulationOptions();
  explicit SimulationOptions(const std::filesystem::path &options_filename);
  explicit SimulationOptions(const std::string &options_filename, const std::string& construct_all_problems);
  void read(const std::filesystem::path &file_name);
  void print(std::ostream &stream) const;
  BendersBaseOptions get_benders_options() const;
  BaseOptions get_base_options() const;
  ExternalLoopOptions GetExternalLoopOptions() const;

  void write_default() const;
  Str2Dbl _weights;

 private:
  void set_weights();
  Json::Value get_value_from_json(const std::filesystem::path &file_name);

  class InvalidOptionFileException : public std::runtime_error {
   public:
    explicit InvalidOptionFileException(const std::string &arg);
  };
};
