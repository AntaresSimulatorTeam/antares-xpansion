#pragma once

#include "common.h"

class SimulationOptions
{
public:
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) type__ name__;
#include "SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

	SimulationOptions();

	void read(std::string const &file_name);
	void print(std::ostream &stream) const;

	void write_default();

	Str2Dbl _weights;
	inline Str2Dbl weights() const
	{
		return _weights;
	}
};
