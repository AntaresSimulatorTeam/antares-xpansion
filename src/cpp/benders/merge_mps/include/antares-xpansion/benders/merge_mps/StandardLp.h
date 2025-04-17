#pragma once

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/helpers/solver_utils.h"

using raw_standard_lp_data = std::
  tuple<IntVector, std::vector<IntVector>, std::vector<CharVector>, std::vector<DblVector>>;

class StandardLp
{
public:
    // to be used in boost serialization for mpi transfer
    raw_standard_lp_data data;
    static size_t appendCNT;

    explicit StandardLp(SolverAbstract& solver_p);

    int append_in(SolverAbstract& containingSolver_p, const std::string& prefix_p = "") const;

private:
    void init();
    void initialise_int_values_with_zeros();
    void initialise_int_vectors();
    void initialise_char_vectors();
    void initialise_dbl_vectors();

    std::vector<std::string> col_names_;
};
