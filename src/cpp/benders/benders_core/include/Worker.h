#pragma once

#include "common.h"

class Worker;
typedef std::shared_ptr<Worker> WorkerPtr;

/*!
 * \class Worker
 * \brief Mother-class Worker
 *
 *  This class opens and sets a problem from a mps and a mapping variable map
 */

class Worker
{
public:
	Worker() = default;
	void init(Str2Int const &variable_map, std::string const &path_to_mps,
			  std::string const &solver_name, int log_level);
	virtual ~Worker() = default;

	void get_value(double &lb) const;

	void get_splex_num_of_ite_last(int &result) const;

	void free();

public:
	std::string _path_to_mps;
	Str2Int _name_to_id; /*!< Link between the variable name and its identifier */
	Int2Str _id_to_name; /*!< Link between the identifier of a variable and its name*/

public:
	void solve(int &lp_status, const std::string &outputroot) const;

public:
	SolverAbstract::Ptr _solver = nullptr; /*!< Problem stocked in the instance Worker*/
	bool _is_master = false;
};