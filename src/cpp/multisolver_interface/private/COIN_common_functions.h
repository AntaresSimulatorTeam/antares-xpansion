#ifndef COIN_COMMON_FUNCTIONS_H
#define COIN_COMMON_FUNCTIONS_H
#include <vector>

#include "CoinPackedMatrix.hpp"
namespace coin_common {

void fill_rows_from_COIN_matrix(const CoinPackedMatrix &matrix, int *mstart,
                                int *mclind, double *dmatval, int *nels,
                                int first, int last);

void fill_row_type_from_row_bounds(const double *rowLower,
                                   const double *rowUpper, char *qrtype,
                                   int first, int last);

void fill_rhs_from_bounds(const double *rowLower, const double *rowUpper,
                          double *rhs, int first, int last);
void fill_row_bounds_from_new_rows_data(std::vector<double> &rowLower,
                                        std::vector<double> &rowUpper,
                                        int newrows, const char *qrtype,
                                        const double *rhs);
}  // namespace coin_common

#endif  // COIN_COMMON_FUNCTIONS_H
