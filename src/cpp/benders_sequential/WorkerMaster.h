#pragma once

#include "Worker.h"
#include "WorkerSlave.h"

/*!
* \class WorkerSlave
* \brief Class daughter of Worker Class, build and manage a master problem
*/
class WorkerMaster;
typedef std::shared_ptr<WorkerMaster> WorkerMasterPtr;

class WorkerMaster : public Worker {
public:
	int _id_alpha;
	std::vector<int> _id_alpha_i;

	WorkerMaster();
	WorkerMaster(Str2Int const & variable_map, std::string const & problem_name, BendersOptions const & options, int nslaves = 1);
	virtual ~WorkerMaster();

	void get(Point & x0, double & alpha, DblVector & alpha_i);
	void get_dual_values(std::vector<double> & dual);
	int get_number_constraint();

	void add_cut(Point const & s, Point const & x0, double const & rhs);
	void add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs);
	void add_dynamic_cut(Point const & s, double const & sx0, double const & rhs);
	void add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs);
	void delete_constraint(int const nrows);
	void fix_alpha(double const & bestUB);

private:

    void define_matval_mclind(const Point &s, std::vector<double> &matval, std::vector<int> &mclind) const;

    void define_rhs_with_master_variable(const Point &s, const Point &x0, const double &rhs, std::vector<double> &rowrhs) const;

    void define_rhs_from_sx0(const double &sx0, const double &rhs, std::vector<double> &rowrhs) const;

    void
    define_matval_mclind_for_index(const int i, const Point &s, std::vector<double> &matval, std::vector<int> &mclind) const;
};
