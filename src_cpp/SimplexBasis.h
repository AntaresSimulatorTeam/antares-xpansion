#pragma once

#include "common.h"
#include "Worker.h"

typedef std::pair<IntVector, IntVector> SimplexBasis;
typedef std::map<std::string, SimplexBasis> SimplexBasisPackage;
typedef std::vector<SimplexBasisPackage> AllBasisPackage;

typedef std::shared_ptr<SimplexBasis> SimplexBasisPtr;

class SimplexBasisHandler {
public:

	IntVector & get_col();
	IntVector & get_row();

	IntVector const & get_col()const;
	IntVector const & get_row()const;

public:

	SimplexBasisHandler();
	SimplexBasisHandler(SimplexBasisPtr const &data);
	SimplexBasisHandler(SimplexBasisPtr & data);
	virtual ~SimplexBasisHandler();

	bool operator<(SimplexBasisHandler const &  other)const;

	SimplexBasisPtr _data;
	void print(std::ostream & stream)const;
};

std::ostream & operator<<(std::ostream & stream, SimplexBasisHandler const & rhs);

typedef std::set<SimplexBasisHandler> SimplexBasisStorage;


