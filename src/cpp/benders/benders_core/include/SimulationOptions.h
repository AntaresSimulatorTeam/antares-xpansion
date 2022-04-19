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
  explicit SimulationOptions(const std::string &options_filename);

  void read(std::string const &file_name);
  void print(std::ostream &stream) const;
  BendersBaseOptions get_benders_options() const;
  BaseOptions get_base_options() const;

  void write_default() const;
  Str2Dbl _weights;

 private:
  void set_weights();
};
