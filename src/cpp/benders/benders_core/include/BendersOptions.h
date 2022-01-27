#pragma once

#include "common.h"
#include <map>
#include <utility>
#include <string>
struct BendersOptionsData
{
	int
		// Determine the degree of detail of the output, from 1 to 3
		LOG_LEVEL = 1,
		// Maximum number of iterations accepted
		MAX_ITERATIONS = -1,
		// Number of slaves to use to solve the problem
		SLAVE_NUMBER = -1;

	double
		// Absolute required level of precision
		ABSOLUTE_GAP = 1,
		// Absolute required level of precision
		RELATIVE_GAP = 1e-12,
		// If SLAVE_WEIGHT is CONSTANT, set here the divisor required
		SLAVE_WEIGHT_VALUE = 1,
		// TIME_LIMIT
		TIME_LIMIT = 1e12;

	bool
		// True if cuts need to be aggregated, false otherwise
		AGGREGATION = false,
		// True if a trace should be built, false otherwise
		TRACE = true,
		// True if alpha needs to be bounded by best upper bound, false otherwise
		BOUND_ALPHA = true;

	std::string
		// Path to the folder where output files should be printed
		OUTPUTROOT = ".",
		// UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight
		SLAVE_WEIGHT = "CONSTANT",
		// Name of the master problem file, if different from 'master'
		MASTER_NAME = "master",
		// Number of slaves to use to solve the problem
		STRUCTURE_FILE = "structure.txt",
		// Path to the folder where input files are stored
		INPUTROOT = ".",
		// Name of the csv output file
		CSV_NAME = "benders_output_trace",
		// Name of solver to use
		SOLVER_NAME = "COIN",
		// json file in output/expansion/
		JSON_FILE = ".";
};
class BendersOptions : public BendersOptionsData
{
public:
	// #define BENDERS_OPTIONS_MACRO(name__, type__, default__) type__ name__;
	// #include "BendersOptions.hxx"
	// #undef BENDERS_OPTIONS_MACRO

	BendersOptions();

	void read(std::string const &file_name);
	void print(std::ostream &stream) const;

	void write_default();

	std::string get_master_path() const;
	std::string get_structure_path() const;
	std::string get_slave_path(std::string const &slave_name) const;

	double slave_weight(int nslaves, std::string const &) const;

	Str2Dbl _weights;

private:
	std::map<std::string, void *> __nameToMember;
	void _initNameToMember();
	template <typename T>
	inline bool is_Tptr(void *ptr, T *t) const
	{
		return (t = reinterpret_cast<T *>(ptr)) != nullptr;
	}
};
