
#pragma once

#include "Timer.h"
#include "BendersOptions.h"
#include "WorkerTrace.h"
#include "common.h"
namespace Output
{

    struct CandidateData
    {
        std::string name;
        double invest;
        double min;
        double max;
    };
    typedef std::vector<CandidateData> CandidatesVec;
    struct Iteration
    {
        double time;
        double lb;
        double ub;
        double best_ub;
        double optimality_gap;
        double relative_gap;
        double investment_cost;
        double operational_cost;
        double overall_cost;
        CandidatesVec candidates;
    };
    typedef std::vector<Iteration> Iterations;
    /*!
     *  \brief struct saves some entries to be later written to the json file
     *
     *   nbWeeks_p : number of the weeks in the study
     *   solution : solution data as iteration
     */
    struct SolutionData
    {
        Iteration solution;
        int nbWeeks_p;
        int best_it;
        std::string problem_status;
    };
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
        Iterations iters;
        SolutionData solution_data;
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
        virtual void initialize(const BendersOptions &options) = 0;

        virtual void end_writing(const IterationsData &iterations_data) = 0;
    };
}
using Writer = std::shared_ptr<Output::OutputWriter>;