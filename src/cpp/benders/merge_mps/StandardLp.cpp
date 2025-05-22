#include "antares-xpansion/benders/merge_mps/StandardLp.h"

enum Attribute
{
    INT_VALUE,
    INT_VECTOR,
    CHAR_VECTOR,
    DBL_VECTOR,
    MAX_ATTRIBUTE
};

enum IntAttribute
{
    NROWS,
    NCOLS,
    NELES,
    MAX_INT_ATTRIBUTE
};

enum IntVectorAttribute
{
    MSTART,
    MINDEX,
    MAX_INT_VECTOR_ATTRIBUTE,
};

enum CharVectorAttribute
{
    ROWTYPE,
    COLTYPE,
    MAX_CHAR_VECTOR_ATTRIBUTE
};

enum DblVectorAttribute
{
    MVALUE,
    RHS,
    RANGE,
    OBJ,
    LB,
    UB,
    MAX_DBL_VECTOR_ATTRIBUTE
};

StandardLp::StandardLp(SolverAbstract& solver_p)
{
    init();

    int ncols = solver_p.get_ncols();
    int nrows = solver_p.get_nrows();
    int nelems = solver_p.get_nelems();

    std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS] = ncols;
    std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS] = nrows;
    std::get<Attribute::INT_VALUE>(data)[IntAttribute::NELES] = nelems;

    col_names_ = solver_p.get_col_names();

    std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MSTART].clear();
    std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MSTART].resize(nrows + 1);

    std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MINDEX].clear();
    std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MINDEX].resize(nelems);

    std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::COLTYPE].clear();
    std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::COLTYPE].resize(ncols);

    std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::ROWTYPE].clear();
    std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::ROWTYPE].resize(nrows);

    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::MVALUE].clear();
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::MVALUE].resize(nelems);

    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RHS].clear();
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RHS].resize(nrows);

    // Range constraint don't exist in a sparse matrix formulation
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RANGE].clear();
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RANGE].resize(nrows);

    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::OBJ].clear();
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::OBJ].resize(ncols);

    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::LB].clear();
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::LB].resize(ncols);

    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::UB].clear();
    std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::UB].resize(ncols);

    solver_getrows(solver_p,
                   std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MSTART],
                   std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MINDEX],
                   std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::MVALUE],
                   0,
                   std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS] - 1);

    solver_getrowtype(solver_p,
                      std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::ROWTYPE],
                      0,
                      std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS] - 1);

    solver_getrhs(solver_p,
                  std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RHS],
                  0,
                  std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS] - 1);

    solver_getcolinfo(solver_p,
                      std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::COLTYPE],
                      std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::LB],
                      std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::UB],
                      0,
                      std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS] - 1);

    solver_get_obj_func_coeffs(solver_p,
                               std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::OBJ],
                               0,
                               std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS] - 1);

    assert(std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MSTART].size()
           == 1 + std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS]);

    assert(std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::COLTYPE].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS]);
    assert(std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::ROWTYPE].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS]);

    assert(std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::MVALUE].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NELES]);
    assert(std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RHS].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NROWS]);

    assert(std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::OBJ].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS]);
    assert(std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::LB].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS]);
    assert(std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::UB].size()
           == std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS]);
}

int StandardLp::append_in(SolverAbstract& containingSolver_p, const std::string& prefix_p) const
{
    // simply increment the columns indices
    std::vector<int> newmindex(std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MINDEX]);
    int nbExistingCols(containingSolver_p.get_ncols());
    for (auto& i: newmindex)
    {
        i += nbExistingCols;
    }

    // rename variables
    std::string prefix_l = (prefix_p != "") ? prefix_p : ("prob" + std::to_string(appendCNT));
    std::vector<std::string> newNames;
    newNames.resize(col_names_.size());
    std::transform(col_names_.begin(),
                   col_names_.end(),
                   newNames.begin(),
                   [&prefix_l](std::string varName_p) -> std::string
                   { return prefix_l + varName_p; });

    std::vector<int> mstart(std::get<Attribute::INT_VALUE>(data)[IntAttribute::NCOLS], 0);
    solver_addcols(containingSolver_p,
                   std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::OBJ],
                   mstart,
                   std::vector<int>(0, 0),
                   std::vector<double>(0, 0.0),
                   std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::LB],
                   std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::UB],
                   std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::COLTYPE],
                   newNames);

    solver_addrows(containingSolver_p,
                   std::get<Attribute::CHAR_VECTOR>(data)[CharVectorAttribute::ROWTYPE],
                   std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::RHS],
                   {},
                   std::get<Attribute::INT_VECTOR>(data)[IntVectorAttribute::MSTART],
                   newmindex,
                   std::get<Attribute::DBL_VECTOR>(data)[DblVectorAttribute::MVALUE]);

    ++appendCNT;

    return nbExistingCols;
}

void StandardLp::init()
{
    initialise_int_values_with_zeros();
    initialise_int_vectors();
    initialise_char_vectors();
    initialise_dbl_vectors();
}

void StandardLp::initialise_int_values_with_zeros()
{
    std::get<Attribute::INT_VALUE>(data).assign(IntAttribute::MAX_INT_ATTRIBUTE, 0);
}

void StandardLp::initialise_int_vectors()
{
    std::get<Attribute::INT_VECTOR>(data).assign(IntVectorAttribute::MAX_INT_VECTOR_ATTRIBUTE,
                                                 std::vector<int>());
}

void StandardLp::initialise_char_vectors()
{
    std::get<Attribute::CHAR_VECTOR>(data).assign(CharVectorAttribute::MAX_CHAR_VECTOR_ATTRIBUTE,
                                                  std::vector<char>());
}

void StandardLp::initialise_dbl_vectors()
{
    std::get<Attribute::DBL_VECTOR>(data).assign(DblVectorAttribute::MAX_DBL_VECTOR_ATTRIBUTE,
                                                 std::vector<double>());
}
