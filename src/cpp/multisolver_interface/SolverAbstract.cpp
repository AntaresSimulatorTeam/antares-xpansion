#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

namespace {
    bool verifyProblemDimensions(const SolverAbstract *merged, const SolverAbstract *master) {
        if (merged->get_ncols() != master->get_ncols())
            return false;
        if (merged->get_nrows() != master->get_nrows())
            return false;
        if (merged->get_nelems() != master->get_nelems())
            return false;
        return true;
    }

    bool verifyObjectiveFunction(const SolverAbstract *merged, const SolverAbstract *master) {
        std::vector<double> obj_merged(merged->get_ncols());
        std::vector<double> obj_master(master->get_ncols());
        merged->get_obj(obj_merged.data(), 0, merged->get_ncols() - 1);
        master->get_obj(obj_master.data(), 0, master->get_ncols() - 1);

        for (int i = 0; i < merged->get_ncols(); ++i) {
            if (obj_merged[i] != obj_master[i])
                return false;
        }
        return true;
    }

    bool verifySolution(const SolverAbstract *merged, const SolverAbstract *master) {
        std::vector<double> sol_merged(merged->get_ncols());
        std::vector<double> sol_master(master->get_ncols());
        merged->get_lp_sol(sol_merged.data(), nullptr, nullptr);
        master->get_lp_sol(sol_master.data(), nullptr, nullptr);

        for (int i = 0; i < merged->get_ncols(); ++i) {
            if (sol_merged[i] != sol_master[i]) {
                return false;
            }
        }
        return true;
    }

    template<typename T>

    bool IsEqual(const T &lhs, const T &rhs, const T &epsilon = std::numeric_limits<T>::epsilon()) {
        if constexpr (std::is_floating_point_v<T>) {
            return std::abs(lhs - rhs) <= epsilon * std::max(std::abs(lhs), std::abs(rhs));
        } else return (lhs == rhs);
    }

    bool verifyConstraints(const SolverAbstract *merged, const SolverAbstract *master) {
        std::vector<int> mstart(merged->get_nrows() + 1);
        std::vector<int> cindex(merged->get_nelems());
        std::vector<double> matval_merged(merged->get_nelems());
        int n_merged = 0;
        merged->get_rows(mstart.data(), cindex.data(), matval_merged.data(), merged->get_nelems(), &n_merged, 0,
                         merged->get_nrows() - 1);

        std::vector<int> mstart_master(master->get_nrows() + 1);
        std::vector<int> cindex_master(master->get_nelems());
        std::vector<double> matval_master(master->get_nelems());
        int n_master = 0;
        master->get_rows(mstart_master.data(), cindex_master.data(), matval_master.data(), master->get_nelems(),
                         &n_master,
                         0,
                         master->get_nrows() - 1);

        if (n_merged != n_master) {
            return false;
        }
        for (int i = 0; i < n_merged; ++i) {
            if (mstart[i] != mstart_master[i]) {
                return false;
            }
            if (cindex[i] != cindex_master[i]) {
                return false;
            }
            if (!IsEqual(matval_merged[i], matval_master[i])) {
                return false;
            }
        }
        return true;
    }

    bool verifyRHS(const SolverAbstract *merged, const SolverAbstract *master) {
        std::vector<double> rhs_merged(merged->get_nrows());
        std::vector<double> rhs_master(master->get_nrows());
        merged->get_rhs(rhs_merged.data(), 0, merged->get_nrows() - 1);
        master->get_rhs(rhs_master.data(), 0, master->get_nrows() - 1);

        for (int i = 0; i < merged->get_nrows(); ++i) {
            if (!IsEqual(rhs_merged[i], rhs_master[i])) {
                return false;
            }
        }
        return true;
    }

    bool verifyRowTypes(const SolverAbstract *merged, const SolverAbstract *master) {
        std::vector<char> order_merged(merged->get_nrows());
        std::vector<char> order_master(master->get_nrows());
        merged->get_row_type(order_merged.data(), 0, merged->get_nrows() - 1);
        master->get_row_type(order_master.data(), 0, master->get_nrows() - 1);

        for (int i = 0; i < merged->get_nrows(); ++i) {
            if (order_merged[i] != order_master[i]) {
                return false;
            }
        }
        return true;
    }

    bool verifyBounds(const SolverAbstract *merged, const SolverAbstract *master) {
        std::vector<double> lb_merged(merged->get_ncols());
        std::vector<double> lb_master(master->get_ncols());
        merged->get_lb(lb_merged.data(), 0, merged->get_ncols() - 1);
        master->get_lb(lb_master.data(), 0, master->get_ncols() - 1);

        for (int i = 0; i < merged->get_ncols(); ++i) {
            if (!IsEqual(lb_merged[i], lb_master[i])) {
                return false;
            }
        }


        std::vector<double> ub_merged(merged->get_ncols());
        std::vector<double> ub_master(master->get_ncols());
        merged->get_ub(ub_merged.data(), 0, merged->get_ncols() - 1);
        master->get_ub(ub_master.data(), 0, master->get_ncols() - 1);

        for (int i = 0; i < merged->get_ncols(); ++i) {
            if (!IsEqual(ub_merged[i], ub_master[i])) {
                return false;
            }
        }
        return true;
    }
}

bool SolverAbstract::operator==(const SolverAbstract &other) const {
    if (this == &other) {
        return true;
    }
    bool is_equal = true;
    is_equal = is_equal && verifyProblemDimensions(this, &other);
    is_equal = is_equal && verifyObjectiveFunction(this, &other);
    is_equal = is_equal && verifyConstraints(this, &other);
    is_equal = is_equal && verifyRHS(this, &other);
    is_equal = is_equal && verifyRowTypes(this, &other);
    is_equal = is_equal && verifyBounds(this, &other);
    return is_equal;
}
