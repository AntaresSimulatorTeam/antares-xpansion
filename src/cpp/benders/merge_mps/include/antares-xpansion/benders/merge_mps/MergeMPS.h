#pragma once

#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/helpers/solver_utils.h"

enum Attribute {
  INT_VALUE,
  INT_VECTOR,
  CHAR_VECTOR,
  DBL_VECTOR,
  MAX_ATTRIBUTE
};

enum IntAttribute { NROWS, NCOLS, NELES, MAX_INT_ATTRIBUTE };

enum IntVectorAttribute {
  MSTART,
  MINDEX,
  MAX_INT_VECTOR_ATTRIBUTE,
};
enum CharVectorAttribute { ROWTYPE, COLTYPE, MAX_CHAR_VECTOR_ATTRIBUTE };

enum DblVectorAttribute {
  MVALUE,
  RHS,
  RANGE,
  OBJ,
  LB,
  UB,
  MAX_DBL_VECTOR_ATTRIBUTE
};
typedef std::tuple<IntVector, std::vector<IntVector>, std::vector<CharVector>,
                   std::vector<DblVector>>
    raw_standard_lp_data;

class StandardLp {
 private:
  std::vector<std::string> _colNames;

 public:
  // to be used in boost serialization for mpi transfer
  raw_standard_lp_data _data;
  static size_t appendCNT;

 public:
  void init() {
    initialise_int_values_with_zeros();
    initialise_int_vectors();
    initialise_char_vectors();
    initialise_dbl_vectors();
  }

  explicit StandardLp(SolverAbstract &solver_p) {
    init();

    int ncols = solver_p.get_ncols();
    int nrows = solver_p.get_nrows();
    int nelems = solver_p.get_nelems();

    std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] = ncols;
    std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] = nrows;
    std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES] = nelems;

    _colNames = solver_p.get_col_names();

    std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].clear();
    std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART].resize(
        nrows + 1);

    std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].clear();
    std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX].resize(
        nelems);

    std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE]
        .clear();
    std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE]
        .resize(ncols);

    std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE]
        .clear();
    std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE]
        .resize(nrows);

    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].clear();
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE].resize(
        nelems);

    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].clear();
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS].resize(
        nrows);

    // Range constraint don't exist in a sparse matrix formulation
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].clear();
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RANGE].resize(
        nrows);

    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].clear();
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ].resize(
        ncols);

    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].clear();
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].resize(
        ncols);

    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].clear();
    std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].resize(
        ncols);

    solver_getrows(
        solver_p,
        std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART],
        std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX],
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE], 0,
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

    solver_getrowtype(
        solver_p,
        std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE],
        0, std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

    solver_getrhs(
        solver_p,
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS], 0,
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS] - 1);

    solver_getcolinfo(
        solver_p,
        std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE],
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB],
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB], 0,
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] - 1);

    solver_get_obj_func_coeffs(
        solver_p,
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ], 0,
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS] - 1);

    assert(std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART]
               .size() ==
           1 + std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

    assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE]
               .size() ==
           std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
    assert(std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE]
               .size() ==
           std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

    assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE]
               .size() ==
           std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NELES]);
    assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS]
               .size() ==
           std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NROWS]);

    assert(std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ]
               .size() ==
           std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
    assert(
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB].size() ==
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
    assert(
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB].size() ==
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS]);
  }

  int append_in(SolverAbstract::Ptr containingSolver_p,
                std::string const &prefix_p = "") const {
    // simply increment the columns indices
    IntVector newmindex(
        std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MINDEX]);
    int nbExistingCols(containingSolver_p->get_ncols());
    for (auto &i : newmindex) {
      i += nbExistingCols;
    }

    // rename variables
    std::string prefix_l =
        (prefix_p != "") ? prefix_p : ("prob" + std::to_string(appendCNT));
    std::vector<std::string> newNames;
    newNames.resize(_colNames.size());
    std::transform(_colNames.begin(), _colNames.end(), newNames.begin(),
                   [&prefix_l](std::string varName_p) -> std::string {
                     return prefix_l + varName_p;
                   });

    std::vector<int> mstart(
        std::get<Attribute::INT_VALUE>(_data)[IntAttribute::NCOLS], 0);
    solver_addcols(
        *containingSolver_p,
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::OBJ], mstart,
        IntVector(0, 0), DblVector(0, 0.0),
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::LB],
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::UB],
        std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::COLTYPE],
        newNames);

    solver_addrows(
        *containingSolver_p,
        std::get<Attribute::CHAR_VECTOR>(_data)[CharVectorAttribute::ROWTYPE],
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::RHS], {},
        std::get<Attribute::INT_VECTOR>(_data)[IntVectorAttribute::MSTART],
        newmindex,
        std::get<Attribute::DBL_VECTOR>(_data)[DblVectorAttribute::MVALUE]);

    ++appendCNT;

    return nbExistingCols;
  }

 private:
  void initialise_int_values_with_zeros() {
    std::get<Attribute::INT_VALUE>(_data).assign(
        IntAttribute::MAX_INT_ATTRIBUTE, 0);
  }
  void initialise_int_vectors() {
    std::get<Attribute::INT_VECTOR>(_data).assign(
        IntVectorAttribute::MAX_INT_VECTOR_ATTRIBUTE, IntVector());
  }
  void initialise_char_vectors() {
    std::get<Attribute::CHAR_VECTOR>(_data).assign(
        CharVectorAttribute::MAX_CHAR_VECTOR_ATTRIBUTE, CharVector());
  }
  void initialise_dbl_vectors() {
    std::get<Attribute::DBL_VECTOR>(_data).assign(
        DblVectorAttribute::MAX_DBL_VECTOR_ATTRIBUTE, DblVector());
  }
};

class MergeMPS {
 public:
  MergeMPS(const MergeMPSOptions &options, Logger &logger, Writer writer);
  void launch();
  double slave_weight(int nslaves, std::string const &name) const;

  MergeMPSOptions _options;
  Logger _logger;
  Writer _writer;
};
