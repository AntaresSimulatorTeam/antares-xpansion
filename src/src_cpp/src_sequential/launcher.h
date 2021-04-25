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


	StandardLp(SolverAbstract::Ptr solver_p)
	{
		init();

		int ncols = solver_p->get_ncols();
		int nrows = solver_p->get_nrows();
		int nelems = solver_p->get_nelems();

		std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] = ncols;
		std::cout << "vars: " << std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] << "\n";

		std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] = nrows;
		std::cout << "constraints: " << std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] << "\n";

		std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES] = nelems;
		std::cout << "nelems: " << std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES] << "\n";

		_colNames.resize(ncols);
		solver_p->get_col_names(0, ncols - 1, _colNames);

		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].clear();
		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].resize(nrows + 1);

		std::cout << "mindex size = " << std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].size() << std::endl;
		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].clear();
		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].resize(nelems);
		std::cout << "mindex size = " << std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].size() << std::endl;

		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].clear();
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].resize(ncols);

		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].clear();
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].resize(nrows);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].resize(nelems);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].resize(nrows);

		// Range constraint don't exist in a sparse matrix formualtion
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].resize(nrows);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].resize(ncols);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].resize(ncols);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].clear();
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].resize(ncols);


		std::cout << "coucou StandardLp befreo getows" << std::endl;
		ORTgetrows(solver_p,
					std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART],
					std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX],
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE],
					0,
					std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);
		
		std::cout << "mindex size = " << std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].size() << std::endl;
		for (auto const& val : std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]) {
			std::cout << val << "   ";
		}
		std::cout << std::endl;
		for (auto const& val : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE]) {
			std::cout << val << "   ";
		}
		std::cout << std::endl;
		std::cout << "coucou StandardLp befreo getRowtype" << std::endl;
		ORTgetrowtype(solver_p,
					  std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE],
					  0,
					  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);
		std::cout << "coucou StandardLp befreo getrhs" << std::endl;
		ORTgetrhs(solver_p,
				  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS],
				  0,
				  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);
		std::cout << "coucou StandardLp befreo comment" << std::endl;
		// Range constraint don't exist in a sparse matrix formualtion
		/*ORTgetrhsrange(solver_p,
						std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE],
						0,
						std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);*/
		std::cout << "coucou StandardLp befreo colinfo" << std::endl;
		ORTgetcolinfo(solver_p,
					  std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE],
					  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB],
					  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB],
					  0,
					  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] - 1);
		std::cout << "coucou StandardLp befreo gerobj" << std::endl;
		ORTgetobj(solver_p,
				  std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ],
				  0,
				  std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]-1);
		std::cout << "coucou StandardLp befreo assert" << std::endl;

		assert(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);
		//assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
		assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].size() == std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
	}

	int append_in(SolverAbstract::Ptr containingSolver_p, std::string const & prefix_p = "") const {

		// simply increment the columns indices
		IntVector newmindex(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]);
		int nbExistingCols(containingSolver_p->get_ncols());
		for (auto & i : newmindex) {
			i += nbExistingCols;
		}

		//rename variables
		std::string prefix_l = (prefix_p != "") ? prefix_p : ("prob"+std::to_string(appendCNT));
		std::vector<std::string> newNames;
		newNames.resize(_colNames.size());
		std::transform(_colNames.begin(), _colNames.end(), newNames.begin(),
					[&prefix_l](std::string varName_p)->std::string{ return prefix_l + varName_p; });
		std::cout << "coucou append in before addCol" << std::endl;


		std::cout << "newcols " << std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] << std::endl;
		std::vector<int> mstart(std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS], 0);

		std::cout << "Obj " << std::endl;
		for (auto const& val : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ]) { std::cout << val << "   "; }
		std::cout << "\n mstart" << std::endl;
		for (auto const& val : mstart) { std::cout << val << "   "; }
		std::cout << "\n lb" << std::endl;
		for (auto const& val : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB]) { std::cout << val << "   "; }
		std::cout << "\n ub" << std::endl;
		for (auto const& val : std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB]) { std::cout << val << "   "; }
		std::cout << "\n coltype" << std::endl;
		for (auto const& val : std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE]) { std::cout << val << "   "; }
		std::cout << "\n names" << std::endl;
		for (auto const& val : newNames) { std::cout << val << "   "; }
		std::cout << "\n" << std::endl;
		ORTaddcols(containingSolver_p,
				   std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ],
				   mstart, IntVector(0, 0), DblVector(0, 0.0),
				   std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB],
				   std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB],
				   std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE],
				   newNames);

		std::cout << "coucou append in before addRox" << std::endl;
		ORTaddrows(containingSolver_p,
					std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE],
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS],
					{},
					std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART],
					newmindex,
					std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE]);

		++appendCNT;

		return nbExistingCols;
	}
};
