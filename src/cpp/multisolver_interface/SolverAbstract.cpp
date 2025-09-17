#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

namespace
{
bool areProblemDimensionsEquals(const SolverAbstract* one, const SolverAbstract* other)
{
    return (one->get_ncols() == other->get_ncols()) && (one->get_nrows() == other->get_nrows())
           && (one->get_nelems() == other->get_nelems());
}

bool areObjectiveFunctionEquals(const SolverAbstract* one, const SolverAbstract* other)
{
    std::vector<double> obj_one(one->get_ncols());
    std::vector<double> obj_other(other->get_ncols());
    one->get_obj(obj_one.data(), 0, one->get_ncols() - 1);
    other->get_obj(obj_other.data(), 0, other->get_ncols() - 1);

    return std::ranges::equal(obj_one, obj_other);
}

template<typename T>

bool IsEqual(const T& lhs, const T& rhs, const T& epsilon = std::numeric_limits<T>::epsilon())
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return std::abs(lhs - rhs) <= epsilon * std::max(std::abs(lhs), std::abs(rhs));
    }
    else
    {
        return (lhs == rhs);
    }
}

bool areConstraintsEquals(const SolverAbstract* one, const SolverAbstract* other)
{
    std::vector<int> mstart(one->get_nrows() + 1);
    std::vector<int> cindex(one->get_nelems());
    std::vector<double> matval_one(one->get_nelems());
    int n_one = 0;
    one->get_rows(mstart.data(),
                  cindex.data(),
                  matval_one.data(),
                  one->get_nelems(),
                  &n_one,
                  0,
                  one->get_nrows() - 1);

    std::vector<int> mstart_other(other->get_nrows() + 1);
    std::vector<int> cindex_other(other->get_nelems());
    std::vector<double> matval_other(other->get_nelems());
    int n_other = 0;
    other->get_rows(mstart_other.data(),
                    cindex_other.data(),
                    matval_other.data(),
                    other->get_nelems(),
                    &n_other,
                    0,
                    other->get_nrows() - 1);

    if (n_one != n_other)
    {
        return false;
    }
    return std::ranges::equal(mstart, mstart_other) && std::ranges::equal(cindex, cindex_other)
           && std::ranges::equal(matval_one,
                                 matval_other,
                                 [](auto l, auto r) { return IsEqual(l, r); });
}

bool areRHSEquals(const SolverAbstract* one, const SolverAbstract* other)
{
    std::vector<double> rhs_one(one->get_nrows());
    std::vector<double> rhs_other(other->get_nrows());
    one->get_rhs(rhs_one.data(), 0, one->get_nrows() - 1);
    other->get_rhs(rhs_other.data(), 0, other->get_nrows() - 1);

    return std::ranges::equal(rhs_one, rhs_other, [](auto l, auto r) { return IsEqual(l, r); });
}

bool areRowTypesEquals(const SolverAbstract* one, const SolverAbstract* other)
{
    std::vector<char> order_one(one->get_nrows());
    std::vector<char> order_other(other->get_nrows());
    one->get_row_type(order_one.data(), 0, one->get_nrows() - 1);
    other->get_row_type(order_other.data(), 0, other->get_nrows() - 1);

    return std::ranges::equal(order_one, order_other);
}

bool areBoundsEquals(const SolverAbstract* one, const SolverAbstract* other)
{
    std::vector<double> lb_one(one->get_ncols());
    std::vector<double> lb_other(other->get_ncols());
    one->get_lb(lb_one.data(), 0, one->get_ncols() - 1);
    other->get_lb(lb_other.data(), 0, other->get_ncols() - 1);

    if (!std::ranges::equal(lb_one, lb_other, [](auto l, auto r) { return IsEqual(l, r); }))
    {
        return false;
    }

    std::vector<double> ub_one(one->get_ncols());
    std::vector<double> ub_other(other->get_ncols());
    one->get_ub(ub_one.data(), 0, one->get_ncols() - 1);
    other->get_ub(ub_other.data(), 0, other->get_ncols() - 1);

    if (!std::ranges::equal(ub_one, ub_other, [](auto l, auto r) { return IsEqual(l, r); }))
    {
        return false;
    }
    return true;
}
} // namespace

bool SolverAbstract::operator==(const SolverAbstract& other) const
{
    if (this == &other)
    {
        return true;
    }
    bool is_equal = true;
    is_equal = is_equal && areProblemDimensionsEquals(this, &other);
    is_equal = is_equal && areObjectiveFunctionEquals(this, &other);
    is_equal = is_equal && areConstraintsEquals(this, &other);
    is_equal = is_equal && areRHSEquals(this, &other);
    is_equal = is_equal && areRowTypesEquals(this, &other);
    is_equal = is_equal && areBoundsEquals(this, &other);
    return is_equal;
}

std::list<std::ostream*>& SolverAbstract::get_stream() const
{
    return _streams;
}
