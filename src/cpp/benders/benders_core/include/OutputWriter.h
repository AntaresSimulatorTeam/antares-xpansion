
#pragma once

#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "common.h"
namespace Output
{

    /*!
     *  \brief struct containing some entries to be later written to the json file
     *
     *  nbWeeks_p : number of the weeks in the study
     *   bendersTrace_p : trace to be written ie iterations details
     *   bendersData_p : final benders data to get the best iteration
     *   min_abs_gap : minimum absolute gap wanted
     *   min_rel_gap : minimum relative gap wanted
     *   max_iter : maximum number of iterations
     */
    struct IterationsData
    {
        int nbWeeks_p;
        BendersTrace bendersTrace_p;
        int it;
        int best_it;
        double best_ub;
        double lb;
        double min_abs_gap;
        double min_rel_gap;
        double max_iter;
    };

    /*!
     *  \brief struct saves some entries to be later written to the json file
     *
     *   nbWeeks_p : number of the weeks in the study
     *   lb_p : solution lower bound
     *   ub_p : solution upper bound
     *   investCost_p : investment cost
     *   operationalCost_p : operational cost
     *   overallCost_p : total cost, sum of invest and operational
     *   solution_p : point giving the solution and the candidates
     *   optimality_p : indicates if optimality was reached
     */
    struct SolutionData
    {
        int nbWeeks_p;
        double lb_p;
        double ub_p;
        double investCost_p;
        double operationalCost_p;
        double overallCost_p;
        Point solution_p;
        bool optimality_p;
    };

    /*!
     * \class OutputWriter
     * \brief OutputWriter class to describe the execuion session of an antares xpansion optimization in a log file
     */
    class OutputWriter
    {

    public:
        /*!
         *  \brief destructor of class OutputWriter
         */
        virtual ~OutputWriter() = default;

        /*!
         *  \brief updates the execution begin time
         */
        virtual void updateBeginTime() = 0;

        /*!
         *  \brief updates the end of execution time
         */
        virtual void updateEndTime() = 0;

        /*!
         *  \brief saves the options of the benders algorithm to be later written to the json file
         *
         *  \param bendersOptions_p : set of options used for the optimization
         */
        virtual void write_options(BendersOptions const &bendersOptions_p) = 0;

        /*!
         *  \brief saves some entries to be later written to the json file
         *
         *  \param iterations_data : containing iterations data
         */
        virtual void write_iteration(const IterationsData &iterations_data) = 0;

        /*!
         *  \brief  saves some entries to be later written to the json file
         *
         *  \param solution_data containing solution data
         */
        virtual void update_solution(const SolutionData &solution_data) = 0;

        /*!
         *  \brief write an a priori errored log output, overwritten if optimization ends
         */
        virtual void write_failure() = 0;

        /*!
         *  \brief write the log data into a file
         */
        virtual void dump() = 0;

        /*!
         * \brief initialize outputs
         * \param options : set of options used for the optimization
         */
        virtual void initialize(BendersOptions options) = 0;

        virtual void end_writing(const IterationsData &iterations_data) = 0;
    };
}
using Writer = std::shared_ptr<Output::OutputWriter>;