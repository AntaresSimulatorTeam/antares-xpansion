#pragma once

#include "common.h"

class BendersOptions
{
public:
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) type__ name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO

	BendersOptions();

	void read(std::string const &file_name);
	void print(std::ostream &stream) const;

	void write_default();

	Str2Dbl _weights;
	inline Str2Dbl weights() const
	{
		return _weights;
	}
};
