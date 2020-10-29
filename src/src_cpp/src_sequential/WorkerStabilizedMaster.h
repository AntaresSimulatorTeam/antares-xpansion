#pragma once

#include "WorkerMaster.h"

enum BundleStep {
	SERIOUS_STEP,
	NULL_STEP
};


class BundleIteration {
public:
	double _prediction;
	double _update;
	int _nb_serious_step;
	int _nb_null_step;
	int _nb_update;
	BundleStep _step;
	double _m;
	double _tol;
	double _lb;
	double _ub;
};

class WorkerStabilizedMaster : public WorkerMaster {
public:
public:
	WorkerStabilizedMaster();
	WorkerStabilizedMaster(std::map<std::string, int> const & variable_name, std::string const & problem_name, BendersOptions const & options, int nslaves=1);
	virtual ~WorkerStabilizedMaster();

	//void get(Point & x0, double & alpha);
	//
	//void write(int it);

	//void add_cut(Point const & s, Point const & x0, double rhs);
	//void add_cut_slave(int i, Point const & s, Point const & x0, double rhs);
public:

	void update_center(Point const &);

public:
	PointPtr _center;

	std::map<std::string, double> _weights;


};