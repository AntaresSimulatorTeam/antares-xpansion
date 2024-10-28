
#include "COIN_common_functions.h"

#include <sstream>

#include "CoinFinite.hpp"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
namespace coin_common {

void fill_rows_from_COIN_matrix(const CoinPackedMatrix &matrix, int *mstart,
                                int *mclind, double *dmatval, int *nels,
                                int first, int last) {
  const int *column = matrix.getIndices();
  const CoinBigIndex *rowStart = matrix.getVectorStarts();
  const double *vals = matrix.getElements();

  int firstIndexToReturn = rowStart[first];
  int lastIndexToReturn = rowStart[last + 1] - 1;
  int nelemsToReturn = lastIndexToReturn - firstIndexToReturn + 1;
  // Need to take into account the offset of rowstart as _clp.matrix
  // returnes the entire matrix
  for (int i = first; i < last + 2; i++) {
    mstart[i - first] = rowStart[i] - rowStart[first];
  }

  for (int i = firstIndexToReturn; i < lastIndexToReturn + 1; i++) {
    mclind[i - firstIndexToReturn] = column[i];
    dmatval[i - firstIndexToReturn] = vals[i];
  }
  *nels = nelemsToReturn;
}

void fill_row_type_from_row_bounds(const double *rowLower,
                                   const double *rowUpper, char *qrtype,
                                   int first, int last) {
  for (int i = first; i < last + 1; i++) {
    if (rowLower[i] == rowUpper[i]) {
      qrtype[i - first] = 'E';
    } else if (rowLower[i] > -COIN_DBL_MAX) {
      if (rowUpper[i] < COIN_DBL_MAX) {
        auto error = LOGLOCATION + "ERROR : Row " + std::to_string(i) +
                     " has two different RHS, both right and left.";
        throw GenericSolverException(error);
      } else {
        qrtype[i - first] = 'G';
      }
    } else if (rowUpper[i] < COIN_DBL_MAX) {
      qrtype[i - first] = 'L';
    } else {
      auto error = LOGLOCATION + "ERROR : Row " + std::to_string(i) +
                   " in unconstrained. No RHS found.";
      throw GenericSolverException(error);
    }
  }
}

void fill_rhs_from_bounds(const double *rowLower, const double *rowUpper,
                          double *rhs, int first, int last) {
  for (int i = first; i < last + 1; i++) {
    if (rowLower[i] == rowUpper[i]) {
      rhs[i - first] = rowLower[i];
    } else if (rowLower[i] > -COIN_DBL_MAX) {
      if (rowUpper[i] < COIN_DBL_MAX) {
        auto error = LOGLOCATION + "ERROR : Row " + std::to_string(i) +
                     " has two different RHS, both right and left.";
        throw GenericSolverException(error);
      } else {
        rhs[i - first] = rowLower[i];
      }
    } else if (rowUpper[i] < COIN_DBL_MAX) {
      rhs[i - first] = rowUpper[i];
    } else {
      auto error = LOGLOCATION + "ERROR : Row " + std::to_string(i) +
                   " in unconstrained. No RHS found.";
      throw GenericSolverException(error);
    }
  }
}
void fill_row_bounds_from_new_rows_data(std::vector<double> &rowLower,
                                        std::vector<double> &rowUpper,
                                        int newrows, const char *qrtype,
                                        const double *rhs) {
  for (int i(0); i < newrows; i++) {
    if (qrtype[i] == 'L') {
      rowUpper[i] = rhs[i];
      rowLower[i] = -COIN_DBL_MAX;
    } else if (qrtype[i] == 'G') {
      rowUpper[i] = COIN_DBL_MAX;
      rowLower[i] = rhs[i];
    } else if (qrtype[i] == 'E') {
      rowUpper[i] = rhs[i];
      rowLower[i] = rhs[i];
    } else {
      std::stringstream buffer;
      buffer << LOGLOCATION << "ERROR : add rows, qrtype " << qrtype[i]
             << " of row " << i << " to add unknown.";
      throw GenericSolverException(buffer.str());
    }
  }
}
}  // namespace coin_common
