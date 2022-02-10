
#pragma once

#include "OutputWriter.h"

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
        VoidWriter() = default;

        /*!
         *  \brief destructor of class VoidWriter
         */
        virtual ~VoidWriter() = default;
        virtual void updateBeginTime();

        virtual void updateEndTime();

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
        virtual void write_iterations(const IterationsData &iterations_data);

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
        virtual void update_solution(const SolutionData &solution_data);

        /*!
         *  \brief write an a priori errored json output, overwritten if optimization ends
         */
        virtual void write_failure();

        /*!
         *  \brief write the json data into a file
         */
        virtual void dump();
        virtual void initialize();
        virtual void end_writing(const IterationsData &iterations_data);
    };
}