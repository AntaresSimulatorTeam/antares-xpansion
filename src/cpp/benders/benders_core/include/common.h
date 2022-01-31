#pragma once

#ifdef _MSC_VER
#pragma warning(disable : 4267) // implicit conversion, possible loss of data
#endif

#include <tuple>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <memory>
#include <algorithm>
#include <thread>
#include <cmath>
//#include "Timer.h"
#include "core/ILogger.h"
// MultiSolver interface
#include "multisolver_interface/Solver.h"

struct Predicate;
typedef std::map<std::string, double> Point;

typedef std::shared_ptr<Point> PointPtr;

double const EPSILON_PREDICATE = 1e-8;

typedef std::set<std::string> problem_names;
typedef std::map<std::string, int> Str2Int;
typedef std::map<int, std::string> Int2Str;
typedef std::map<std::string, double> Str2Dbl;
typedef std::vector<int> IntVector;
typedef std::vector<char> CharVector;
typedef std::vector<double> DblVector;
typedef std::vector<std::string> StrVector;
typedef std::map<std::string, Str2Int> CouplingMap;

typedef std::map<std::string, IntVector> SlaveCutId;
typedef std::tuple<int, std::string, int, bool> ActiveCut;
typedef std::vector<ActiveCut> ActiveCutStorage;

typedef std::pair<std::string, std::string> mps_coupling;
typedef std::list<mps_coupling> mps_coupling_list;

struct Predicate
{
	bool operator()(PointPtr const &lhs, PointPtr const &rhs) const
	{
		return *lhs < *rhs;
	}
	bool operator()(Point const &lhs, Point const &rhs) const
	{
		Point::const_iterator it1(lhs.begin());
		Point::const_iterator it2(rhs.begin());

		Point::const_iterator end1(lhs.end());
		Point::const_iterator end2(rhs.end());

		while (it1 != end1 && it2 != end2)
		{
			if (it1->first != it2->first)
			{
				return it1->first < it2->first;
			}
			else
			{
				if (std::fabs(it1->second - it2->second) < EPSILON_PREDICATE)
				{
					++it1;
					++it2;
				}
				else
				{
					return it1->second < it2->second;
				}
			}
		}

		if (it1 == end1 && it2 == end2)
		{
			return false;
		}
		else
		{
			return (it1 == end1);
		}
	}
};

/*!
 *  \brief Stream output overloading for point
 *
 *  \param stream : stream output
 *
 *  \param rhs : point
 */
inline std::ostream &operator<<(std::ostream &stream, Point const &rhs)
{
	for (auto const &kvp : rhs)
	{
		if (kvp.second > 0)
		{
			if (kvp.second == 1)
			{
				stream << "+";
				stream << kvp.first;
			}
			else
			{
				stream << "+";
				stream << kvp.second;
				stream << kvp.first;
			}
		}
		else if (kvp.second < 0)
		{
			stream << kvp.second;
			stream << kvp.first;
		}
	}
	return stream;
}

/*!
 * \struct BendersData
 * \brief Structure used to manage every benders data
 */
struct BendersData
{
	int nbasis;
	double timer_slaves;
	double timer_master;
	double lb;
	double ub;
	double best_ub;
	int maxsimplexiter;
	int minsimplexiter;
	int deletedcut;
	int it;
	bool stop;
	double alpha;
	std::vector<double> alpha_i;
	double slave_cost;
	double invest_cost;
	int best_it;
	Point bestx;
	Point x0;
	Point min_invest;
	Point max_invest;
	int nslaves;
	double dnslaves;
	int master_status;
	double elapsed_time;
	StoppingCriterion stopping_criterion;
};

double norm_point(Point const &x0, Point const &x1);

std::ostream &operator<<(std::ostream &stream, std::vector<IntVector> const &rhs);

LogData bendersDataToLogData(const BendersData &data);

const std::string SLAVE_WEIGHT_CONSTANT("CONSTANT");
const std::string SLAVE_WEIGHT_UNIFORM("UNIFORM");

struct BendersBaseOptions
{
	int
		LOG_LEVEL,
		MAX_ITERATIONS,
		SLAVE_NUMBER;

	double
		ABSOLUTE_GAP,
		RELATIVE_GAP,
		SLAVE_WEIGHT_VALUE,
		TIME_LIMIT;

	bool
		AGGREGATION,
		TRACE,
		BOUND_ALPHA;

	std::string
		OUTPUTROOT,
		SLAVE_WEIGHT,
		MASTER_NAME,
		STRUCTURE_FILE,
		INPUTROOT,
		CSV_NAME,
		SOLVER_NAME;

	Str2Dbl weights;
};
