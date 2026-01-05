#pragma once

#include "SubproblemWorker.h"
#include "Worker.h"
/*!
 * \class SubproblemWorker
 * \brief Class daughter of Worker Class, build and manage a master problem
 */
class WorkerMaster;
typedef std::shared_ptr<WorkerMaster> WorkerMasterPtr;

class WorkerMaster: public Worker
{
public:
    WorkerMaster(const VariableMap& variable_map,
                 const std::string& solver_name,
                 int log_level,
                 int subproblems_count,
                 SolverLogManager& solver_log_manager,
                 bool mps_has_alpha,
                 Logger logger,
                 ProblemsFormat format,
                 IBendersProblemProvider* benders_problem_provider,
                 double master_solution_tolerance,
                 double cut_coefficient_tolerance);
    ~WorkerMaster() override = default;

    void get(Point& x0,
             double& overall_subpb_cost_under_approx,
             DblVector& single_subpb_costs_under_approx);
    void get_dual_values(std::vector<double>& dual) const;
    [[nodiscard]] int get_number_constraint() const;

    void add_cut(const Point& s, const Point& x0, const double& rhs) const;
    void add_cut_by_iter(int i, const Point& s, const double& sx0, const double& rhs) const;
    void add_dynamic_cut(const Point& s, const double& sx0, const double& rhs) const;
    void addSubproblemCut(int i,
                          const Point& subgradient,
                          const Point& x_cut,
                          const double& rhs) const;

    void addGroupSubproblemCut(std::vector<int> subproblem_ids,
                               const Point& subgradient,
                               const Point& x_cut,
                               const double& rhs) const;

    void fix_alpha(const double& bestUB) const;

    virtual void DeactivateIntegrityConstraints() const;
    virtual void ActivateIntegrityConstraints() const;

    [[nodiscard]] virtual std::vector<int> get_id_int_vars() const
    {
        return _id_int_vars;
    }

private:
    std::vector<int> _id_int_vars;
    std::vector<int> _id_single_subpb_costs_under_approx;
    int _id_alpha = 0;
    int subproblems_count;
    bool _mps_has_alpha = false;
    double _master_solution_tolerance;
    void define_matval_mclind(const Point& s,
                              std::vector<double>& matval,
                              std::vector<int>& mclind) const;

    void DefineRhsWithMasterVariable(const Point& s,
                                     const Point& x0,
                                     const double& rhs,
                                     std::vector<double>& rowrhs) const;

    void define_rhs_from_sx0(const double& sx0,
                             const double& rhs,
                             std::vector<double>& rowrhs) const;

    void define_matval_mclind_for_index(std::vector<int> subproblem_ids,
                                        const Point& s,
                                        std::vector<double>& matval,
                                        std::vector<int>& mclind) const;

    void _set_upper_bounds() const;
    void _set_alpha_var();
    void _set_nb_units_var_ids();
    void restoreFeasibility(std::vector<double>& solution);

public:
    // Used only for testing purposes
    void set_id_alpha(double id_alpha)
    {
        _id_alpha = id_alpha;
    }

    // Used only for testing purposes
    void set_id_single_subpb_costs_under_approx(std::vector<int> id_single_subpb_costs_under_approx)
    {
        _id_single_subpb_costs_under_approx = id_single_subpb_costs_under_approx;
    }
};
