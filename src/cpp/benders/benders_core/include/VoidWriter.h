
#pragma once

#include "OutputWriter.h"
#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "common.h"

namespace Output
{
    /*!
     * \class VoidWriter
     * \brief VoidWriter class to describe the execuion session of an antares xpansion optimization
     */
    class VoidWriter : public OutputWriter
    {

    public:
        /*!
         *  \brief VoidWriter default constructor
         */
        VoidWriter();

        /*!
         *  \brief destructor of class VoidWriter
         */
        virtual ~VoidWriter();
        virtual void updateBeginTime();

        virtual void updateEndTime();
        /*!
         *  \brief saves the options of the benders algorithm to be later written
         *
         *  \param bendersOptions_p : set of options used for the optimization
         */
        virtual void write_options(BendersOptions const &bendersOptions_p);

        /*!
         *  \brief saves some entries to be later written
         *
         *  \param nbWeeks_p : number of the weeks in the study
         *  \param bendersTrace_p : trace to be written ie iterations details
         *  \param bendersData_p : final benders data to get the best iteration
         *  \param min_abs_gap : minimum absolute gap wanted
         *  \param min_rel_gap : minimum relative gap wanted
         *  \param max_iter : maximum number of iterations
         */
        virtual void write_iteration(int const &nbWeeks_p, BendersTrace const &bendersTrace_p,
                                     BendersData const &bendersData_p, double const &min_abs_gap,
                                     double const &min_rel_gap, double const &max_iter);

        /*!
         *  \brief  saves some entries to be later written
         *
         *  \param nbWeeks_p : number of the weeks in the study
         *  \param lb_p : solution lower bound
         *  \param ub_p : solution upper bound
         *  \param investCost_p : investment cost
         *  \param operationalCost_p : operational cost
         *  \param overallCost_p : total cost, sum of invest and operational
         *  \param solution_p : point giving the solution and the candidates
         *  \param optimality_p : indicates if optimality was reached
         */
        virtual void update_solution(int nbWeeks_p, double const &lb_p, double const &ub_p, double const &investCost_p,
                                     double const &operationalCost_p, double const &overallCost_p,
                                     Point const &solution_p, bool const &optimality_p);

        /*!
         *  \brief write an a priori errored json output, overwritten if optimization ends
         */
        virtual void write_failure();

        /*!
         *  \brief write the json data into a file
         */
        virtual void dump();
        virtual void initialize(BendersOptions options);
        virtual void end_writing(int const &nbWeeks_p, BendersTrace const &bendersTrace_p, BendersData const &bendersData_p, double const &min_abs_gap, double const &min_rel_gap, double const &max_iter);
    };
}