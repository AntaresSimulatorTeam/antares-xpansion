
#pragma once

#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "common.h"
namespace Output
{
    class Time
    {
    public:
        /*!
         *  \brief Time default constructor
         */
        Time();

        /*!
         *  \brief destructor of class Time
         */
        virtual ~Time();
        /*!
         *  \brief updates the execution begin time
         */
        virtual void updateBeginTime();

        /*!
         *  \brief updates the end of execution time
         */
        virtual void updateEndTime();

        // attributes of the optimization execution

        /*!
         *  returns the start of execution time as a string
         */
        std::string getBegin();

        /*!
         *  returns the end of execution time as a string
         */
        std::string getEnd();

        /*!
         *  \brief return a double indicating the exection time in seconds
         */
        double getDuration();

    private:
        // begin time of the optimization
        std::time_t _beginTime;
        // end time of the optimization
        std::time_t _endTime;
    };

    /*!
     * \class OutputWriter
     * \brief OutputWriter class to describe the execuion session of an antares xpansion optimization in a log file
     */
    class OutputWriter
    {
    protected:
        // // begin time of the optimization
        // std::time_t _beginTime;
        // // end time of the optimization
        // std::time_t _endTime;

        // // attributes of the optimization execution

        // /*!
        //  *  returns the start of execution time as a string
        //  */
        // std::string getBegin();

        // /*!
        //  *  returns the end of execution time as a string
        //  */
        // std::string getEnd();

        // /*!
        //  *  \brief return a double indicating the exection time in seconds
        //  */
        // double getDuration();
        Time _time;
        std::string _filename;

    public:
        /*!
         *  \brief OutputWriter default constructor
         */
        OutputWriter();

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
         *  \param nbWeeks_p : number of the weeks in the study
         *  \param bendersTrace_p : trace to be written ie iterations details
         *  \param bendersData_p : final benders data to get the best iteration
         *  \param min_abs_gap : minimum absolute gap wanted
         *  \param min_rel_gap : minimum relative gap wanted
         *  \param max_iter : maximum number of iterations
         */
        virtual void write_iteration(int const &nbWeeks_p, BendersTrace const &bendersTrace_p,
                                     BendersData const &bendersData_p, double const &min_abs_gap,
                                     double const &min_rel_gap, double const &max_iter) = 0;

        /*!
         *  \brief  saves some entries to be later written to the json file
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
                                     Point const &solution_p, bool const &optimality_p) = 0;

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

        virtual void end_writing(int const &nbWeeks_p, BendersTrace const &bendersTrace_p, BendersData const &bendersData_p, double const &min_abs_gap, double const &min_rel_gap, double const &max_iter) = 0;
    };
}
using Writer = std::shared_ptr<Output::OutputWriter>;