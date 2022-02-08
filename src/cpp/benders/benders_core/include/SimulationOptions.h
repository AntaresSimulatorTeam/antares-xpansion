#pragma once

#include "common.h"

class SimulationOptions
{
public:
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) type__ name__;
#include "SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

	SimulationOptions();
	SimulationOptions(const std::string &options_filename);

	void read(std::string const &file_name);
	void print(std::ostream &stream) const;
	BendersBaseOptions get_benders_options() const;

	void write_default();
	Str2Dbl _weights;

private:
	void set_weights();
};
