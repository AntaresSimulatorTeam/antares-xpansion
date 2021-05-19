#include "WorkerMaster.h"

#include "ortools_utils.h"

WorkerMaster::WorkerMaster() {
}


WorkerMaster::~WorkerMaster() {
}

/*!
*  \brief Return optimal variables of a problem
*
*  Set optimal variables of a problem which has the form (min(x,alpha) : f(x) + alpha)
*
*  \param x0 : reference to an empty map list
*
*  \param alpha : reference to an empty double
*/
void WorkerMaster::get(Point & x0, double & alpha, DblVector & alpha_i) {
	x0.clear();
	std::vector<double> ptr;
	ORTgetlpsolution(*_solver, ptr);
	assert(_id_alpha_i.back()+1 == ptr.size());
	for (auto const & kvp : _id_to_name) {
		x0[kvp.second] = ptr[kvp.first];
	}
	alpha = ptr[_id_alpha];
	for (int i(0); i < _id_alpha_i.size(); ++i) {
		alpha_i[i] = ptr[_id_alpha_i[i]];
	}
}

/*!
*  \brief Set dual values of a problem in a vector
*
*  \param dual : reference to a vector of double
*/
void WorkerMaster::get_dual_values(std::vector<double> & dual) {
	ORTgetlpdual(*_solver, dual);
}

/*!
*  \brief Return number of constraint in a problem
*/
int WorkerMaster::get_number_constraint() {
	return _solver->NumConstraints();
}

/*!
*  \brief Delete nrows last rows of a problem
*
*  \param nrows : number of rows to delete
*/
void WorkerMaster::delete_constraint(int const nrows) {
	std::vector<int> mindex(nrows, 0);
	int const nconstraint(get_number_constraint());
	for (int i(0); i < nrows; i++) {
		mindex[i] = nconstraint - nrows + i;
	}
	// XPRSdelrows(_xprs, nrows, mindex.data());
	ORTdeactivaterows(*_solver, mindex); //rows are not really deleted
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut(Point const & s, Point const & x0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	// int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	// std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mstart = {0};
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;

	size_t mclindCnt_l(0);
	for (auto const & kvp : _name_to_id) {
		rowrhs.front() += (s.find(kvp.first)->second * x0.find(kvp.first)->second);
		mclind[mclindCnt_l] = kvp.second;
		matval[mclindCnt_l] = s.find(kvp.first)->second;
		++mclindCnt_l;
	}

	mclind.back() = _id_alpha;
	matval.back() = -1;

	ORTaddrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

/*!
*  \brief Add benders cut to a problem
*
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_dynamic_cut(Point const & s, double const & sx0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	// int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	// std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mstart = {0};
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	rowrhs.front() += sx0;

	size_t mclindCnt_l(0);
	for (auto const & kvp : _name_to_id) {
		mclind[mclindCnt_l] = kvp.second;
		matval[mclindCnt_l] = s.find(kvp.first)->second;
		++mclindCnt_l;
	}

	mclind.back() = _id_alpha;
	matval.back() = -1;

	ORTaddrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

/*!
*  \brief Add benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param sx0 : subgradient times x0
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut_by_iter(int const i, Point const & s, double const & sx0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	// int nrows(1);
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	// std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mstart = {0};
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;
	rowrhs.front() += sx0;

	size_t mclindCnt_l(0);
	for (auto const & kvp : _name_to_id) {
		mclind[mclindCnt_l] = kvp.second;
		matval[mclindCnt_l] = s.find(kvp.first)->second;
		++mclindCnt_l;
	}
	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;

	ORTaddrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}


/*!
*  \brief Add one benders cut to a problem
*
*  \param i : identifier of a slave problem
*  \param s : optimal slave variables
*  \param x0 : optimal Master variables
*  \param rhs : optimal slave value
*/
void WorkerMaster::add_cut_slave(int i, Point const & s, Point const & x0, double const & rhs) {
	// cut is -rhs >= alpha  + s^(x-x0)
	int ncoeffs(1 + (int)_name_to_id.size());
	std::vector<char> rowtype(1, 'L');
	std::vector<double> rowrhs(1, 0);
	std::vector<double> matval(ncoeffs, 1);
	// std::vector<int> mstart(nrows + 1, 0);
	std::vector<int> mstart = {0};
	std::vector<int> mclind(ncoeffs);

	rowrhs.front() -= rhs;

	size_t mclindCnt_l(0);
	for (auto const & kvp : _name_to_id) {
		rowrhs.front() += s.find(kvp.first)->second * x0.find(kvp.first)->second;
		mclind[mclindCnt_l] = kvp.second;
		matval[mclindCnt_l] = s.find(kvp.first)->second;
		++mclindCnt_l;
	}
	mclind.back() = _id_alpha_i[i];
	matval.back() = -1;

	ORTaddrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}


/*!
*  \brief Constructor of a Master Problem
*
*  Construct a Master Problem by loading mps and mapping files and adding the variable alpha
*
*  \param variable_map : map linking each variable to its id in the problem
*  \param path_to_mps : path to the problem mps file
*  \param options : set of benders options
*  \param nslaves : number of slaves
*/
WorkerMaster::WorkerMaster(Str2Int const & variable_map, std::string const & path_to_mps, BendersOptions const & options, int nslaves) :Worker() {
	_is_master = true;
	init(variable_map, path_to_mps);

	// add the variable alpha
	auto const it(_name_to_id.find("alpha"));
	if (it == _name_to_id.end()) {
		double lb(-1e10); /*!< Lower Bound */
		double ub(+1e20); /*!< Upper Bound*/
		double obj(+1);
		std::vector<int> start(2, 0);
		_id_alpha = _solver->NumVariables(); /* Set the number of columns in _id_alpha */
		ORTaddcols(*_solver, {obj}, {}, {}, {}, {lb}, {ub}, {'C'}, {"alpha"}); /* Add variable alpha and its parameters */

		_id_alpha_i.resize(nslaves, -1);
		for (int i(0); i < nslaves; ++i) {
			std::stringstream buffer;
			buffer << "alpha_" << i;
			_id_alpha_i[i] = _solver->NumVariables();
			ORTaddcols(*_solver, {0}, {}, {}, {}, {lb}, {ub}, {'C'}, {buffer.str()}); /* Add variable alpha_i and its parameters */
		}
		{
			std::vector<char> rowtype = {'E'};
			std::vector<double> rowrhs = {0};
			std::vector<double> matval(nslaves + 1, 0);
			std::vector<int> mclind(nslaves + 1);
			std::vector<int> mstart = {0};
			mclind[0] = _id_alpha;
			matval[0] = 1;

			for (int i(0); i < nslaves; ++i) {
				mclind[i + 1] = _id_alpha_i[i];
				matval[i + 1] = -1;
			}
			ORTaddrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
		}
	}
	else {
		std::cout << "ERROR a variable named alpha is in input" << std::endl;
	}
}

/*!
*  \brief Fix an upper bound and the variable alpha of a problem
*
*  \param bestUB : bound to fix
*/
void WorkerMaster::fix_alpha(double const & bestUB) {
	_solver->variables()[_id_alpha]->SetUB(bestUB);
}
