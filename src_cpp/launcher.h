#pragma once

#include "common.h"
#include <xprs.h>

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
	StandardLp(XPRSprob & _xp) {
		init();

		XPRSgetintattrib(_xp, XPRS_COLS, &std::get<Attribute::INT>(_data)[IntAttribute::NCOLS]);
		XPRSgetintattrib(_xp, XPRS_ROWS, &std::get<Attribute::INT>(_data)[IntAttribute::NROWS]);
		XPRSgetintattrib(_xp, XPRS_ELEMS, &std::get<Attribute::INT>(_data)[IntAttribute::NELES]);

		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS] + 1, 0);
		std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].assign(std::get<Attribute::INT>(_data)[IntAttribute::NELES], 0);

		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS], 'E');

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NELES], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].assign(std::get<Attribute::INT>(_data)[IntAttribute::NROWS], 0);

		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);
		std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].assign(std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0);

		int ncoeffs(0);

		XPRSgetrows(_xp, std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].data(), std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].data(), std::get<Attribute::INT>(_data)[IntAttribute::NELES], &ncoeffs, 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);

		XPRSgetrowtype(_xp, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		XPRSgetrhs(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		XPRSgetrhsrange(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NROWS] - 1);
		XPRSgetcoltype(_xp, std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);
		XPRSgetlb(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);
		XPRSgetub(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);
		XPRSgetobj(_xp, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].data(), 0, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS] - 1);

	}

	int append_in(XPRSprob & xp) const {
		IntVector newmindex(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]);
		int ncols(0);
		int status = XPRSgetintattrib(xp, XPRS_COLS, &ncols);
		// symply increment the columns indexes
		for (auto & i : newmindex) {
			i += ncols;
		}
		status = XPRSaddcols(xp, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], 0, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].data(), NULL, NULL, NULL, std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].data());
		status = XPRSchgcoltype(xp, std::get<Attribute::INT>(_data)[IntAttribute::NCOLS], newmindex.data(), std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE].data());
		status = XPRSaddrows(xp, std::get<Attribute::INT>(_data)[IntAttribute::NROWS], std::get<Attribute::INT>(_data)[IntAttribute::NELES], std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].data(), std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].data(), newmindex.data(), std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].data());
		return ncols;
	}
};