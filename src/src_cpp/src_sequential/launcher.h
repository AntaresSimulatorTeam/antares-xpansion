#pragma once

#include "common.h"

#include "ortools_utils.h"

class BendersOptions;
int build_input(BendersOptions const & options, CouplingMap & coupling_map);

BendersOptions build_benders_options(int argc, char** argv);

void sequential_launch(BendersOptions const &options);

void usage(int argc);

enum Attribute {
	INT,
	INT_VECTOR,
	CHAR_VECTOR,
	DBL_VECTOR,
	MAX_ATTRIBUTE
};

enum IntAttribute {
	NROWS,
	NCOLS,
	NELES,
	MAX_INT_ATTRIBUTE
};


enum IntVectorAttribute {
	MSTART,
	MINDEX,
	MAX_INT_VECTOR_ATTRIBUTE,
};
enum CharVectorAttribute {
	ROWTYPE,
	COLTYPE,
	MAX_CHAR_VECTOR_ATTRIBUTE
};

enum DblVectorAttribute {
	MVALUE,
	RHS,
	RANGE,
	OBJ,
	LB,
	UB,
	MAX_DBL_VECTOR_ATTRIBUTE
};
typedef std::tuple<IntVector, std::vector<IntVector>, std::vector<CharVector>, std::vector<DblVector> > raw_standard_lp_data;

class StandardLp {
public:
	// to be used in boost serialization for mpi transfer
	raw_standard_lp_data _data;
public:
	void init() {
		std::get<Attribute::INT>(_data).assign(IntAttribute::MAX_INT_ATTRIBUTE, 0);

		std::get<Attribute::INT_VECTOR>(_data).assign(IntVectorAttribute::MAX_INT_VECTOR_ATTRIBUTE, IntVector());
		std::get<Attribute::CHAR_VECTOR>(_data).assign(CharVectorAttribute::MAX_CHAR_VECTOR_ATTRIBUTE, CharVector());
		std::get<Attribute::DBL_VECTOR>(_data).assign(DblVectorAttribute::MAX_DBL_VECTOR_ATTRIBUTE, DblVector());
	}


	StandardLp(operations_research::MPSolver & solver_p)
	{
		init();

		std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] = solver_p.NumVariables();
		std::cout << "vars: " << solver_p.NumVariables() << "\n";
		std::get<Attribute::INT>(_data)[IntAttribute::NROWS] = solver_p.NumConstraints();
		std::cout << "constraints: " << solver_p.NumConstraints() << "\n";
		std::get<Attribute::INT>(_data)[IntAttribute::NELES] = 0;
		for(operations_research::MPConstraint* constraint_l : solver_p.constraints())
		{
			std::get<Attribute::INT>(_data)[IntAttribute::NELES] += constraint_l->terms().size();
		}
		std::cout << "nelems: " << std::get<Attribute::INT>(_data)[IntAttribute::NELES] << "\n";


		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].clear();
		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].clear();
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].clear();
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].clear();

		//@TODO verify correspondance between ortools variable ids and supposed ones
		ORTgetrows(solver_p, std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART], std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX], std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE], 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);

		ORTgetrowtype(solver_p, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE], 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		ORTgetrhs(solver_p, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS], 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		ORTgetrhsrange(solver_p, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE], 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);

		for(auto el : std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE]) std::cout << el << " ";
		std::cout << std::endl << "RHS: ";
		for(auto el : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS]) std::cout << el << " ";
		std::cout << std::endl << "range: " ;
		for(auto el : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE]) std::cout << el << " ";
		std::cout << std::endl << std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].size() << std::endl;

		//@TODO check if we use semi-continuous or partial-integer variables
		ORTgetcolinfo(solver_p, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE], std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB], std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB], 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);

		for(auto el : std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE]) std::cout << el << " ";
		std::cout << std::endl << "LB: ";
		for(auto el : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB]) std::cout << el << " ";
		std::cout << std::endl << "UB: " ;
		for(auto el : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB]) std::cout << el << " ";
		std::cout << std::endl << std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].size() << std::endl;

		std::vector<double> v;//std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ]
		ORTgetobj(solver_p, v, 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]-1);

		assert(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].size() == std::get<Attribute::INT>(_data)[IntAttribute::NROWS] + 1);
		assert(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].size() == std::get<Attribute::INT>(_data)[IntAttribute::NELES]);

		assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].size() == std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].size() == std::get<Attribute::INT>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].size() == std::get<Attribute::INT>(_data)[IntAttribute::NELES]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].size() == std::get<Attribute::INT>(_data)[IntAttribute::NROWS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].size() == std::get<Attribute::INT>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].size() == std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].size() == std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].size() == std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]);

	}

	int append_in(operations_research::MPSolver & containingSolver_p) const {
		IntVector newmindex(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]);
		int nbExistingCols(containingSolver_p.NumVariables());
		// simply increment the columns indexes
		for (auto & i : newmindex) {
			i += nbExistingCols;
		}

		ORTaddcols(containingSolver_p, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ], {}, {}, {},  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB], std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB], std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE]);

		ORTaddrows(containingSolver_p, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE], std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS], std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE], std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART], newmindex, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE]);
		return nbExistingCols;
	}
};
