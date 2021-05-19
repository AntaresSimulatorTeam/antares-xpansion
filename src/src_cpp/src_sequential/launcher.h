#pragma once

#include "common.h"

#include "ortools_utils.h"

class BendersOptions;
int build_input(BendersOptions const & options, CouplingMap & coupling_map);

BendersOptions build_benders_options(int argc, char** argv);

void sequential_launch(BendersOptions const &options);

void usage(int argc);

enum Attribute {
	INT_VALUE,
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
private:
	std::vector<std::string> _colNames;

public:
	// to be used in boost serialization for mpi transfer
	raw_standard_lp_data _data;
	static size_t appendCNT;
public:
	void init() {
		std::get<Attribute::INT_VALUE>(_data).assign(IntAttribute::MAX_INT_ATTRIBUTE, 0);

		std::get<Attribute::INT_VECTOR>(_data).assign(IntVectorAttribute::MAX_INT_VECTOR_ATTRIBUTE, IntVector());
		std::get<Attribute::CHAR_VECTOR>(_data).assign(CharVectorAttribute::MAX_CHAR_VECTOR_ATTRIBUTE, CharVector());
		std::get<Attribute::DBL_VECTOR>(_data).assign(DblVectorAttribute::MAX_DBL_VECTOR_ATTRIBUTE, DblVector());
	}


	StandardLp(operations_research::MPSolver & solver_p)
	{
		init();

		std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] = solver_p.NumVariables();
		std::cout << "vars: " << solver_p.NumVariables() << "\n";

		std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] = solver_p.NumConstraints();
		std::cout << "constraints: " << solver_p.NumConstraints() << "\n";

		std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES] = 0;
		for(operations_research::MPConstraint* constraint_l : solver_p.constraints())
		{
			std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES] += constraint_l->terms().size();
		}
		std::cout << "nelems: " << std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES] << "\n";

		for(auto var_l : solver_p.variables())
		{
			_colNames.push_back(var_l->name());
		}

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
		ORTgetrows(solver_p,
					std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART],
					std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX],
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE],
					0,
					std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

		ORTgetrowtype(solver_p,
					  std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE],
					  0,
					  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

		ORTgetrhs(solver_p,
				  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS],
				  0,
				  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

		ORTgetrhsrange(solver_p,
						std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE],
						0,
						std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

		//@TODO check if we use semi-continuous or partial-integer variables
		ORTgetcolinfo(solver_p,
					  std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE],
					  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB],
					  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB],
					  0,
					  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] - 1);

		ORTgetobj(solver_p,
				  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ],
				  0,
				  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]-1);

		assert(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
	}

	int append_in(operations_research::MPSolver & containingSolver_p, std::string const & prefix_p = "") const {

		// simply increment the columns indices
		IntVector newmindex(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]);
		int nbExistingCols(containingSolver_p.NumVariables());
		for (auto & i : newmindex) {
			i += nbExistingCols;
		}

		//rename variables
		std::string prefix_l = (prefix_p != "") ? prefix_p : ("prob"+std::to_string(appendCNT));
		std::vector<std::string> newNames;
		newNames.resize(_colNames.size());
		std::transform(_colNames.begin(), _colNames.end(), newNames.begin(),
					[&prefix_l](std::string varName_p)->std::string{ return prefix_l + varName_p; });


		ORTaddcols(containingSolver_p,
				   std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ],
				   {}, {}, {},
				   std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB],
				   std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB],
				   std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE],
				   newNames);


		ORTaddrows(containingSolver_p,
					std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE],
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS],
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE],
					std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART],
					newmindex,
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE]);

		++appendCNT;

		return nbExistingCols;
	}
};
