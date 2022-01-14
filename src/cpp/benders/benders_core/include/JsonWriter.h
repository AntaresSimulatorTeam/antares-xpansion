
#pragma once

#include "OutputWriter.h"
#include "BendersOptions.h"
#include "TimeUtil.h"

#include <json/writer.h>
namespace Output
{
    /*!
     * \class JsonWriter
     * \brief JsonWriter class to describe the execuion session of an antares xpansion optimization in a json file
     */
    class JsonWriter : public OutputWriter
    {
    private:
        std::shared_ptr<TimeUtil> _time;
        std::string _filename;
        // attributes of the optimization execution
        Json::Value _output;

    public:
        /*!
         *  \brief JsonWriter default constructor
         */
        JsonWriter() = delete;
        JsonWriter(std::shared_ptr<TimeUtil> timer);

        /*!
         *  \brief destructor of class JsonWriter
         */
        virtual ~JsonWriter() = default;

        /*!
         *  \brief updates the execution begin time
         */
        virtual void updateBeginTime();

        /*!
         *  \brief updates the end of execution time
         */
        virtual void updateEndTime();

        /*!
         *  \brief saves the options of the benders algorithm to be later written to the json file
         *
         *  \param bendersOptions_p : set of options used for the optimization
         */
        virtual void write_options(BendersOptions const &bendersOptions_p);

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
        virtual void write_iteration(const IterationsData &iterations_data);

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
        virtual void update_solution(const SolutionData &solution_data);

        /*!
         *  \brief write an a priori errored json output, overwritten if optimization ends
         */
        virtual void write_failure();

        /*!
         *  \brief write the json data into a file
         */
        virtual void dump();
        /*!
         * \brief initialize outputs
         * \param options : set of options used for the optimization
         */
        void initialize(const BendersOptions &options);

        void end_writing(const IterationsData &iterations_data);
    };
}